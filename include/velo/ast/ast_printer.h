#ifndef INC_VELO_AST_AST_PRINTER_H
#define INC_VELO_AST_AST_PRINTER_H

#include "ast.h"

namespace Velo::AST {
    // A simple AST printer for debugging and early development stages
    class ASTPrinter final {
    public:
        [[nodiscard]] auto print(const Program &program) const -> std::string;
    };
}

#endif //INC_VELO_AST_AST_PRINTER_H
