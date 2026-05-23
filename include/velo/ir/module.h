#ifndef INC_VELO_IR_MODULE_H
#define INC_VELO_IR_MODULE_H

#include <vector>

#include "function.h"

namespace Velo::IR {
    struct Module final {
        std::vector<Function> functions {};
    };
}

#endif //INC_VELO_IR_MODULE_H
