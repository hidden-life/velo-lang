#ifndef INC_VELO_BYTECODE_VM_H
#define INC_VELO_BYTECODE_VM_H

#include "velo/runtime/execution_result.h"
#include "velo/runtime/runtime.h"

#include <sstream>

#include "bytecode.h"

namespace Velo::Bytecode {
    namespace Detail {
        template <typename Predicate>
        auto compareIntegerValues(
            std::vector<Runtime::Value> &stack,
            Predicate predicate
        ) -> Runtime::ExecutionResult {
            if (stack.size() < 2U) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Comparison requires two values on the stack."
                };
            }

            const auto right = stack.back();
            stack.pop_back();
            const auto left = stack.back();
            stack.pop_back();

            if (!std::holds_alternative<int>(left) || !std::holds_alternative<int>(right)) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Comparison expect integer operands."
                };
            }

            stack.emplace_back(predicate(std::get<int>(left), std::get<int>(right)));

            return {};
        }

        template <typename Predicate>
        auto compareEqualityValues(
            std::vector<Runtime::Value> &stack,
            Predicate predicate
        ) -> Runtime::ExecutionResult {
            if (stack.size() < 2U) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Equality comparison requires two values on the stack."
                };
            }

            const auto right = stack.back();
            stack.pop_back();

            const auto left = stack.back();
            stack.pop_back();

            if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
                stack.emplace_back(predicate(std::get<int>(left), std::get<int>(right)));
                return {};
            }

            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                stack.emplace_back(predicate(std::get<std::string>(left), std::get<std::string>(right)));
                return {};
            }

            if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
                stack.emplace_back(predicate(std::get<bool>(left), std::get<bool>(right)));
                return {};
            }

            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Equality comparison expects operands of the same comparable type."
            };
        }

        template <typename Operation>
        auto evaluateBinaryInt(
            std::vector<Runtime::Value> &stack,
            Operation op
        ) -> Runtime::ExecutionResult {
            if (stack.size() < 2U) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Integer operation requires two values on the stack."
                };
            }

            const auto right = stack.back();
            stack.pop_back();
            const auto left = stack.back();
            stack.pop_back();

            if (!std::holds_alternative<int>(left) || !std::holds_alternative<int>(right)) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Integer operation expects integer operands."
                };
            }

            return op(std::get<int>(left), std::get<int>(right), stack);
        }

        template <typename Predicate>
        auto evaluateBinaryBool(
            std::vector<Runtime::Value> &stack,
            Predicate predicate
        ) -> Runtime::ExecutionResult {
            if (stack.size() < 2U) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Logical operation requires two values on the stack."
                };
            }

            const auto right = stack.back();
            stack.pop_back();
            const auto left = stack.back();
            stack.pop_back();

            if (!std::holds_alternative<bool>(left) || !std::holds_alternative<bool>(right)) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Logical operation expects bool operands."
                };
            }

            stack.emplace_back(predicate(std::get<bool>(left), std::get<bool>(right)));
            return {};
        }

        [[nodiscard]] inline auto splitString(
            const std::string &value,
            char delimiter
        ) -> std::vector<std::string> {
            std::vector<std::string> result;
            std::stringstream stream(value);
            std::string segment;

            while (std::getline(stream, segment, delimiter)) {
                if (!segment.empty()) {
                    result.push_back(segment);
                }
            }

            return result;
        }

        struct StructOperand final {
            std::string typeName {};
            std::vector<std::string> fieldNames {};
        };

        [[nodiscard]] inline auto decodeStructOperand(const std::string &encoded) -> StructOperand {
            StructOperand result;
            const auto separator = encoded.find('|');
            if (separator != std::string::npos) {
                result.typeName = encoded;
                return result;
            }

            result.typeName = encoded.substr(0U, separator);
            result.fieldNames = splitString(encoded.substr(separator + 1U), ',');

            return result;
        }

        [[nodiscard]] inline auto splitFieldPath(const std::string &encodedPath) -> std::vector<std::string> {
            return splitString(encodedPath, '.');
        }

        [[nodiscard]] inline auto decodeMapKeys(const std::string &encodedKeys) -> std::vector<std::string> {
            std::vector<std::string> keys;
            std::size_t offset = 0U;

            while (offset < encodedKeys.size()) {
                const auto colon = encodedKeys.find(':', offset);
                if (colon == std::string::npos) {
                    return {};
                }

                const auto lengthText = encodedKeys.substr(offset, colon - offset);
                std::size_t keyLength = 0U;

                try {
                    keyLength = static_cast<std::size_t>(std::stoull(lengthText));
                } catch (...) {
                    return {};
                }

                const auto keyBegin = colon + 1U;
                const auto keyEnd = keyBegin + keyLength;
                if (keyEnd > encodedKeys.size()) {
                    return {};
                }

                keys.push_back(encodedKeys.substr(keyBegin, keyLength));
                offset = keyEnd;
            }

            return keys;
        }
    }

    class VM final {
    public:
        explicit VM(Runtime::Runtime &runtime) : _runtime(runtime) {}

        [[nodiscard]] auto execute(const Module &module) -> Runtime::ExecutionResult {
            _currentModule = &module;
            _stack.clear();
            _locals.clear();

            const Function *mainFunction = nullptr;
            for (const auto &func : module.functions) {
                if (func.name == "main") {
                    mainFunction = &func;
                    break;
                }
            }

            if (mainFunction == nullptr) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Bytecode entry point 'main' was not found."
                };
            }

            return executeFunction(*mainFunction);
        }

    private:
        Runtime::Runtime &_runtime;
        const Module *_currentModule {nullptr};
        std::vector<Runtime::Value> _stack {};
        std::vector<Runtime::Value> _locals {};

        bool _lastJumpTaken {false};

        [[nodiscard]] auto executeFunction(const Function &function) -> Runtime::ExecutionResult {
            std::size_t instructionPointer = 0U;

            while (instructionPointer < function.instructions.size()) {
                const auto &instr = function.instructions[instructionPointer];
                auto result = executeInstruction(instr);

                if (!result.success) {
                    return result;
                }

                if (instr.code == OpCode::Return) {
                    return result;
                }

                if (instr.code == OpCode::Jump) {
                    instructionPointer = instr.targetOperand;
                    continue;
                }

                if (instr.code == OpCode::JumpIfFalse) {
                    if (_lastJumpTaken) {
                        instructionPointer = instr.targetOperand;
                        _lastJumpTaken = false;
                        continue;
                    }
                }

                ++instructionPointer;
            }

            return Runtime::ExecutionResult {};
        }

        [[nodiscard]] auto executeInstruction(const Instruction &instruction) -> Runtime::ExecutionResult {
            switch (instruction.code) {
                case OpCode::PushInt:
                    _stack.emplace_back(instruction.intOperand);
                    return {};

                case OpCode::PushString:
                    _stack.emplace_back(instruction.stringOperand);
                    return {};

                case OpCode::PushBool:
                    _stack.emplace_back(instruction.boolOperand);
                    return {};

                case OpCode::LoadLocal:
                    return loadLocal(instruction.indexOperand);

                case OpCode::StoreLocal:
                    return storeLocal(instruction.indexOperand);

                case OpCode::AddInt:
                    return Detail::evaluateBinaryInt(
                        _stack,
                        [](int left, int right, std::vector<Runtime::Value> &stack) {
                            stack.emplace_back(left + right);
                            return Runtime::ExecutionResult {};
                        }
                    );

                case OpCode::SubInt:
                    return Detail::evaluateBinaryInt(
                        _stack,
                        [](int left, int right, std::vector<Runtime::Value> &stack) {
                            stack.emplace_back(left - right);
                            return Runtime::ExecutionResult {};
                        }
                    );

                case OpCode::MulInt:
                    return Detail::evaluateBinaryInt(
                        _stack,
                        [](int left, int right, std::vector<Runtime::Value> &stack) {
                            stack.emplace_back(left * right);
                            return Runtime::ExecutionResult {};
                        }
                    );

                case OpCode::DivInt:
                    return Detail::evaluateBinaryInt(
                        _stack,
                        [](int left, int right, std::vector<Runtime::Value> &stack) {
                            if (right == 0) {
                                return Runtime::ExecutionResult {
                                    .success = false,
                                    .exitCode = 1,
                                    .error = "Division by zero."
                                };
                            }

                            stack.emplace_back(left / right);
                            return Runtime::ExecutionResult {};
                        }
                    );

                case OpCode::ModInt:
                    return Detail::evaluateBinaryInt(
                        _stack,
                        [](int left, int right, std::vector<Runtime::Value> &stack) {
                            if (right == 0) {
                                return Runtime::ExecutionResult {
                                    .success = false,
                                    .exitCode = 1,
                                    .error = "Modulo by zero."
                                };
                            }

                            stack.emplace_back(left % right);
                            return Runtime::ExecutionResult {};
                        }
                    );

                case OpCode::NegInt:
                    return negInt();

                case OpCode::LogicalNot:
                    return logicalNot();

                case OpCode::LogicalAnd:
                    return Detail::evaluateBinaryBool(
                        _stack,
                        [](bool left, bool right) {
                            return left && right;
                        }
                    );

                case OpCode::LogicalOr:
                    return Detail::evaluateBinaryBool(
                        _stack,
                        [](bool left, bool right) {
                            return left || right;
                        }
                    );

                case OpCode::CompareEqual:
                    return Detail::compareEqualityValues(
                        _stack,
                        [](const auto &left, const auto &right) {
                            return left == right;
                        }
                    );

                case OpCode::CompareNotEqual:
                    return Detail::compareEqualityValues(
                        _stack,
                        [](const auto &left, const auto &right) {
                            return left != right;
                        }
                    );

                case OpCode::CompareLessInt:
                    return Detail::compareIntegerValues(_stack, [](int left, int right) {
                        return left < right;
                    });

                case OpCode::CompareGreaterInt:
                    return Detail::compareIntegerValues(_stack, [](int left, int right) {
                        return left > right;
                    });

                case OpCode::CompareLessEqualInt:
                    return Detail::compareIntegerValues(_stack, [](int left, int right) {
                        return left <= right;
                    });

                case OpCode::CompareGreaterEqualInt:
                    return Detail::compareIntegerValues(_stack, [](int left, int right) {
                        return left >= right;
                    });

                case OpCode::Jump:
                    return {};

                case OpCode::JumpIfFalse:
                    return jumpIfFalse();

                case OpCode::CallFunction:
                    return callFunction(instruction.stringOperand, instruction.argsCount);

                case OpCode::CallBuiltin:
                    return callBuiltin(instruction.stringOperand, instruction.argsCount);

                case OpCode::BuildStruct:
                    return buildStruct(instruction.stringOperand, instruction.argsCount);

                case OpCode::BuildArray:
                    return buildArray(instruction.argsCount);

                case OpCode::BuildMap:
                    return buildMap(instruction.stringOperand, instruction.argsCount);

                case OpCode::LoadField:
                    return loadField(instruction.stringOperand);

                case OpCode::StoreFieldPath:
                    return storeFieldPath(instruction.stringOperand);

                case OpCode::LoadIndex:
                    return loadIndex();

                case OpCode::StoreIndexPath:
                    return storeIndexPath(instruction.argsCount);

                case OpCode::Return:
                    return returnFromFunction();

                case OpCode::Pop:
                    if (!_stack.empty()) {
                        _stack.pop_back();
                    }
                    return {};
                }

            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Unknown bytecode instruction."
            };
        }

        [[nodiscard]] auto loadLocal(std::size_t idx) -> Runtime::ExecutionResult {
            if (idx >= _locals.size()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Local index is out of range."
                };
            }

            _stack.push_back(Runtime::cloneValue(_locals[idx]));

            return {};
        }

        [[nodiscard]] auto storeLocal(std::size_t idx) -> Runtime::ExecutionResult {
            if (_stack.empty()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "StoreLocal requires a value on the stack."
                };
            }

            const auto value = Runtime::cloneValue(_stack.back());
            _stack.pop_back();

            if (idx > _locals.size()) {
                _locals.resize(idx);
            }

            if (idx == _locals.size()) {
                _locals.push_back(value);
            } else {
                _locals[idx] = value;
            }

            return {};
        }

        [[nodiscard]] auto negInt() -> Runtime::ExecutionResult {
            if (_stack.empty()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "NegInt requires one value on the stack."
                };
            }

            const auto value = _stack.back();
            _stack.pop_back();

            if (!std::holds_alternative<int>(value)) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "NegInt expects integer operand."
                };
            }

            _stack.emplace_back();

            return {};
        }

        [[nodiscard]] auto logicalNot() -> Runtime::ExecutionResult {
            if (_stack.empty()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "LogicalNot requires one value on the stack."
                };
            }

            const auto value = _stack.back();
            _stack.pop_back();

            if (!std::holds_alternative<bool>(value)) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "LogicalNot expects bool operand."
                };
            }

            _stack.emplace_back();
            return {};
        }

        [[nodiscard]] auto jumpIfFalse() -> Runtime::ExecutionResult {
            if (_stack.empty()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "JumpIfFalse requires a condition value."
                };
            }

            const auto condition = _stack.back();
            _stack.pop_back();

            if (!std::holds_alternative<bool>(condition)) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "JumpIfFalse condition must be bool."
                };
            }

            _lastJumpTaken = !std::get<bool>(condition);

            return {};
        }

        [[nodiscard]] auto callBuiltin(
            const std::string &name,
            std::size_t argsCount
        ) -> Runtime::ExecutionResult {
            const auto *func = _runtime.builtins().find(name);
            if (func == nullptr) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Unknown builtin function: " + name
                };
            }

            if (argsCount != func->arity()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Builtin function '" + name + "' expects " +
                        std::to_string(func->arity()) +
                        " argument(s), but " +
                        std::to_string(argsCount) +
                        " provided."
                };
            }

            if (_stack.size() < argsCount) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Not enough arguments for builtin function: " + name
                };
            }

            std::vector<Runtime::Value> arguments;
            arguments.reserve(argsCount);

            const auto first = _stack.end() - static_cast<std::ptrdiff_t>(argsCount);
            for (auto it = first; it != _stack.end(); ++it) {
                arguments.push_back(Runtime::cloneValue(*it));
            }

            _stack.erase(first, _stack.end());

            auto result = func->call(arguments);
            if (!result.success) {
                return result;
            }

            if (result.returnValue.has_value()) {
                _stack.push_back(Runtime::cloneValue(*result.returnValue));
            }

            return result;
        }

        [[nodiscard]] auto callFunction(const std::string &name, std::size_t argsCount) -> Runtime::ExecutionResult {
            if (_currentModule == nullptr) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "No bytecode module is currently loaded."
                };
            }

            const Function *target = nullptr;
            for (const auto &func : _currentModule->functions) {
                if (func.name == name) {
                    target = &func;
                    break;
                }
            }

            if (target == nullptr) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Unknown user-defined function: " + name
                };
            }

            if (argsCount != target->parameters.size()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Function '" + name + "' expects " +
                        std::to_string(target->parameters.size()) +
                            " argument(s), but " +
                            std::to_string(argsCount) +
                            " provided."
                };
            }

            if (_stack.size() < argsCount) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Not enough values on stack for function call: " + name
                };
            }

            std::vector<Runtime::Value> arguments;
            arguments.reserve(argsCount);

            const auto first = _stack.end() - static_cast<std::ptrdiff_t>(argsCount);
            for (auto it = first; it != _stack.end(); ++it) {
                arguments.push_back(Runtime::cloneValue(*it));
            }

            _stack.erase(first, _stack.end());

            auto callerStack = std::move(_stack);
            auto callerLocals = std::move(_locals);

            _stack.clear();
            _locals.clear();

            for (const auto &arg : arguments) {
                _locals.push_back(Runtime::cloneValue(arg));
            }

            auto result = executeFunction(*target);
            if (!result.success) {
                return result;
            }

            Runtime::Value returnValue {};
            bool hasReturnValue = false;

            if (!_stack.empty()) {
                returnValue = Runtime::cloneValue(_stack.back());
                hasReturnValue = true;
            }

            _stack = std::move(callerStack);
            _locals = std::move(callerLocals);

            if (hasReturnValue) {
                _stack.push_back(Runtime::cloneValue(returnValue));
            }

            return {};
        }

        [[nodiscard]] auto buildStruct(
            const std::string &encodedOp,
            std::size_t fieldsCount
        ) -> Runtime::ExecutionResult {
            if (_stack.size() < fieldsCount) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Operand stack underflow while building struct."
                };
            }

            const auto decoded = Detail::decodeStructOperand(encodedOp);
            if (decoded.fieldNames.size() != fieldsCount) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "BuildStruct field metadata does not match fields count."
                };
            }

            auto structValue = std::make_shared<Runtime::StructValue>();
            structValue->typeName = decoded.typeName;

            const auto first = _stack.end() - static_cast<std::ptrdiff_t>(fieldsCount);
            for (std::size_t idx = 0U; idx < fieldsCount; ++idx) {
                structValue->fields[decoded.fieldNames[idx]] = Runtime::cloneValue(*(first + static_cast<std::ptrdiff_t>(idx)));
            }

            _stack.erase(first, _stack.end());
            _stack.push_back(structValue);

            return {};
        }

        [[nodiscard]] auto buildArray(std::size_t elementsCount) -> Runtime::ExecutionResult {
            if (_stack.size() < elementsCount) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Operand stack underflow while building array."
                };
            }

            auto arrayValue = std::make_shared<Runtime::ArrayValue>();
            arrayValue->elements.reserve(elementsCount);

            const auto first = _stack.end() - static_cast<std::ptrdiff_t>(elementsCount);
            for (std::size_t idx = 0U; idx < elementsCount; ++idx) {
                arrayValue->elements.push_back(
                    Runtime::cloneValue(*(first + static_cast<std::ptrdiff_t>(idx)))
                );
            }

            _stack.erase(first, _stack.end());
            _stack.push_back(arrayValue);

            return {};
        }

        [[nodiscard]] auto buildMap(const std::string &encodedKeys, std::size_t entriesCount) -> Runtime::ExecutionResult {
            const auto keys = Detail::decodeMapKeys(encodedKeys);
            if (keys.size() != entriesCount) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Map key metadata does not match entries count."
                };
            }

            if (_stack.size() < entriesCount) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Operand stack underflow while building map."
                };
            }

            auto mapValue = std::make_shared<Runtime::MapValue>();
            const auto first = _stack.end() - static_cast<std::ptrdiff_t>(entriesCount);
            for (std::size_t idx = 0U; idx < entriesCount; ++idx) {
                mapValue->entries[keys[idx]] = Runtime::cloneValue(
                    *(first + static_cast<std::ptrdiff_t>(idx))
                );
            }

            _stack.erase(first, _stack.end());
            _stack.push_back(mapValue);

            return {};
        }

        [[nodiscard]] auto loadField(const std::string &fieldName) -> Runtime::ExecutionResult {
            if (_stack.empty()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "LoadField requires a struct value on the stack."
                };
            }

            const auto value = _stack.back();
            _stack.pop_back();

            if (!std::holds_alternative<Runtime::StructValuePtr>(value)) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "LoadField expects a struct value."
                };
            }

            const auto structValue = std::get<Runtime::StructValuePtr>(value);
            if (structValue == nullptr) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "LoadField received a null struct value."
                };
            }

            const auto fieldIt = structValue->fields.find(fieldName);
            if (fieldIt == structValue->fields.end()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Unknown field '" + fieldName + "' in struct value '" + structValue->typeName + "'."
                };
            }

            _stack.push_back(Runtime::cloneValue(fieldIt->second));

            return {};
        }

        [[nodiscard]] auto storeFieldPath(const std::string &encoded) -> Runtime::ExecutionResult {
            const auto path = Detail::splitFieldPath(encoded);
            if (_stack.empty()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "StoreFieldPath requires a non-empty field path."
                };
            }

            if (_stack.size() < 2U) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "StoreFieldPath requires a value and a struct value on the stack."
                };
            }

            const auto rootValue = _stack.back();
            _stack.pop_back();
            const auto assignedValue = _stack.back();
            _stack.pop_back();

            if (!std::holds_alternative<Runtime::StructValuePtr>(rootValue)) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "StoreFieldPath expects a struct value."
                };
            }

            const auto rootStruct = std::get<Runtime::StructValuePtr>(rootValue);
            if (rootStruct == nullptr) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "StoreFieldPath received a null struct value."
                };
            }

            Runtime::StructValuePtr current = rootStruct;

            for (std::size_t idx = 0U; idx + 1U < path.size(); ++idx) {
                const auto fieldIt = current->fields.find(path[idx]);
                if (fieldIt == current->fields.end()) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Unknown field '" + path[idx] + "' in struct value '" + current->typeName + "'."
                    };
                }

                if (!std::holds_alternative<Runtime::StructValuePtr>(fieldIt->second)) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Field '" + path[idx] + "' is not a struct value."
                    };
                }

                current = std::get<Runtime::StructValuePtr>(fieldIt->second);
                if (current == nullptr) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Field '" + path[idx] + "' is a null struct value."
                    };
                }
            }

            const std::string &leafFieldName = path.back();
            const auto leafIt = current->fields.find(leafFieldName);
            if (leafIt == current->fields.end()) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "Unknown field '" + leafFieldName + "' in struct value '" + current->typeName + "'."
                };
            }

            current->fields[leafFieldName] = Runtime::cloneValue(assignedValue);
            _stack.push_back(Runtime::cloneValue(rootValue));

            return {};
        }

        [[nodiscard]] auto loadIndex() -> Runtime::ExecutionResult {
            if (_stack.size() < 2U) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "LoadIndex requires a target value and an index value on the stack."
                };
            }

            const auto indexValue = _stack.back();
            _stack.pop_back();

            const auto targetValue = _stack.back();
            _stack.pop_back();

            if (std::holds_alternative<Runtime::ArrayValuePtr>(targetValue)) {
                if (!std::holds_alternative<int>(indexValue)) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "LoadIndex expects integer index for array value."
                    };
                }

                const int index = std::get<int>(indexValue);
                if (index < 0) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Array index out of range."
                    };
                }

                const auto array = std::get<Runtime::ArrayValuePtr>(targetValue);
                if (array == nullptr) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "LoadIndex received a null array value."
                    };
                }

                const auto indexAsSize = static_cast<std::size_t>(index);
                if (indexAsSize >= array->elements.size()) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Array index out of range."
                    };
                }

                _stack.push_back(Runtime::cloneValue(array->elements[indexAsSize]));

                return {};
            }

            if (std::holds_alternative<Runtime::MapValuePtr>(targetValue)) {
                if (!std::holds_alternative<std::string>(indexValue)) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "LoadIndex expects string key for map value."
                    };
                }

                const auto mapValue = std::get<Runtime::MapValuePtr>(targetValue);
                if (mapValue == nullptr) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "LoadIndex received a null map value."
                    };
                }

                const auto &key = std::get<std::string>(indexValue);
                const auto entryIt = mapValue->entries.find(key);
                if (entryIt == mapValue->entries.end()) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Map key not found: " + key
                    };
                }

                _stack.push_back(Runtime::cloneValue(entryIt->second));

                return {};
            }

            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "LoadIndex expects an array or map value."
            };
        }

        [[nodiscard]] auto storeIndexPath(std::size_t indexCount) -> Runtime::ExecutionResult {
            if (indexCount == 0U) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "StoreIndexPath requires at least one index."
                };
            }

            if (_stack.size() < indexCount + 2U) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "StoreIndexPath requires a value, an array value, and index values on the stack."
                };
            }

            std::vector<int> indexes(indexCount);

            for (std::size_t reverseIdx = 0U; reverseIdx < indexCount; ++reverseIdx) {

                const auto indexValue = _stack.back();
                _stack.pop_back();

                if (!std::holds_alternative<int>(indexValue)) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "StoreIndexPath expects integer indexes."
                    };
                }

                const int index = std::get<int>(indexValue);
                if (index < 0) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Array index out of range."
                    };
                }

                indexes[indexCount - 1U - reverseIdx] = index;
            }

            const auto rootValue = _stack.back();
            _stack.pop_back();

            const auto assignedValue = _stack.back();
            _stack.pop_back();

            if (!std::holds_alternative<Runtime::ArrayValuePtr>(rootValue)) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "StoreIndexPath expects an array value."
                };
            }

            const auto rootArray = std::get<Runtime::ArrayValuePtr>(rootValue);
            if (rootArray == nullptr) {
                return Runtime::ExecutionResult {
                    .success = false,
                    .exitCode = 1,
                    .error = "StoreIndexPath received a null array value."
                };
            }

            Runtime::ArrayValuePtr current = rootArray;

            for (std::size_t pathIdx = 0U; pathIdx < indexes.size(); ++pathIdx) {
                const auto indexAsSize = static_cast<std::size_t>(indexes[pathIdx]);

                if (indexAsSize >= current->elements.size()) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Array index out of range."
                    };
                }

                const bool isLeaf = pathIdx + 1U == indexes.size();
                if (isLeaf) {
                    current->elements[indexAsSize] = Runtime::cloneValue(assignedValue);
                    break;
                }

                auto &nextValue = current->elements[indexAsSize];
                if (!std::holds_alternative<Runtime::ArrayValuePtr>(nextValue)) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "StoreIndexPath expected nested array value."
                    };
                }

                current = std::get<Runtime::ArrayValuePtr>(nextValue);
                if (current == nullptr) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "StoreIndexPath received a null nested array value."
                    };
                }
            }

            _stack.push_back(Runtime::cloneValue(rootValue));

            return {};
        }

        [[nodiscard]] auto returnFromFunction() -> Runtime::ExecutionResult {
            if (!_stack.empty()) {
                const auto returnValue = Runtime::cloneValue(_stack.back());
                if (std::holds_alternative<int>(returnValue)) {
                    return Runtime::ExecutionResult {
                        .success = true,
                        .exitCode = std::get<int>(returnValue),
                        .error = {},
                        .returnValue = returnValue
                    };
                }

                return Runtime::ExecutionResult {
                    .success = true,
                    .exitCode = 0,
                    .error = {},
                    .returnValue = returnValue
                };
            }

            return Runtime::ExecutionResult {};
        }
    };
}

#endif //INC_VELO_BYTECODE_VM_H
