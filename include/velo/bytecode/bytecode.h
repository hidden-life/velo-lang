#ifndef INC_VELO_BYTECODE_BYTECODE_H
#define INC_VELO_BYTECODE_BYTECODE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Velo::Bytecode {
    enum class OpCode : std::uint8_t {
        PushInt,
        PushString,
        PushBool,

        LoadLocal,
        StoreLocal,

        AddInt,
        SubInt,
        MulInt,
        DivInt,
        ModInt,
        NegInt,

        LogicalNot,
        LogicalOr,
        LogicalAnd,

        CompareEqual,
        CompareNotEqual,
        CompareLessInt,
        CompareGreaterInt,
        CompareLessEqualInt,
        CompareGreaterEqualInt,

        Jump,
        JumpIfFalse,

        CallFunction,
        CallBuiltin,

        BuildStruct,
        BuildArray,
        BuildMap,
        LoadField,
        StoreFieldPath,
        LoadIndex,
        StoreIndexPath,

        Return,
        Pop,
    };

    struct Instruction final {
        OpCode code { OpCode::Pop };

        std::string stringOperand {};
        int intOperand { 0 };
        bool boolOperand { false };

        std::size_t argsCount { 0U };
        std::size_t indexOperand { 0U };
        std::size_t targetOperand { 0U };
    };

    struct Function final {
        std::string name {};
        std::vector<std::string> parameters {};
        std::vector<Instruction> instructions {};
    };

    struct Module final {
        std::vector<Function> functions {};
    };

    [[nodiscard]] inline auto opCodeToString(OpCode code) -> std::string {
        switch (code) {
            case OpCode::PushInt:
                return "PushInt";
            case OpCode::PushString:
                return "PushString";
            case OpCode::PushBool:
                return "PushBool";

            case OpCode::LoadLocal:
                return "LoadLocal";
            case OpCode::StoreLocal:
                return "StoreLocal";

            case OpCode::AddInt:
                return "AddInt";
            case OpCode::SubInt:
                return "SubInt";
            case OpCode::MulInt:
                return "MulInt";
            case OpCode::DivInt:
                return "DivInt";
            case OpCode::ModInt:
                return "ModInt";
            case OpCode::NegInt:
                return "NegInt";

            case OpCode::LogicalNot:
                return "LogicalNot";
            case OpCode::LogicalAnd:
                return "LogicalAnd";
            case OpCode::LogicalOr:
                return "LogicalOr";

            case OpCode::CompareEqual:
                return "CompareEqual";
            case OpCode::CompareNotEqual:
                return "CompareNotEqual";
            case OpCode::CompareLessInt:
                return "CompareLessInt";
            case OpCode::CompareGreaterInt:
                return "CompareGreaterInt";
            case OpCode::CompareLessEqualInt:
                return "CompareLessEqualInt";
            case OpCode::CompareGreaterEqualInt:
                return "CompareGreaterEqualInt";

            case OpCode::Jump:
                return "Jump";
            case OpCode::JumpIfFalse:
                return "JumpIfFalse";

            case OpCode::CallFunction:
                return "CallFunction";
            case OpCode::CallBuiltin:
                return "CallBuiltin";

            case OpCode::BuildStruct:
                return "BuildStruct";
            case OpCode::BuildArray:
                return "BuildArray";
            case OpCode::BuildMap:
                return "BuildMap";
            case OpCode::LoadField:
                return "LoadField";
            case OpCode::StoreFieldPath:
                return "StoreFieldPath";
            case OpCode::LoadIndex:
                return "LoadIndex";
            case OpCode::StoreIndexPath:
                return "StoreIndexPath";

            case OpCode::Return:
                return "Return";
            case OpCode::Pop:
                return "Pop";
        }

        return "Unknown";
    }
}

#endif //INC_VELO_BYTECODE_BYTECODE_H
