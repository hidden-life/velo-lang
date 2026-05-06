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
        .intOperand = 0,
        .argsCount = 0U
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

TEST(InterpreterTest, ReportsWrongBuiltinArgumentCount) {
    Module module;

    Function mainFunc;
    mainFunc.name = "main";
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushString,
        .stringOperand = "hello",
        .intOperand = 0,
        .argsCount = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::CallBuiltin,
        .stringOperand = "console::println",
        .intOperand = 0,
        .argsCount = 2U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .stringOperand = {},
        .intOperand = 0,
        .argsCount = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
        .stringOperand = {},
        .intOperand = 0,
        .argsCount = 0U
    });

    module.functions.push_back(std::move(mainFunc));

    Runtime runtime;
    Interpreter interpreter(runtime);
    const auto result = interpreter.execute(module);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_FALSE(result.error.empty());
}

TEST(InterpreterTest, ExecutesUserDefinedFunctionCall) {
    Module module;
    Function helperFunc;
    helperFunc.name = "helper";
    helperFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .stringOperand = {},
        .intOperand = 0,
        .argsCount = 0U
    });
    helperFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
        .stringOperand = {},
        .intOperand = 0,
        .argsCount = 0U
    });

    Function mainFunc;
    mainFunc.name = "main";
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::CallFunction,
        .stringOperand = "helper",
        .intOperand = 0,
        .argsCount = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .stringOperand = {},
        .intOperand = 0,
        .argsCount = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
        .stringOperand = {},
        .intOperand = 0,
        .argsCount = 0U
    });

    module.functions.push_back(std::move(helperFunc));
    module.functions.push_back(std::move(mainFunc));

    Runtime runtime;
    Interpreter interpreter(runtime);

    const auto result = interpreter.execute(module);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_TRUE(result.error.empty());
}

TEST(InterpreterTest, CLeansStackAfterExpressionStatement) {
    Module module;
    Function mainFunc;
    mainFunc.name = "main";

    // helper-like call
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 42,
        .argsCount = 0U
    });

    // pop should remove this value
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Pop,
        .argsCount = 0U
    });

    // return normal code
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 0,
        .argsCount = 0U
    });

    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
        .argsCount = 0U
    });

    module.functions.push_back(std::move(mainFunc));

    Runtime runtime;
    Interpreter interpreter(runtime);

    const auto result = interpreter.execute(module);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 0);
}

TEST(InterpreterTest, PropagatesReturnValueFromFunction) {
    Module module;
    // value() -> return 42
    Function valueFn;
    valueFn.name = "value";
    valueFn.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 42,
        .argsCount = 0U
    });
    valueFn.instructions.push_back(Instruction {
        .code = OpCode::Return,
        .argsCount = 0U
    });

    // main() -> return value()
    Function mainFunc;
    mainFunc.name = "main";
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::CallFunction,
        .stringOperand = "value",
        .argsCount = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
        .argsCount = 0U
    });

    module.functions.push_back(std::move(valueFn));
    module.functions.push_back(std::move(mainFunc));

    Runtime runtime;
    Interpreter interpreter(runtime);

    const auto result = interpreter.execute(module);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 42);
}

TEST(InterpreterTest, LoadsFunctionParameterAsLocalValue) {
    Module module;

    Function identityFunc;
    identityFunc.name = "identity";
    identityFunc.parameters = {"value"};
    identityFunc.instructions.push_back(Instruction {
        .code = OpCode::LoadLocal,
        .indexOperand = 0U
    });
    identityFunc.instructions.push_back(Instruction {
        .code = OpCode::Return
    });

    Function mainFunc;
    mainFunc.name = "main";
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 42,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::CallFunction,
        .stringOperand = "identity",
        .argsCount = 1U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
    });

    module.functions.push_back(std::move(identityFunc));
    module.functions.push_back(std::move(mainFunc));

    Runtime runtime;
    Interpreter interpreter(runtime);

    const auto result = interpreter.execute(module);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 42);
    EXPECT_TRUE(result.error.empty());
}

TEST(InterpreterTest, ExecutesIntegerAddition) {
    Module module;

    Function mainFunc;
    mainFunc.name = "main";
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 20
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 22,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::AddInt,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunc));

    Runtime runtime;
    Interpreter interpreter(runtime);
    const auto result = interpreter.execute(module);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 42);
    EXPECT_TRUE(result.error.empty());
}

TEST(InterpreterTest, ExecutesIntegerGreaterComparison) {
    Module module;
    Function mainFunc;
    mainFunc.name = "main";
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 42,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 10,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::CompareGreaterInt,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::JumpIfFalse,
        .targetOperand = 6U,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 1,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 0,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunc));
    Runtime runtime;
    Interpreter interpreter(runtime);
    const auto result = interpreter.execute(module);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_TRUE(result.error.empty());
}

TEST(InterpreterTest, ExecutesSimpleWhileLikeJumpLoop) {
    Module module;
    Function mainFunc;
    mainFunc.name = "main";

    // local x = 0
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 0,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::StoreLocal,
        .indexOperand = 0U
    });
    // condition_start index = 2
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::LoadLocal,
        .indexOperand = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 3,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::CompareLessInt,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::JumpIfFalse,
        .targetOperand = 11U,
    });
    // x = x + 1
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::LoadLocal,
        .indexOperand = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 1,
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::AddInt
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::StoreLocal,
        .indexOperand = 0U
    });
    // jump to condition start
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Jump,
        .targetOperand = 2U
    });
    // return x
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::LoadLocal,
        .indexOperand = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return,
    });

    module.functions.push_back(std::move(mainFunc));
    Runtime runtime;
    Interpreter interpreter(runtime);
    const auto result = interpreter.execute(module);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 3);
    EXPECT_TRUE(result.error.empty());
}

TEST(InterpreterTest, ExecutesBreakLikeJumpOutOfLoop) {
    Module module;
    Function mainFunc;
    mainFunc.name = "main";

    // x = 0
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 0
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::StoreLocal,
        .indexOperand = 0U
    });

    // loop condition: true
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushBool,
        .boolOperand = true
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::JumpIfFalse,
        .targetOperand = 8U
    });

    // x = x + 1
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::LoadLocal,
        .indexOperand = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 1
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::AddInt
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::StoreLocal,
        .indexOperand = 0U
    });

    // break; -> jump to return block
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Jump,
        .targetOperand = 10U
    });

    // normal loop back edge, should be skipped because break jumps over it
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Jump,
        .targetOperand = 2U
    });

    // return x
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::LoadLocal,
        .indexOperand = 0U
    });
    mainFunc.instructions.push_back(Instruction {
        .code = OpCode::Return
    });

    module.functions.push_back(std::move(mainFunc));

    Runtime runtime;
    Interpreter interpreter(runtime);

    const auto result = interpreter.execute(module);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_TRUE(result.error.empty());
}

TEST(InterpreterTest, ExecutesLogicalOperators) {
    Module module;

    Function mainFunction;
    mainFunction.name = "main";

    // true && !false => true
    mainFunction.instructions.push_back(Instruction {
        .code = OpCode::PushBool,
        .boolOperand = true
    });
    mainFunction.instructions.push_back(Instruction {
        .code = OpCode::PushBool,
        .boolOperand = false
    });
    mainFunction.instructions.push_back(Instruction {
        .code = OpCode::LogicalNot
    });
    mainFunction.instructions.push_back(Instruction {
        .code = OpCode::LogicalAnd
    });
    mainFunction.instructions.push_back(Instruction {
        .code = OpCode::JumpIfFalse,
        .targetOperand = 7U
    });
    mainFunction.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 1
    });
    mainFunction.instructions.push_back(Instruction {
        .code = OpCode::Return
    });
    mainFunction.instructions.push_back(Instruction {
        .code = OpCode::PushInt,
        .intOperand = 0
    });
    mainFunction.instructions.push_back(Instruction {
        .code = OpCode::Return
    });

    module.functions.push_back(std::move(mainFunction));

    Runtime runtime;
    Interpreter interpreter(runtime);

    const auto result = interpreter.execute(module);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_TRUE(result.error.empty());
}