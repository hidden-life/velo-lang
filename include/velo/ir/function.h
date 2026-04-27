#ifndef INC_VELO_IR_FUNCTION_H
#define INC_VELO_IR_FUNCTION_H

#include <string>
#include <vector>

#include "instruction.h"

namespace Velo::IR {
    struct Function final {
        std::string name;
        std::vector<Instruction> instructions {};
        std::vector<std::string> parameters {};
    };
}

#endif //INC_VELO_IR_FUNCTION_H
