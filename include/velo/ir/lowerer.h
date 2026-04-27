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

        std::unordered_map<std::string, std::size_t> _locals {};
    };
}

#endif //INC_VELO_IR_LOWERER_H
