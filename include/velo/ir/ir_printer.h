#ifndef INC_VELO_IR_IR_PRINTER_H
#define INC_VELO_IR_IR_PRINTER_H

#include <iosfwd>
#include <string>
#include <string_view>

#include "module.h"
#include "function.h"
#include "instruction.h"

namespace Velo::IR {
    class IRPrinter final {
    public:
        [[nodiscard]] auto print(const Module &module) const -> std::string;

    private:
        void printFunction(std::ostream &stream, const Function &func) const;
        void printInstruction(std::ostream &stream, std::size_t index, const Instruction &instruction) const;

        [[nodiscard]] static auto opCodeToString(OpCode code) -> std::string_view;
    };
}

#endif //INC_VELO_IR_IR_PRINTER_H
