#include "velo/interpreter/interpreter.h"

#include <iostream>
#include <ostream>
#include <sstream>

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

                _stack.push_back(_locals[inst.indexOperand]);
                return {};
            case OpCode::StoreLocal: {
                if (_stack.empty()) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "StoreLocal requires a value on the stack."
                    };
                }

                const auto value = _stack.back();
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
            case OpCode::LoadField: {
                return loadField(inst.stringOperand);
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
            _locals.push_back(std::move(arg));
        }

        // execute a function
        auto result = executeFunc(*it);
        if (!result.success) {
            return result;
        }

        Runtime::Value returnValue {};
        if (!_stack.empty()) {
            returnValue = _stack.back();
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
                *(first + static_cast<std::ptrdiff_t>(idx))
            );
        }

        _stack.erase(first, _stack.end());
        _stack.push_back(structVal);

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

        _stack.push_back(fieldIt->second);

        return {};
    }
}
