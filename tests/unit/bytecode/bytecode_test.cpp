#include <gtest/gtest.h>

#include  "velo/bytecode/bytecode.h"

TEST(BytecodeTest, CreatesInstructionWithOperands) {
    Velo::Bytecode::Instruction instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 42,
    };

    EXPECT_EQ(instruction.code, Velo::Bytecode::OpCode::PushInt);
    EXPECT_EQ(instruction.intOperand, 42);
}

TEST(BytecodeTest, CreatesFunctionWithInstructions) {
    Velo::Bytecode::Function func;
    func.name = "main";

    func.instructions.push_back(Velo::Bytecode::Instruction {
            .code = Velo::Bytecode::OpCode::PushInt,
            .intOperand = 42,
        });
    func.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    EXPECT_EQ(func.name, "main");
    ASSERT_EQ(func.instructions.size(), 2U);
    EXPECT_EQ(func.instructions[0].code, Velo::Bytecode::OpCode::PushInt);
    EXPECT_EQ(func.instructions[1].code, Velo::Bytecode::OpCode::Return);
}

TEST(BytecodeTest, CreatesModuleWithFunctions) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function func;
    func.name = "main";

    module.functions.push_back(std::move(func));

    ASSERT_EQ(module.functions.size(), 1U);
    EXPECT_EQ(module.functions[0].name, "main");
}

TEST(BytecodeTest, ConvertsOpCodeToString) {
    EXPECT_EQ(
        Velo::Bytecode::opCodeToString(Velo::Bytecode::OpCode::PushInt),
        "PushInt"
    );

    EXPECT_EQ(
        Velo::Bytecode::opCodeToString(Velo::Bytecode::OpCode::BuildArray),
        "BuildArray"
    );

    EXPECT_EQ(
        Velo::Bytecode::opCodeToString(Velo::Bytecode::OpCode::StoreIndexPath),
        "StoreIndexPath"
    );

    EXPECT_EQ(
        Velo::Bytecode::opCodeToString(Velo::Bytecode::OpCode::Return),
        "Return"
    );
}