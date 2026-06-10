#include <gtest/gtest.h>

#include "velo/bytecode/file_format.h"
#include "velo/bytecode/vm.h"
#include "velo/runtime/runtime.h"

TEST(BytecodeFileFormatTest, WritesMagicAndVersion) {
    Velo::Bytecode::Module module;

    const Velo::Bytecode::FileFormat format;
    const auto text = format.write(module);

    EXPECT_NE(text.find("VELO_BYTECODE_TEXT 1"), std::string::npos);
}

TEST(BytecodeFileFormatTest, RoundTripsSimpleFunction) {
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

    const Velo::Bytecode::FileFormat format;
    const auto text = format.write(module);
    const auto readResult = format.read(text);

    ASSERT_TRUE(readResult.success);
    EXPECT_TRUE(readResult.error.empty());

    ASSERT_EQ(readResult.module.functions.size(), 1U);

    const auto &function = readResult.module.functions[0];
    EXPECT_EQ(function.name, "main");
    ASSERT_EQ(function.instructions.size(), 2U);

    EXPECT_EQ(function.instructions[0].code, Velo::Bytecode::OpCode::PushInt);
    EXPECT_EQ(function.instructions[0].intOperand, 42);
    EXPECT_EQ(function.instructions[1].code, Velo::Bytecode::OpCode::Return);
}

TEST(BytecodeFileFormatTest, RoundTripsFunctionParameters) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function function;
    function.name = "add";
    function.parameters.push_back("left");
    function.parameters.push_back("right");

    function.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::LoadLocal,
        .indexOperand = 0U,
    });
    function.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::LoadLocal,
        .indexOperand = 1U,
    });
    function.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::AddInt,
    });
    function.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    module.functions.push_back(std::move(function));

    const Velo::Bytecode::FileFormat format;
    const auto readResult = format.read(format.write(module));

    ASSERT_TRUE(readResult.success);
    ASSERT_EQ(readResult.module.functions.size(), 1U);

    const auto &roundTripped = readResult.module.functions[0];
    EXPECT_EQ(roundTripped.name, "add");

    ASSERT_EQ(roundTripped.parameters.size(), 2U);
    EXPECT_EQ(roundTripped.parameters[0], "left");
    EXPECT_EQ(roundTripped.parameters[1], "right");

    ASSERT_EQ(roundTripped.instructions.size(), 4U);
    EXPECT_EQ(roundTripped.instructions[0].code, Velo::Bytecode::OpCode::LoadLocal);
    EXPECT_EQ(roundTripped.instructions[0].indexOperand, 0U);
    EXPECT_EQ(roundTripped.instructions[1].code, Velo::Bytecode::OpCode::LoadLocal);
    EXPECT_EQ(roundTripped.instructions[1].indexOperand, 1U);
}

TEST(BytecodeFileFormatTest, RoundTripsStringAndBoolOperands) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushString,
        .stringOperand = "hello bytecode",
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushBool,
        .boolOperand = true,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::FileFormat format;
    const auto readResult = format.read(format.write(module));

    ASSERT_TRUE(readResult.success);
    ASSERT_EQ(readResult.module.functions.size(), 1U);

    const auto &instructions = readResult.module.functions[0].instructions;
    ASSERT_EQ(instructions.size(), 3U);

    EXPECT_EQ(instructions[0].code, Velo::Bytecode::OpCode::PushString);
    EXPECT_EQ(instructions[0].stringOperand, "hello bytecode");

    EXPECT_EQ(instructions[1].code, Velo::Bytecode::OpCode::PushBool);
    EXPECT_TRUE(instructions[1].boolOperand);
}

TEST(BytecodeFileFormatTest, RoundTripsArrayAndBuiltinInstructions) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 1,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 2,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 3,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::BuildArray,
        .argsCount = 3U,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::CallBuiltin,
        .stringOperand = "array::len",
        .argsCount = 1U,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::FileFormat format;
    const auto readResult = format.read(format.write(module));

    ASSERT_TRUE(readResult.success);
    ASSERT_EQ(readResult.module.functions.size(), 1U);

    const auto &instructions = readResult.module.functions[0].instructions;
    ASSERT_EQ(instructions.size(), 6U);

    EXPECT_EQ(instructions[3].code, Velo::Bytecode::OpCode::BuildArray);
    EXPECT_EQ(instructions[3].argsCount, 3U);

    EXPECT_EQ(instructions[4].code, Velo::Bytecode::OpCode::CallBuiltin);
    EXPECT_EQ(instructions[4].stringOperand, "array::len");
    EXPECT_EQ(instructions[4].argsCount, 1U);
}

TEST(BytecodeFileFormatTest, VmExecutesRoundTrippedModule) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 40,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 2,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::AddInt,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::FileFormat format;
    const auto readResult = format.read(format.write(module));

    ASSERT_TRUE(readResult.success);

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto executeResult = vm.execute(readResult.module);

    ASSERT_TRUE(executeResult.success);
    EXPECT_EQ(executeResult.exitCode, 42);
    EXPECT_TRUE(executeResult.error.empty());
}

TEST(BytecodeFileFormatTest, RejectsInvalidMagic) {
    const Velo::Bytecode::FileFormat format;
    const auto readResult = format.read(
        R"(NOT_BYTECODE 1
)"
    );

    EXPECT_FALSE(readResult.success);
    EXPECT_NE(readResult.error.find("magic"), std::string::npos);
}

TEST(BytecodeFileFormatTest, RejectsInvalidInstructionOpcode) {
    const Velo::Bytecode::FileFormat format;
    const auto readResult = format.read(
        R"(VELO_BYTECODE_TEXT 1
function "main" 0
instruction MissingOpcode "" 0 0 0 0 0
endfunction
)"
    );

    EXPECT_FALSE(readResult.success);
    EXPECT_NE(readResult.error.find("Invalid instruction"), std::string::npos);
}