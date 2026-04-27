#include "velo/interpreter/interpreter.h"

#include <iostream>
#include <ostream>

namespace Velo::Interpreter {
    Interpreter::Interpreter(Runtime::Runtime &runtime) : _runtime(runtime) {
    }

    auto Interpreter::execute(const IR::Module &module) -> Runtime::ExecutionResult {
        _currentModule = &module;
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
        for (const auto &inst : func.instructions) {
            auto result = executeInstruction(inst);
            if (!result.success) {
                return result;
            }

            if (inst.code == IR::OpCode::Return) {
                return result;
            }
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
                if (inst.indexOperand >= _stack.size()) {
                    return Runtime::ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "Local index is out of range."
                    };
                }

                _stack.push_back(_stack[inst.indexOperand]);
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
        // new stack
        _stack.clear();

        // At this stage parameters are placed onto the callee stack.
        // Later this will be replaced with a real frame with local slots.
        for (auto &arg : arguments) {
            _stack.push_back(std::move(arg));
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
        _stack.push_back(returnValue);

        return {};
    }
}
