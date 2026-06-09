#include <gtest/gtest.h>

#include "velo/bytecode/disassembler.h"

TEST(BytecodeDisassemblerTest, DisassemblesSimpleFunction) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 42,
    });

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Disassembler disassembler;
    const auto text = disassembler.disassemble(module);

    EXPECT_NE(text.find("fn main"), std::string::npos);
    EXPECT_NE(text.find("0000 PushInt 42"), std::string::npos);
    EXPECT_NE(text.find("0001 Return"), std::string::npos);
}

TEST(BytecodeDisassemblerTest, DisassemblesLocalOperands) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::StoreLocal,
        .indexOperand = 0U,
    });

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::LoadLocal,
        .indexOperand = 0U,
    });

    module.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Disassembler disassembler;
    const auto text = disassembler.disassemble(module);

    EXPECT_NE(text.find("0000 StoreLocal local[0]"), std::string::npos);
    EXPECT_NE(text.find("0001 LoadLocal local[0]"), std::string::npos);
}

TEST(BytecodeDisassemblerTest, DisassemblesArrayInstructions) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::BuildArray,
        .argsCount = 3U,
    });

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::LoadIndex,
    });

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::StoreIndexPath,
        .argsCount = 2U,
    });

    module.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Disassembler disassembler;
    const auto text = disassembler.disassemble(module);

    EXPECT_NE(text.find("0000 BuildArray elements=3"), std::string::npos);
    EXPECT_NE(text.find("0001 LoadIndex"), std::string::npos);
    EXPECT_NE(text.find("0002 StoreIndexPath indexes=2"), std::string::npos);
}

TEST(BytecodeDisassemblerTest, DisassemblesBuiltinCall) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::CallBuiltin,
        .stringOperand = "array::len",
        .argsCount = 1U,
    });

    module.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Disassembler disassembler;
    const auto text = disassembler.disassemble(module);

    EXPECT_NE(text.find("0000 CallBuiltin array::len args=1"), std::string::npos);
}