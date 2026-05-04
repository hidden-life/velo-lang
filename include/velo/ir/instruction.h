#ifndef INC_VELO_IR_INSTRUCTION_H
#define INC_VELO_IR_INSTRUCTION_H

#include <string>

namespace Velo::IR {
    // Instruction kind for the minimal IR.
    enum class OpCode {
        PushInt,
        PushString,
        LoadLocal,
        StoreLocal,
        CallBuiltin,
        CallFunction,
        Pop,
        Return,

        AddInt,

        PushBool,
        JumpIfFalse,
        Jump,
    };

    struct Instruction final {
        OpCode code;

        std::string stringOperand {};
        int intOperand {0};
        bool boolOperand {false};
        // Argument count for CallBuiltin/CallFunction.
        std::size_t argsCount {0};
        // Local value index, for example a function parameter.
        std::size_t indexOperand {0};
        // Target instruction index for jump instructions.
        std::size_t targetOperand {0};
    };
}

#endif //INC_VELO_IR_INSTRUCTION_H
