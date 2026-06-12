#include "velo/interpreter/interpreter.h"

#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

namespace {
    template <typename Predicate>
    auto compareIntegerValues(
        std::vector<Velo::Runtime::Value> &stack,
        Predicate predicate
    ) -> Velo::Runtime::ExecutionResult {
        if (stack.size() < 2U) {
            return Velo::Runtime::ExecutionResult {
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
            return Velo::Runtime::ExecutionResult{
                .success = false,
                .exitCode = 1,
                .error = "Comparison expects integer operands."
            };
        }

        stack.emplace_back(predicate(std::get<int>(left), std::get<int>(right)));

        return {};
    }

    template <typename Predicate>
    auto compareEqualityValues(std::vector<Velo::Runtime::Value> &stack, Predicate predicate) -> Velo::Runtime::ExecutionResult {
        if (stack.size() < 2U) {
            return Velo::Runtime::ExecutionResult {
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

        return Velo::Runtime::ExecutionResult {
            .success = false,
            .exitCode = 1,
            .error = "Equality comparison expects operands of the same comparable type."
        };
    }

    template <typename Predicate>
    auto evaluateBinaryBool(
        std::vector<Velo::Runtime::Value> &stack,
        Predicate predicate
    ) -> Velo::Runtime::ExecutionResult {
        if (stack.size() < 2U) {
            return Velo::Runtime::ExecutionResult {
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
            return Velo::Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Logical operation expects bool operands."
            };
        }

        stack.emplace_back(predicate(std::get<bool>(left), std::get<bool>(right)));

        return {};
    }

    template <typename Operation>
    auto evaluateBinaryInt(std::vector<Velo::Runtime::Value> &stack, Operation op) -> Velo::Runtime::ExecutionResult {
        if (stack.size() < 2U) {
            return Velo::Runtime::ExecutionResult {
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
            return Velo::Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Integer operation expects integer operands."
            };
        }

        return op(std::get<int>(left), std::get<int>(right), stack);
    }

    auto evaluateUnaryIntNegation(std::vector<Velo::Runtime::Value> &stack) -> Velo::Runtime::ExecutionResult {
        if (stack.empty()) {
            return Velo::Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "NegInt requires one value on the stack."
            };
        }

        const auto value = stack.back();
        stack.pop_back();

        if (!std::holds_alternative<int>(value)) {
            return Velo::Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "NegInt expects integer operand."
            };
        }

        stack.emplace_back(-std::get<int>(value));

        return {};
    }

    auto splitStructOperand(const std::string &encodedOperand) -> std::pair<std::string, std::vector<std::string>> {
        const auto colonPos = encodedOperand.find(':');
        if (colonPos == std::string::npos) {
            return {encodedOperand, {}};
        }

        std::string typeName = encodedOperand.substr(0, colonPos);
        std::vector<std::string> fields;

        const std::string fieldsText = encodedOperand.substr(colonPos + 1U);
        std::string current;

        for (const char ch : fieldsText) {
            if (ch == ',') {
                fields.push_back(current);
                current.clear();
                continue;
            }

            current += ch;
        }

        if (!current.empty()) {
            fields.push_back(current);
        }

        return {std::move(typeName), std::move(fields)};
    }

    [[nodiscard]] auto decodeMapKeys(const std::string &encodedKeys) -> std::vector<std::string> {
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

    [[nodiscard]] auto splitFieldPath(const std::string &encoded) -> std::vector<std::string> {
        std::vector<std::string> result;
        std::stringstream stream(encoded);
        std::string segment;

        while (std::getline(stream, segment, '.')) {
            if (!segment.empty()) {
                result.push_back(segment);
            }
        }

        return result;
    }
}

namespace Velo::Interpreter {
    Interpreter::Interpreter(Runtime::Runtime &runtime) : _runtime(runtime) {
    }

    auto Interpreter::execute(const IR::Module &module) -> Runtime::ExecutionResult {
        _currentModule = &module;
        _stack.clear();
        _locals.clear();

        const auto it = std::ranges::find_if(
            module.functions,
            [](const IR::Function &function) {
                return function.name == "main";
            }
        );

        if (it == module.functions.end()) {
            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Runtime entry point 'main' was not found."
            };
        }

        return executeFunc(*it);
    }

    auto Interpreter::executeFunc(const IR::Function &func) -> Runtime::ExecutionResult {
        std::size_t instructionPointer = 0U;
        while (instructionPointer < func.instructions.size()) {
            const auto &inst = func.instructions[instructionPointer];
            auto result = executeInstruction(inst);
            if (!result.success) {
                return result;
            }

            if (inst.code == IR::OpCode::Return) {
                return result;
            }

            if (inst.code == IR::OpCode::Jump) {
                instructionPointer = inst.targetOperand;
                continue;
            }

            if (inst.code == IR::OpCode::JumpIfFalse) {
                if (_lastJumpTaken) {
                    instructionPointer = inst.targetOperand;
                    _lastJumpTaken = false;
                    continue;
                }
            }

            ++instructionPointer;
        }

        return Runtime::ExecutionResult {};
    }

    auto Interpreter::executeInstruction(const IR::Instruction &inst) -> Runtime::ExecutionResult {
        using IR::OpCode;

        switch (inst.code) {
            case OpCode::PushInt:
                _stack.emplace_back(inst.intOperand);
                return {};
            case OpCode::PushString:
                _stack.emplace_back(inst.stringOperand);
                return {};
            case OpCode::CallBuiltin:
                return callBuiltin(inst.stringOperand, inst.argsCount);
            case OpCode::CallFunction:
                return callFunction(inst.stringOperand, inst.argsCount);
            case OpCode::Return:
                if (!_stack.empty() && std::holds_alternative<int>(_stack.back())) {
                    return Runtime::ExecutionResult {
                        .success = true,
                        .exitCode = std::get<int>(_stack.back()),
                        .error = {}
                    };
                }
                return {};
            case OpCode::Pop:
                if (!_stack.empty()) {
                    _stack.pop_back();
                }
                return {};
            case OpCode::LoadLocal:
                if (inst.indexOperand >= _locals.size()) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Local index is out of range."
                    };
                }

                _stack.push_back(Runtime::cloneValue(_locals[inst.indexOperand]));
                return {};
            case OpCode::StoreLocal: {
                if (_stack.empty()) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "StoreLocal requires a value on the stack."
                    };
                }

                const auto value = Runtime::cloneValue(_stack.back());
                _stack.pop_back();

                if (inst.indexOperand > _locals.size()) {
                    _locals.resize(inst.indexOperand);
                }

                if (inst.indexOperand == _locals.size()) {
                    _locals.push_back(value);
                } else {
                    _locals[inst.indexOperand] = value;
                }

                return {};
            }
            case OpCode::PushBool: {
                _stack.emplace_back(inst.boolOperand);
                return {};
            }
            case OpCode::JumpIfFalse: {
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
            case OpCode::Jump:
                return {};
            case OpCode::CompareEqual:
                return compareEqualityValues(_stack, [](const auto &left, const auto &right) {
                    return left == right;
                });
            case OpCode::CompareNotEqual:
                return compareEqualityValues(_stack, [](const auto &left, const auto &right) {
                    return left != right;
                });
            case OpCode::CompareEqualInt:
                return compareIntegerValues(_stack, [](int left, int right) { return left == right; });

            case OpCode::CompareNotEqualInt:
                return compareIntegerValues(_stack, [](int left, int right) { return left != right; });
            case OpCode::CompareLessInt:
                return compareIntegerValues(_stack, [](int left, int right) { return left < right;});
            case OpCode::CompareGreaterInt:
                return compareIntegerValues(_stack, [](int left, int right) { return left > right; });
            case OpCode::CompareLessEqualInt:
                return compareIntegerValues(_stack, [](int left, int right) { return left <= right; });
            case OpCode::CompareGreaterEqualInt:
                return compareIntegerValues(_stack, [](int left, int right) { return left >= right; });
            case OpCode::AddInt: {
                return evaluateBinaryInt(
                    _stack,
                        [](int left, int right, std::vector<Runtime::Value> &stack) {
                            stack.emplace_back(left + right);
                            return Runtime::ExecutionResult {};
                        }
                    );
            }
            case OpCode::SubInt: {
                return evaluateBinaryInt(
                    _stack,
                    [](int left, int right, std::vector<Runtime::Value> &stack) {
                        stack.emplace_back(left - right);
                        return Runtime::ExecutionResult {};
                    }
                );
            }
            case OpCode::MulInt: {
                return evaluateBinaryInt(
                    _stack,
                    [](int left, int right, std::vector<Runtime::Value> &stack) {
                        stack.emplace_back(left * right);
                        return Runtime::ExecutionResult {};
                    }
                );
            }
            case OpCode::DivInt: {
                return evaluateBinaryInt(
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
            }
            case OpCode::ModInt: {
                return evaluateBinaryInt(
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
            }
            case OpCode::NegInt: {
                return evaluateUnaryIntNegation(_stack);
            }
            case OpCode::LogicalNot: {
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

                _stack.emplace_back(!std::get<bool>(value));
                return {};
            }
            case OpCode::LogicalAnd: {
                return evaluateBinaryBool(
                    _stack,
                    [](bool left, bool right) { return left && right; }
                );
            }
            case OpCode::LogicalOr: {
                return evaluateBinaryBool(
                    _stack,
                    [](bool left, bool right) { return left || right; }
                );
            }
            case OpCode::BuildStruct: {
                return buildStruct(inst.stringOperand, inst.argsCount);
            }
            case OpCode::BuildArray: {
                return buildArray(inst.argsCount);
            }
            case OpCode::BuildMap: {
                return buildMap(inst.stringOperand, inst.argsCount);
            }
            case OpCode::LoadField: {
                return loadField(inst.stringOperand);
            }
            case OpCode::LoadIndex: {
                return loadIndex();
            }
            case OpCode::StoreFieldPath: {
                return storeFieldPath(inst.stringOperand);
            }
            case OpCode::StoreIndexPath: {
                return storeIndexPath(inst.argsCount);
            }
        }

        return Runtime::ExecutionResult {
            .success = false,
            .exitCode = 1,
            .error = "Unknown interpreter instruction."
        };
    }

    auto Interpreter::callBuiltin(const std::string &name, std::size_t argsCount) -> Runtime::ExecutionResult {
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
                false,
                1,
                "Builtin function '" + name + "' expects " +
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
        arguments.insert(arguments.end(), first, _stack.end());
        _stack.erase(first, _stack.end());

        auto result = func->call(arguments);
        if (!result.success) {
            return result;
        }

        if (result.returnValue.has_value()) {
            _stack.push_back(*result.returnValue);
        }

        return {};
    }

    auto Interpreter::callFunction(const std::string &name, std::size_t argsCount) -> Runtime::ExecutionResult {
        if (_currentModule == nullptr) {
            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "No IR module is currently loaded."
            };
        }

        const auto it = std::ranges::find_if(
            _currentModule->functions,
            [&name](const IR::Function &func) {
                return func.name == name;
            }
        );

        if (it == _currentModule->functions.end()) {
            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Unknown user-defined function: " + name
            };
        }

        const auto &f = *it;
        if (argsCount != f.parameters.size()) {
            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Function '" + name + "' expects " +
                    std::to_string(f.parameters.size()) +
                    " argument(s), but " +
                    std::to_string(argsCount) +
                    " provided."
            };
        }

        std::vector<Runtime::Value> arguments;
        if (_stack.size() < argsCount) {
            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Not enough values on stack for function call: " + name
            };
        }

        const auto first = _stack.end() - static_cast<std::ptrdiff_t>(argsCount);
        arguments.insert(arguments.end(), first, _stack.end());
        _stack.erase(first, _stack.end());

        // save current stack
        std::vector<Runtime::Value> callerStack = std::move(_stack);
        std::vector<Runtime::Value> callerLocals = std::move(_locals);
        // new stack
        _stack.clear();
        _locals.clear();

        // At this stage parameters are placed onto the callee stack.
        // Later this will be replaced with a real frame with local slots.
        for (auto &arg : arguments) {
            _locals.push_back(Runtime::cloneValue(arg));
        }

        // execute a function
        auto result = executeFunc(*it);
        if (!result.success) {
            return result;
        }

        Runtime::Value returnValue {};
        if (!_stack.empty()) {
            returnValue = Runtime::cloneValue(_stack.back());
        }

        // recover caller stack
        _stack = std::move(callerStack);
        _locals = std::move(callerLocals);
        _stack.push_back(returnValue);

        return {};
    }

    auto Interpreter::buildStruct(const std::string &encodedOperand, std::size_t fieldsCount) -> Runtime::ExecutionResult {
        const auto [typeName, fieldNames] = splitStructOperand(encodedOperand);

        if (fieldNames.size() != fieldsCount) {
            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Struct field metadata does not match field count."
            };
        }

        if (_stack.size() < fieldsCount) {
            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Operand stack underflow while building struct."
            };
        }

        auto structVal = std::make_shared<Runtime::StructValue>();
        structVal->typeName = typeName;

        const auto first = _stack.end() - static_cast<std::ptrdiff_t>(fieldsCount);
        for (std::size_t idx = 0U; idx < fieldsCount; idx++) {
            structVal->fields.emplace(
                fieldNames[idx],
                Runtime::cloneValue(*(first + static_cast<std::ptrdiff_t>(idx)))
            );
        }

        _stack.erase(first, _stack.end());
        _stack.push_back(structVal);

        return {};
    }

    auto Interpreter::buildArray(std::size_t elementsCount) -> Runtime::ExecutionResult {
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
        for (std::size_t idx = 0U; idx < elementsCount; idx++) {
            arrayValue->elements.push_back(Runtime::cloneValue(*(first + static_cast<std::ptrdiff_t>(idx))));
        }

        _stack.erase(first, _stack.end());
        _stack.push_back(arrayValue);

        return {};
    }

    auto Interpreter::buildMap(const std::string &encodedKeys, std::size_t entriesCount) -> Runtime::ExecutionResult {
        const auto keys = decodeMapKeys(encodedKeys);
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

    auto Interpreter::loadField(const std::string &fieldName) -> Runtime::ExecutionResult {
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

    auto Interpreter::storeFieldPath(const std::string &encodedPath) -> Runtime::ExecutionResult {
        const auto path = splitFieldPath(encodedPath);
        if (path.empty()) {
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
                .error = "StoreFieldPath a struct value."
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
        for (std::size_t idx = 0; idx + 1U < path.size(); ++idx) {
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

    auto Interpreter::loadIndex() -> Runtime::ExecutionResult {
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

    auto Interpreter::storeIndexPath(std::size_t indexCount) -> Runtime::ExecutionResult {
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
}
