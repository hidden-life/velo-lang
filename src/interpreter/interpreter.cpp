#include "velo/interpreter/interpreter.h"

#include <iostream>
#include <ostream>

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
            case OpCode::AddInt:
                if (_stack.size() < 2U) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Not enough values on stack for AddInt."
                    };
                }

                const auto right = _stack.back();
                _stack.pop_back();
                const auto left = _stack.back();
                _stack.pop_back();

                if (!std::holds_alternative<int>(left) || !std::holds_alternative<int>(right)) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "AddInt expects integer operands."
                    };
                }

                _stack.emplace_back(std::get<int>(left) + std::get<int>(right));

                return {};
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

        return func->call(arguments);
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
}
