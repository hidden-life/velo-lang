#include <gtest/gtest.h>

#include "velo/bytecode/compiler.h"

TEST(BytecodeCompilerTest, CompilesEmptyIrModule) {
    Velo::IR::Module irModule;

    const Velo::Bytecode::Compiler compiler;
    const auto bytecodeModule = compiler.compile(irModule);

    EXPECT_TRUE(bytecodeModule.functions.empty());
}

TEST(BytecodeCompilerTest, CompilesFunctionMetadata) {
    Velo::IR::Module irModule;

    Velo::IR::Function mainFunction;
    mainFunction.name = "main";
    mainFunction.parameters.push_back("argc");

    irModule.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Compiler compiler;
    const auto bytecodeModule = compiler.compile(irModule);

    ASSERT_EQ(bytecodeModule.functions.size(), 1U);

    const auto &compiledMain = bytecodeModule.functions.front();
    EXPECT_EQ(compiledMain.name, "main");

    ASSERT_EQ(compiledMain.parameters.size(), 1U);
    EXPECT_EQ(compiledMain.parameters[0], "argc");
}

TEST(BytecodeCompilerTest, CompilesInstructionOperands) {
    Velo::IR::Module irModule;

    Velo::IR::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::PushInt,
        .intOperand = 42,
    });

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::StoreLocal,
        .indexOperand = 3U,
    });

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::CallBuiltin,
        .stringOperand = "array::len",
        .argsCount = 1U,
    });

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::JumpIfFalse,
        .targetOperand = 9U,
    });

    irModule.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Compiler compiler;
    const auto bytecodeModule = compiler.compile(irModule);

    ASSERT_EQ(bytecodeModule.functions.size(), 1U);

    const auto &instructions = bytecodeModule.functions.front().instructions;
    ASSERT_EQ(instructions.size(), 4U);

    EXPECT_EQ(instructions[0].code, Velo::Bytecode::OpCode::PushInt);
    EXPECT_EQ(instructions[0].intOperand, 42);

    EXPECT_EQ(instructions[1].code, Velo::Bytecode::OpCode::StoreLocal);
    EXPECT_EQ(instructions[1].indexOperand, 3U);

    EXPECT_EQ(instructions[2].code, Velo::Bytecode::OpCode::CallBuiltin);
    EXPECT_EQ(instructions[2].stringOperand, "array::len");
    EXPECT_EQ(instructions[2].argsCount, 1U);

    EXPECT_EQ(instructions[3].code, Velo::Bytecode::OpCode::JumpIfFalse);
    EXPECT_EQ(instructions[3].targetOperand, 9U);
}

TEST(BytecodeCompilerTest, CompilesArrayInstructions) {
    Velo::IR::Module irModule;

    Velo::IR::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::BuildArray,
        .argsCount = 3U,
    });

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::LoadIndex,
    });

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::StoreIndexPath,
        .argsCount = 2U,
    });

    irModule.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Compiler compiler;
    const auto bytecodeModule = compiler.compile(irModule);

    ASSERT_EQ(bytecodeModule.functions.size(), 1U);

    const auto &instructions = bytecodeModule.functions.front().instructions;
    ASSERT_EQ(instructions.size(), 3U);

    EXPECT_EQ(instructions[0].code, Velo::Bytecode::OpCode::BuildArray);
    EXPECT_EQ(instructions[0].argsCount, 3U);

    EXPECT_EQ(instructions[1].code, Velo::Bytecode::OpCode::LoadIndex);

    EXPECT_EQ(instructions[2].code, Velo::Bytecode::OpCode::StoreIndexPath);
    EXPECT_EQ(instructions[2].argsCount, 2U);
}

TEST(BytecodeCompilerTest, CompilesStructInstructions) {
    Velo::IR::Module irModule;

    Velo::IR::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::BuildStruct,
        .stringOperand = "User|id,name",
        .argsCount = 2U,
    });

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::LoadField,
        .stringOperand = "id",
    });

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::StoreFieldPath,
        .stringOperand = "profile.id",
    });

    irModule.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Compiler compiler;
    const auto bytecodeModule = compiler.compile(irModule);

    ASSERT_EQ(bytecodeModule.functions.size(), 1U);

    const auto &instructions = bytecodeModule.functions.front().instructions;
    ASSERT_EQ(instructions.size(), 3U);

    EXPECT_EQ(instructions[0].code, Velo::Bytecode::OpCode::BuildStruct);
    EXPECT_EQ(instructions[0].stringOperand, "User|id,name");
    EXPECT_EQ(instructions[0].argsCount, 2U);

    EXPECT_EQ(instructions[1].code, Velo::Bytecode::OpCode::LoadField);
    EXPECT_EQ(instructions[1].stringOperand, "id");

    EXPECT_EQ(instructions[2].code, Velo::Bytecode::OpCode::StoreFieldPath);
    EXPECT_EQ(instructions[2].stringOperand, "profile.id");
}

TEST(BytecodeCompilerTest, MapsLegacyIntegerEqualityToGenericBytecodeEquality) {
    Velo::IR::Module irModule;

    Velo::IR::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::CompareEqualInt,
    });

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::CompareNotEqualInt,
    });

    irModule.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Compiler compiler;
    const auto bytecodeModule = compiler.compile(irModule);

    ASSERT_EQ(bytecodeModule.functions.size(), 1U);

    const auto &instructions = bytecodeModule.functions.front().instructions;
    ASSERT_EQ(instructions.size(), 2U);

    EXPECT_EQ(instructions[0].code, Velo::Bytecode::OpCode::CompareEqual);
    EXPECT_EQ(instructions[1].code, Velo::Bytecode::OpCode::CompareNotEqual);
}