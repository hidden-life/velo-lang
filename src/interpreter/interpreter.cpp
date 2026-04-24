#include "velo/interpreter/interpreter.h"

#include <iostream>
#include <ostream>

namespace Velo::Interpreter {
    auto Interpreter::execute(const IR::Module &module) -> int {
        for (const auto &fn : module.functions) {
            if (fn.name == "main") {
                executeFunc(fn);
                return 0;
            }
        }

        return 1;
    }

    void Interpreter::executeFunc(const IR::Function &func) {
        for (const auto &inst : func.instructions) {
            executeInstruction(inst);
        }
    }

    void Interpreter::executeInstruction(const IR::Instruction &inst) {
        using namespace IR;

        switch (inst.code) {
            case OpCode::PushInt:
                _stack.emplace_back(inst.intOperand);
                break;
            case OpCode::PushString:
                _stack.emplace_back(inst.stringOperand);
                break;
            case OpCode::CallBuiltin:
                callBuiltin(inst.stringOperand);
                break;
            case OpCode::Return:
                return;
        }
    }

    void Interpreter::callBuiltin(const std::string &name) {
        if (name == "console::println") {
            if (_stack.empty()) {
                return;
            }

            const auto value = _stack.back();
            _stack.pop_back();

            if (std::holds_alternative<std::string>(value)) {
                std::cout << std::get<std::string>(value) << std::endl;
            } else if (std::holds_alternative<int>(value)) {
                std::cout << std::get<int>(value) << std::endl;
            }

            return;
        }
    }
}
