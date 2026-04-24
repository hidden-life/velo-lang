#include <gtest/gtest.h>

#include "velo/interpreter/interpreter.h"
#include "velo/ir/function.h"
#include "velo/ir/module.h"

using Velo::IR::Function;
using Velo::IR::Instruction;
using Velo::IR::Module;
using Velo::IR::OpCode;
using Velo::Interpreter::Interpreter;
using Velo::Runtime::Runtime;

TEST(InterpreterTest, ExecutesMainAndReturnsExitCode) {
    Module module;

    Function mainFunc;
    mainFunc.name = "main";
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .stringOperand = {},
        .intOperand = 0
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
        .stringOperand = {},
        .intOperand = 0
    });

    module.functions.push_back(std::move(mainFunc));

    Runtime runtime;
    Interpreter interpreter(runtime);
    const auto result = interpreter.execute(module);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_TRUE(result.error.empty());
}

TEST(InterpreterTest, ReportsUnknownBuiltintFunction) {
    Module module;

    Function mainFunc;
    mainFunc.name = "main";
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::CallBuiltin,
        .stringOperand = "missing::func",
        .intOperand = 0
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .stringOperand = {},
        .intOperand = 0
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
        .stringOperand = {},
        .intOperand = 0
    });

    module.functions.push_back(std::move(mainFunc));
    Runtime runtime;
    Interpreter interpreter(runtime);
    const auto result = interpreter.execute(module);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_FALSE(result.error.empty());
}