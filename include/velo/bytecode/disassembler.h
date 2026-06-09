#ifndef INC_VELO_BYTECODE_DISASSEMBLER_H
#define INC_VELO_BYTECODE_DISASSEMBLER_H

#include <iomanip>
#include <string>
#include <sstream>

#include "bytecode.h"

namespace Velo::Bytecode {
    class Disassembler final {
    public:
        [[nodiscard]] auto disassemble(const Module &module) const -> std::string {
            std::ostringstream stream;

            for (std::size_t idx = 0U; idx < module.functions.size(); ++idx) {
                if (idx > 0U) {
                    stream << "\n";
                }

                disassembleFunction(module.functions[idx], stream);
            }

            return stream.str();
        }

    private:
        void disassembleFunction(const Function &function, std::ostringstream &stream) const {
            stream << "fn " << function.name;

            if (!function.parameters.empty()) {
                stream << "(";

                for (std::size_t idx = 0U; idx < function.parameters.size(); ++idx) {
                    if (idx > 0U) {
                        stream << ", ";
                    }

                    stream << function.parameters[idx];
                }

                stream << ")";
            }

            stream << "\n";

            for (std::size_t idx = 0U; idx < function.instructions.size(); ++idx) {
                stream << std::setw(4) << std::setfill('0') << idx << " ";
                stream << std::setfill(' ');
                disassembleInstruction(function.instructions[idx], stream);
                stream << "\n";
            }
        }

        void disassembleInstruction(const Instruction &instruction, std::ostringstream &stream) const {
            stream << opCodeToString(instruction.code);

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
                case OpCode::StoreLocal:
                    stream << " local[" << instruction.indexOperand << "]";
                    break;

                case OpCode::Jump:
                case OpCode::JumpIfFalse:
                    stream << " target=" << instruction.targetOperand;
                    break;

                case OpCode::CallBuiltin:
                case OpCode::CallFunction:
                    stream << " " << instruction.stringOperand << " args=" << instruction.argsCount;
                    break;

                case OpCode::BuildStruct:
                    stream << " " << instruction.stringOperand << " fields=" << instruction.argsCount;
                    break;

                case OpCode::BuildArray:
                    stream << " elements=" << instruction.argsCount;
                    break;

                case OpCode::LoadField:
                    stream << " " << instruction.stringOperand;
                    break;

                case OpCode::StoreFieldPath:
                    stream << " " << instruction.stringOperand;
                    break;

                case OpCode::StoreIndexPath:
                    stream << " indexes=" << instruction.argsCount;
                    break;

                default:
                    break;
            }
        }
    };
}

#endif //INC_VELO_BYTECODE_DISASSEMBLER_H
