#ifndef INC_VELO_IR_LOWERER_H
#define INC_VELO_IR_LOWERER_H

#include <unordered_map>

#include "module.h"
#include "velo/ast/ast.h"

namespace Velo::IR {
    // Lowers AST into executable IR.
    class Lowerer final {
    public:
        [[nodiscard]] auto lower(const AST::Program &program) -> Module;

    private:
        Function lowerFunction(const AST::FunctionDeclaration &func);
        void lowerStatement(const AST::Statement &stmt, Function &func);
        void lowerExpression(const AST::Expression &expr, Function &func);
        [[nodiscard]] auto findLocalIndex(const std::string &name) const -> const std::size_t*;

        void lowerLogicalAndExpression(const AST::BinaryExpression &expr, Function &func);
        void lowerLogicalOrExpression(const AST::BinaryExpression &expr, Function &func);

        std::unordered_map<std::string, std::size_t> _locals {};

        struct LoopContext {
            size_t conditionIndex;
            std::vector<size_t> breakJumps;
        };

        std::vector<LoopContext> _loopStack;
    };
}

#endif //INC_VELO_IR_LOWERER_H
