#ifndef INC_VELO_BYTECODE_COMPILER_H
#define INC_VELO_BYTECODE_COMPILER_H

#include "bytecode.h"
#include "velo/ir/module.h"

namespace Velo::Bytecode {
    class Compiler final {
    public:
        [[nodiscard]] auto compile(const IR::Module &irModule) const -> Module {
            Module bytecodeModule;

            bytecodeModule.functions.reserve(irModule.functions.size());

            for (const auto &irFunc : irModule.functions) {
                bytecodeModule.functions.push_back(compileFunction(irFunc));
            }

            return bytecodeModule;
        }

    private:
        [[nodiscard]] auto compileFunction(const IR::Function &irFunction) const -> Function {
            Function bytecodeFunction;

            bytecodeFunction.name = irFunction.name;
            bytecodeFunction.parameters = irFunction.parameters;
            bytecodeFunction.instructions.reserve(irFunction.instructions.size());

            for (const auto &irInstruction : irFunction.instructions) {
                bytecodeFunction.instructions.push_back(compileInstruction(irInstruction));
            }

            return bytecodeFunction;
        }

        [[nodiscard]] auto compileInstruction(const IR::Instruction &irInstruction) const -> Instruction {
            return Instruction {
                .code = mapOpCode(irInstruction.code),
                .stringOperand = irInstruction.stringOperand,
                .intOperand = irInstruction.intOperand,
                .boolOperand = irInstruction.boolOperand,
                .argsCount = irInstruction.argsCount,
                .indexOperand = irInstruction.indexOperand,
                .targetOperand = irInstruction.targetOperand,
            };
        }

        [[nodiscard]] auto mapOpCode(const IR::OpCode code) const -> OpCode {
            switch (code) {
                case IR::OpCode::PushInt:
                    return OpCode::PushInt;
                    case IR::OpCode::PushString:
                return OpCode::PushString;
                case IR::OpCode::PushBool:
                    return OpCode::PushBool;

                case IR::OpCode::LoadLocal:
                    return OpCode::LoadLocal;
                case IR::OpCode::StoreLocal:
                    return OpCode::StoreLocal;

                case IR::OpCode::AddInt:
                    return OpCode::AddInt;
                case IR::OpCode::SubInt:
                    return OpCode::SubInt;
                case IR::OpCode::MulInt:
                    return OpCode::MulInt;
                case IR::OpCode::DivInt:
                    return OpCode::DivInt;
                case IR::OpCode::ModInt:
                    return OpCode::ModInt;
                case IR::OpCode::NegInt:
                    return OpCode::NegInt;

                case IR::OpCode::LogicalNot:
                    return OpCode::LogicalNot;
                case IR::OpCode::LogicalAnd:
                    return OpCode::LogicalAnd;
                case IR::OpCode::LogicalOr:
                    return OpCode::LogicalOr;

                case IR::OpCode::CompareEqual:
                    return OpCode::CompareEqual;
                case IR::OpCode::CompareNotEqual:
                    return OpCode::CompareNotEqual;

                // Compatibility with older IR instructions that may still exist in the enum.
                // The bytecode model uses generic equality instructions.
                case IR::OpCode::CompareEqualInt:
                    return OpCode::CompareEqual;
                case IR::OpCode::CompareNotEqualInt:
                    return OpCode::CompareNotEqual;

                case IR::OpCode::CompareLessInt:
                    return OpCode::CompareLessInt;
                case IR::OpCode::CompareGreaterInt:
                    return OpCode::CompareGreaterInt;
                case IR::OpCode::CompareLessEqualInt:
                    return OpCode::CompareLessEqualInt;
                case IR::OpCode::CompareGreaterEqualInt:
                    return OpCode::CompareGreaterEqualInt;

                case IR::OpCode::Jump:
                    return OpCode::Jump;
                case IR::OpCode::JumpIfFalse:
                    return OpCode::JumpIfFalse;

                case IR::OpCode::CallFunction:
                    return OpCode::CallFunction;
                case IR::OpCode::CallBuiltin:
                    return OpCode::CallBuiltin;

                case IR::OpCode::BuildStruct:
                    return OpCode::BuildStruct;
                case IR::OpCode::BuildArray:
                    return OpCode::BuildArray;
                case IR::OpCode::BuildMap:
                    return OpCode::BuildMap;

                case IR::OpCode::LoadField:
                    return OpCode::LoadField;
                case IR::OpCode::StoreFieldPath:
                    return OpCode::StoreFieldPath;

                case IR::OpCode::LoadIndex:
                    return OpCode::LoadIndex;
                case IR::OpCode::StoreIndexPath:
                    return OpCode::StoreIndexPath;

                case IR::OpCode::Return:
                    return OpCode::Return;
                case IR::OpCode::Pop:
                    return OpCode::Pop;
            }

            return OpCode::Pop;
        }
    };
};

#endif //INC_VELO_BYTECODE_COMPILER_H
