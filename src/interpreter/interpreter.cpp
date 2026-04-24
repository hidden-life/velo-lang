#include "velo/interpreter/interpreter.h"

#include <iostream>
#include <ostream>

namespace Velo::Interpreter {
    Interpreter::Interpreter(Runtime::Runtime &runtime) : _runtime(runtime) {
    }

    auto Interpreter::execute(const IR::Module &module) -> Runtime::ExecutionResult {
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
                return callBuiltin(inst.stringOperand);
            case OpCode::Return:
                if (!_stack.empty() && std::holds_alternative<int>(_stack.back())) {
                    return Runtime::ExecutionResult {
                        .success = true,
                        .exitCode = std::get<int>(_stack.back()),
                        .error = {}
                    };
                }
                return {};
        }

        return Runtime::ExecutionResult {
            .success = false,
            .exitCode = 1,
            .error = "Unknown interpreter instruction."
        };
    }

    auto Interpreter::callBuiltin(const std::string &name) -> Runtime::ExecutionResult {
        const auto *func = _runtime.builtins().find(name);
        if (func == nullptr) {
            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Unknown builtin function:" + name
            };
        }

        if (_stack.size() < func->arity()) {
            return Runtime::ExecutionResult {
                .success = false,
                .exitCode = 1,
                .error = "Not enough arguments for builtin function: " + name
            };
        }

        std::vector<Runtime::Value> arguments;
        arguments.reserve(func->arity());

        const auto first = _stack.end() - static_cast<std::ptrdiff_t>(func->arity());
        arguments.insert(arguments.end(), first, _stack.end());
        _stack.erase(first, _stack.end());

        return func->call(arguments);
    }
}
