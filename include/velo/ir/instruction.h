#ifndef INC_VELO_IR_INSTRUCTION_H
#define INC_VELO_IR_INSTRUCTION_H

#include <string>

namespace Velo::IR {
    // Instruction kind for the minimal IR.
    enum class OpCode {
        PushInt,
        PushString,
        CallBuiltin,
        CallFunction,
        Pop,
        Return,
    };

    struct Instruction final {
        OpCode code;

        std::string stringOperand {};
        int intOperand {0};
        // Argument count for CallBuiltin/CallFunction.
        std::size_t argsCount {0};
    };
}

#endif //INC_VELO_IR_INSTRUCTION_H
