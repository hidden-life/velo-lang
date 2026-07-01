#ifndef INC_VELO_IR_FUNCTION_H
#define INC_VELO_IR_FUNCTION_H

#include <string>
#include <vector>

#include "instruction.h"

namespace Velo::IR {
    enum class AnnotationArgumentKind {
        StringLiteral,
        IntegerLiteral,
        BooleanLiteral,
    };

    struct AnnotationArgument final {
        AnnotationArgumentKind kind {AnnotationArgumentKind::StringLiteral};
        std::string value {};
    };

    struct Annotation final {
        std::string name {};
        std::vector<AnnotationArgument> arguments {};
    };

    struct Function final {
        std::string name;
        std::vector<Instruction> instructions {};
        std::vector<std::string> parameters {};
        std::vector<Annotation> annotations {};
    };
}

#endif //INC_VELO_IR_FUNCTION_H
