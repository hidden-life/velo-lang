#include "velo/ir/ir_printer.h"

#include <ostream>
#include <sstream>

namespace Velo::IR {
    auto IRPrinter::print(const Module &module) const -> std::string {
        std::ostringstream stream;

        stream << "IRModule\n";
        for (const auto &func : module.functions) {
            printFunction(stream, func);
        }

        return stream.str();
    }

    void IRPrinter::printFunction(std::ostream &stream, const Function &func) const {
        stream << "  Function " << func.name;
        if (!func.parameters.empty()) {
            stream << "(";

            for (std::size_t idx = 0; idx < func.parameters.size(); ++idx) {
                if (idx > 0U) {
                    stream << ", ";
                }

                stream << func.parameters[idx];
            }

            stream << ")";
        }

        stream << "\n";

        for (std::size_t idx = 0; idx < func.instructions.size(); ++idx) {
            printInstruction(stream, idx, func.instructions[idx]);
        }
    }

    void IRPrinter::printInstruction(std::ostream &stream, std::size_t index, const Instruction &instruction) const {
        stream << "    " << index << ": " << opCodeToString(instruction.code);

        switch (instruction.code) {
            case OpCode::PushInt:
                stream << " " << instruction.intOperand;
                break;

            case OpCode::PushString:
                stream << " \"" << instruction.stringOperand << "\"";
                break;

            case OpCode::PushBool:
                stream << " " << (instruction.boolOperand ? "true" : "false");
                break;

            case OpCode::LoadLocal:
                stream << " local[" << instruction.indexOperand << "]";
                break;

            case OpCode::StoreLocal:
                stream << " local[" << instruction.indexOperand << "]";
                break;

            case OpCode::CallBuiltin:
                stream << " " << instruction.stringOperand << " args=" << instruction.argsCount;
                break;

            case OpCode::CallFunction:
                stream << " " << instruction.stringOperand << " args=" << instruction.argsCount;
                break;

            case OpCode::JumpIfFalse:
                stream << " -> " << instruction.targetOperand;
                break;

            case OpCode::Jump:
                stream << " -> " << instruction.targetOperand;
                break;

            case OpCode::BuildStruct:
                stream << " " << instruction.stringOperand << " fields=" << instruction.argsCount;
                break;

            case OpCode::Pop:
            case OpCode::Return:
            case OpCode::AddInt:
            case OpCode::SubInt:
            case OpCode::MulInt:
            case OpCode::DivInt:
            case OpCode::ModInt:
            case OpCode::NegInt:
            case OpCode::CompareEqualInt:
            case OpCode::CompareNotEqualInt:
            case OpCode::CompareLessInt:
            case OpCode::CompareGreaterInt:
            case OpCode::CompareLessEqualInt:
            case OpCode::CompareGreaterEqualInt:
            case OpCode::LogicalAnd:
            case OpCode::LogicalNot:
            case OpCode::LogicalOr:
                break;
        }

        stream << "\n";
    }

    auto IRPrinter::opCodeToString(OpCode code) -> std::string_view {
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

            case OpCode::CallBuiltin:
                return "CallBuiltin";

            case OpCode::CallFunction:
                return "CallFunction";

            case OpCode::JumpIfFalse:
                return "JumpIfFalse";

            case OpCode::Jump:
                return "Jump";

            case OpCode::Pop:
                return "Pop";

            case OpCode::Return:
                return "Return";

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

            case OpCode::CompareEqualInt:
                return "CompareEqualInt";

            case OpCode::CompareNotEqualInt:
                return "CompareNotEqualInt";

            case OpCode::CompareLessInt:
                return "CompareLessInt";

            case OpCode::CompareGreaterInt:
                return "CompareGreaterInt";

            case OpCode::CompareLessEqualInt:
                return "CompareLessEqualInt";

            case OpCode::CompareGreaterEqualInt:
                return "CompareGreaterEqualInt";

            case OpCode::LogicalAnd:
                return "LogicalAnd";

            case OpCode::LogicalNot:
                return "LogicalNot";

            case OpCode::LogicalOr:
                return "LogicalOr";

            case OpCode::BuildStruct:
                return "BuildStruct";
        }

        return "Unknown";
    }
}
