#ifndef INC_VELO_INTERPRETER_INTERPRETER_H
#define INC_VELO_INTERPRETER_INTERPRETER_H

#include <variant>
#include <string>

#include "velo/ir/module.h"

namespace Velo::Interpreter {
    using Value = std::variant<int, std::string>;

    class Interpreter final {
    public:
        [[nodiscard]] auto execute(const IR::Module &module) -> int;

    private:
        void executeFunc(const IR::Function &func);
        void executeInstruction(const IR::Instruction &inst);
        void callBuiltin(const std::string &name);

        std::vector<Value> _stack {};
    };
}

#endif //INC_VELO_INTERPRETER_INTERPRETER_H
