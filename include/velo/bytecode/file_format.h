#ifndef INC_VELO_BYTECODE_FILE_FORMAT_H
#define INC_VELO_BYTECODE_FILE_FORMAT_H

#include <iomanip>
#include <string>
#include <sstream>

#include "bytecode.h"

namespace Velo::Bytecode {
    struct FileReadResult final {
        bool success {false};
        std::string error {};
        Module module {};
    };

    class FileFormat final {
    public:
        [[nodiscard]] auto write(const Module &module) const -> std::string {
            std::ostringstream stream;

            stream << magic() << " " << version() << "\n";

            for (const auto &func : module.functions) {
                stream << "function " << std::quoted(func.name) << " " << func.parameters.size() << "\n";
                for (const auto &param : func.parameters) {
                    stream << "param " << std::quoted(param) << "\n";
                }

                for (const auto &instr : func.instructions) {
                    stream << "instruction "
                        << opCodeToString(instr.code) << " "
                        << std::quoted(instr.stringOperand) << " "
                        << instr.intOperand << " "
                        << (instr.boolOperand ? 1 : 0) << " "
                        << instr.argsCount << " "
                        << instr.indexOperand << " "
                        << instr.targetOperand << "\n";
                }

                stream << "endfunction\n";
            }

            return stream.str();
        }

        [[nodiscard]] auto read(std::string_view text) const -> FileReadResult {
            std::istringstream input {std::string(text)};
            std::string line;

            if (!std::getline(input, line)) {
                return FileReadResult {
                    .success = false,
                    .error = "Bytecode file is empty.",
                    .module = {}
                };
            }

            std::istringstream headerStream {line};
            std::string fileMagic;
            int fileVersion = 0;
            headerStream >> fileMagic >> fileVersion;

            if (fileMagic != magic()) {
                return FileReadResult {
                    .success = false,
                    .error = "Invalid bytecode file magic.",
                    .module = {}
                };
            }

            if (fileVersion != version()) {
                return FileReadResult {
                    .success = false,
                    .error = "Invalid bytecode file version.",
                    .module = {}
                };
            }

            Module module;
            std::size_t lineNumber = 1U;

            while (std::getline(input, line)) {
                ++lineNumber;
                if (line.empty()) {
                    continue;
                }

                std::istringstream funcStream {line};
                std::string tag;
                funcStream >> tag;

                if (tag != "function") {
                    return FileReadResult {
                        .success = false,
                        .error = "Expected function declaration at line " + std::to_string(lineNumber) + ".",
                        .module = {}
                    };
                }

                Function function;
                std::size_t paramCount = 0U;

                if (!(funcStream >> std::quoted(function.name) >> paramCount)) {
                    return FileReadResult {
                        .success = false,
                        .error = "Invalid function declaration at line " + std::to_string(lineNumber) + ".",
                        .module = {}
                    };
                }

                for (std::size_t paramIdx = 0U; paramIdx < paramCount; ++paramIdx) {
                    if (!std::getline(input, line)) {
                        return FileReadResult {
                            .success = false,
                            .error = "Unexpected end of file while reading function parameters.",
                            .module = {}
                        };
                    }

                    ++lineNumber;
                    std::istringstream paramStream {line};
                    std::string paramTag;
                    std::string paramName;

                    if (!(paramStream >> paramTag >> std::quoted(paramName)) || paramTag != "param") {
                        return FileReadResult {
                            .success = false,
                            .error = "Invalid parameter declaration at line " + std::to_string(lineNumber) + ".",
                            .module = {}
                        };
                    }

                    function.parameters.push_back(std::move(paramName));
                }

                while (std::getline(input, line)) {
                    ++lineNumber;

                    if (line == "endfunction") {
                        break;
                    }

                    if (line.empty()) {
                        continue;
                    }

                    auto instructionResult = readInstruction(line);
                    if (!instructionResult.has_value()) {
                        return FileReadResult {
                            .success = false,
                            .error = "Invalid instruction at line " + std::to_string(lineNumber) + ".",
                            .module = {}
                        };
                    }

                    function.instructions.push_back(std::move(*instructionResult));
                }

                if (line != "endfunction") {
                    return FileReadResult {
                        .success = false,
                        .error = "Missing endfunction for function '" + function.name + "'.",
                        .module = {}
                    };
                }

                module.functions.push_back(std::move(function));
            }

            return FileReadResult {
                .success = true,
                .error = {},
                .module = std::move(module)
            };
        }

        [[nodiscard]] static constexpr auto magic() -> std::string_view {
            return "VELO_BYTECODE_TEXT";
        }

        [[nodiscard]] static constexpr auto version() -> int {
            return 1;
        }

    private:
        [[nodiscard]] auto readInstruction(const std::string &line) const -> std::optional<Instruction> {
            std::istringstream stream {line};
            std::string tag;
            std::string opcodeText;
            std::string stringOperand;
            int intOperand = 0;
            int boolOperand = 0;
            std::size_t argsCount = 0U;
            std::size_t indexOperand = 0U;
            std::size_t targetOperand = 0U;

            if (!(stream >>
                tag >>
                opcodeText >>
                std::quoted(stringOperand) >>
                intOperand >>
                boolOperand >>
                argsCount >>
                indexOperand >>
                targetOperand)) {
                return std::nullopt;
            }

            if (tag != "instruction") {
                return std::nullopt;
            }

            const auto opcode = opCodeFromString(opcodeText);
            if (!opcode.has_value()) {
                return std::nullopt;
            }

            return Instruction {
                .code = *opcode,
                .stringOperand = std::move(stringOperand),
                .intOperand = intOperand,
                .boolOperand = boolOperand != 0,
                .argsCount = argsCount,
                .indexOperand = indexOperand,
                .targetOperand = targetOperand
            };
        }

        [[nodiscard]] auto opCodeFromString(const std::string &name) const -> std::optional<OpCode> {
#define VELO_BYTECODE_OPCODE_FROM_STRING(op)    \
    if (name == #op) {                          \
            return OpCode::op;                  \
    }

            VELO_BYTECODE_OPCODE_FROM_STRING(PushInt)
            VELO_BYTECODE_OPCODE_FROM_STRING(PushString)
            VELO_BYTECODE_OPCODE_FROM_STRING(PushBool)

            VELO_BYTECODE_OPCODE_FROM_STRING(LoadLocal)
            VELO_BYTECODE_OPCODE_FROM_STRING(StoreLocal)

            VELO_BYTECODE_OPCODE_FROM_STRING(AddInt)
            VELO_BYTECODE_OPCODE_FROM_STRING(SubInt)
            VELO_BYTECODE_OPCODE_FROM_STRING(MulInt)
            VELO_BYTECODE_OPCODE_FROM_STRING(DivInt)
            VELO_BYTECODE_OPCODE_FROM_STRING(ModInt)
            VELO_BYTECODE_OPCODE_FROM_STRING(NegInt)

            VELO_BYTECODE_OPCODE_FROM_STRING(LogicalNot)
            VELO_BYTECODE_OPCODE_FROM_STRING(LogicalAnd)
            VELO_BYTECODE_OPCODE_FROM_STRING(LogicalOr)

            VELO_BYTECODE_OPCODE_FROM_STRING(CompareEqual)
            VELO_BYTECODE_OPCODE_FROM_STRING(CompareNotEqual)
            VELO_BYTECODE_OPCODE_FROM_STRING(CompareLessInt)
            VELO_BYTECODE_OPCODE_FROM_STRING(CompareGreaterInt)
            VELO_BYTECODE_OPCODE_FROM_STRING(CompareLessEqualInt)
            VELO_BYTECODE_OPCODE_FROM_STRING(CompareGreaterEqualInt)

            VELO_BYTECODE_OPCODE_FROM_STRING(Jump)
            VELO_BYTECODE_OPCODE_FROM_STRING(JumpIfFalse)

            VELO_BYTECODE_OPCODE_FROM_STRING(CallFunction)
            VELO_BYTECODE_OPCODE_FROM_STRING(CallBuiltin)

            VELO_BYTECODE_OPCODE_FROM_STRING(BuildStruct)
            VELO_BYTECODE_OPCODE_FROM_STRING(BuildArray)
            VELO_BYTECODE_OPCODE_FROM_STRING(BuildMap)
            VELO_BYTECODE_OPCODE_FROM_STRING(LoadField)
            VELO_BYTECODE_OPCODE_FROM_STRING(StoreFieldPath)
            VELO_BYTECODE_OPCODE_FROM_STRING(LoadIndex)
            VELO_BYTECODE_OPCODE_FROM_STRING(StoreIndexPath)

            VELO_BYTECODE_OPCODE_FROM_STRING(Return)
            VELO_BYTECODE_OPCODE_FROM_STRING(Pop)

#undef VELO_BYTECODE_OPCODE_FROM_STRING

            return std::nullopt;
        }
    };
}

#endif //INC_VELO_BYTECODE_FILE_FORMAT_H
