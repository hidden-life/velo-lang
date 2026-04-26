#ifndef INC_VELO_INTERPRETER_INTERPRETER_H
#define INC_VELO_INTERPRETER_INTERPRETER_H

#include <variant>
#include <string>

#include "velo/ir/module.h"
#include "velo/runtime/runtime.h"

namespace Velo::Interpreter {
    class Interpreter final {
    public:
        explicit Interpreter(Runtime::Runtime &runtime);

        [[nodiscard]] auto execute(const IR::Module &module) -> Runtime::ExecutionResult;

    private:
        [[nodiscard]] auto executeFunc(const IR::Function &func) -> Runtime::ExecutionResult;
        [[nodiscard]] auto executeInstruction(const IR::Instruction &inst) -> Runtime::ExecutionResult;
        [[nodiscard]] auto callBuiltin(const std::string &name, std::size_t argsCount) -> Runtime::ExecutionResult;
        [[nodiscard]] auto callFunction(const std::string &name, std::size_t argsCount) -> Runtime::ExecutionResult;

        std::vector<Runtime::Value> _stack {};
        Runtime::Runtime &_runtime;
        const IR::Module *_currentModule = nullptr;
    };
}

#endif //INC_VELO_INTERPRETER_INTERPRETER_H
