#include <gtest/gtest.h>

#include "velo/bytecode/compiler.h"
#include "velo/bytecode/vm.h"
#include "velo/runtime/runtime.h"

TEST(BytecodeVmTest, ExecutesMainReturningInt) {
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

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto result = vm.execute(module);

    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 42);
    EXPECT_TRUE(result.error.empty());
}

TEST(BytecodeVmTest, ExecutesIntegerArithmetic) {
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

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto result = vm.execute(module);

    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 42);
    EXPECT_TRUE(result.error.empty());
}

TEST(BytecodeVmTest, ExecutesLocalLoadStore) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 42,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::StoreLocal,
        .indexOperand = 0U,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::LoadLocal,
        .indexOperand = 0U,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunction));

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto result = vm.execute(module);

    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 42);
    EXPECT_TRUE(result.error.empty());
}

TEST(BytecodeVmTest, ExecutesJumpIfFalse) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushBool,
        .boolOperand = false,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::JumpIfFalse,
        .targetOperand = 4U,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 1,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 42,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunction));

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto result = vm.execute(module);

    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 42);
    EXPECT_TRUE(result.error.empty());
}

TEST(BytecodeVmTest, ExecutesUserFunctionCall) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function answerFunction;
    answerFunction.name = "answer";
    answerFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 42,
    });
    answerFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::CallFunction,
        .stringOperand = "answer",
        .argsCount = 0U,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    module.functions.push_back(std::move(answerFunction));
    module.functions.push_back(std::move(mainFunction));

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto result = vm.execute(module);

    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 42);
    EXPECT_TRUE(result.error.empty());
}

TEST(BytecodeVmTest, ExecutesBuiltinArrayLen) {
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

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto result = vm.execute(module);

    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 3);
    EXPECT_TRUE(result.error.empty());
}

TEST(BytecodeVmTest, ExecutesArrayIndexRead) {
    Velo::Bytecode::Module module;

    Velo::Bytecode::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 10,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 20,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 30,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::BuildArray,
        .argsCount = 3U,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::StoreLocal,
        .indexOperand = 0U,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::LoadLocal,
        .indexOperand = 0U,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::PushInt,
        .intOperand = 1,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::LoadIndex,
    });
    mainFunction.instructions.push_back(Velo::Bytecode::Instruction {
        .code = Velo::Bytecode::OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunction));

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto result = vm.execute(module);

    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 20);
    EXPECT_TRUE(result.error.empty());
}

TEST(BytecodeVmTest, ExecutesCompiledIrModule) {
    Velo::IR::Module irModule;

    Velo::IR::Function mainFunction;
    mainFunction.name = "main";

    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::PushInt,
        .intOperand = 40,
    });
    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::PushInt,
        .intOperand = 2,
    });
    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::AddInt,
    });
    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::Return,
    });

    irModule.functions.push_back(std::move(mainFunction));

    const Velo::Bytecode::Compiler compiler;
    const auto bytecodeModule = compiler.compile(irModule);

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto result = vm.execute(bytecodeModule);

    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 42);
    EXPECT_TRUE(result.error.empty());
}

TEST(BytecodeVmTest, ReportsMissingMainFunction) {
    Velo::Bytecode::Module module;

    Velo::Runtime::Runtime runtime;
    Velo::Bytecode::VM vm(runtime);

    const auto result = vm.execute(module);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_NE(result.error.find("main"), std::string::npos);
}