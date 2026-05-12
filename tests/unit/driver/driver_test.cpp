#include <gtest/gtest.h>
#include <string>
#include "velo/driver/driver.h"

using Velo::Driver::Driver;

TEST(DriverTest, ParsesSourceTextAndReturnsASTText) {
    Driver driver;
    const auto result = driver.parseText(
        "hello.velo",
        R"(module app;
use std::console;
fn main(): int {
    console::println("Hello, Velo!");
    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.error.empty());
    ASSERT_TRUE(result.diagnostics.empty());

    EXPECT_NE(result.astText.find("Program"), std::string::npos);
    EXPECT_NE(result.astText.find("Module app"), std::string::npos);
    EXPECT_NE(result.astText.find("Call console::println"), std::string::npos);
}

TEST(DriverTest, ReturnsDiagnosticsForInvalidProgram) {
    Driver driver;

    const auto result = driver.parseText(
        "broken.velo",
        R"(module app;
use std::console
fn main(): int {
    return 0;
}
)"
    );

    ASSERT_FALSE(result.success);
    ASSERT_TRUE(result.error.empty());
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().code(), "PAR006");
}

TEST(DriverTest, ReturnsSemanticDiagnosticsForUnknownQualifier) {
    Driver driver;
    const auto result = driver.parseText(
        "broken_semantic.velo",
        R"(module app;
fn main(): int {
    console::println("Hello, Velo!");
    return 0;
}
)"
    );

    ASSERT_FALSE(result.success);
    ASSERT_TRUE(result.error.empty());
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().code(), "SEM008");
}

TEST(DriverTest, ReturnsErrorMessageWhenFileCannotBeLoaded) {
    Driver driver;
    const auto result = driver.parseFile("this_file_should_not_exists.velo");

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.error.empty());
    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesUserDefinedFunctionCall) {
    Driver driver;
    const auto result = driver.parseText(
        "functions.velo",
        R"(module app;
use std::console;

fn helper(): int {
    console::println("from helper");
    return 0;
}

fn main(): int {
    helper();
    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.error.empty());
    ASSERT_TRUE(result.diagnostics.empty());

    EXPECT_NE(result.astText.find("Function helper -> int"), std::string::npos);
    EXPECT_NE(result.astText.find("Call helper"), std::string::npos);
}

TEST(DriverTest, DoesNotPolluteStackAfterFunctionCall) {
    Driver driver;
    const auto result = driver.parseText(
        "stack.velo",
        R"(module app;
use std::console;

fn helper(): int {
    return 123;
}

fn main(): int {
    helper(); // should not break stack
    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.error.empty());
}

TEST(DriverTest, ReturnsValueFromUserFunction) {
    Driver driver;
    const auto result = driver.parseText(
        "return.velo",
        R"(module app;

fn value(): int {
    return 42;
}

fn main(): int {
    return value();
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.error.empty());
}

TEST(DriverTest, ReturnsFunctionParameterValue) {
    Driver driver;
    const auto result = driver.parseText(
        "identity.velo",
        R"(module app;

fn identity(val: int): int {
    return val;
}

fn main(): int {
    return identity(42);
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.error.empty());
    ASSERT_TRUE(result.diagnostics.empty());
}

TEST(DriverTest, ExecutesIntegerAdditionThroughUserFunction) {
    Driver driver;
    const auto result = driver.parseText(
        "add.velo",
        R"(module app;

fn add(a: int, b: int): int {
    return a + b;
}

fn main(): int {
    return add(20, 22);
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.error.empty());
    ASSERT_TRUE(result.diagnostics.empty());
}

TEST(DriverTest, ReturnsMissingReturnInNonVoidFunction) {
    Driver driver;
    const auto result = driver.parseText(
        "missing_return.velo" ,
        R"(module app;
fn broken(): int {
    42;
}

fn main(): int {
    return 0;
}
)"
    );

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.diagnostics.empty());

    EXPECT_EQ(result.diagnostics.front().code(), "SEM017");
}

TEST(DriverTest, ExecutesLocalVariableProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "locals.velo",
        R"(module app;
fn main(): int {
    let x: int = 42;
    return x;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
}

TEST(DriverTest, ExecutesMutableVariableProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "var.velo",
        R"(module app;
fn main(): int {
    var x: int = 1;
    x = x + 41;
    return x;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
}

TEST(DriverTest, ExecutesIfElseProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "if.velo",
        R"(module app;
fn main(): int {
    if (true) {
        return 42;
    } else {
        return 0;
    }
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
}

TEST(DriverTest, ExecutesComparisonProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "comparison.velo",
        R"(module app;
fn main(): int {
    if (42 > 10) {
        return 1;
    } else {
        return 0;
    }
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());
    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesWhileLoopProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "while.velo",
        R"(module app;
fn main(): int {
    var x: int = 0;
    while(x < 5) {
        x = x + 1;
    }

    return x;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());
    EXPECT_EQ(result.exitCode, 5);
}

TEST(DriverTest, ExecutesBreakStatementInLoop) {
    Driver driver;
    const auto result = driver.parseText(
        "break.velo",
        R"(module app;
fn main(): int {
    var x: int = 0;

    while(true) {
        x = x + 1;
        break;
    }

    return x;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());
    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesContinueStatementInLoop) {
    Driver driver;
    const auto result = driver.parseText(
        "break.velo",
        R"(module app;
fn main(): int {
    var x: int = 0;
    var y: int = 0;

    while(x < 3) {
        x = x + 1;
        continue;
        y = y + 1;
    }

    return y;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());
    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, ExecutesLogicalOperatorProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "logical.velo",
        R"(module app;
fn main(): int {
    let x: int = 5;
    if (x > 0 && x < 10) {
        return 1;
    }

    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());
    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesBoolParameterProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "bool_parameter.velo",
        R"(module app;
fn identity(val: bool): bool {
    return val;
}

fn main(): int {
    if (identity(true)) {
        return 1;
    }

    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());
    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ReportsUnknownDeclaredType) {
    Driver driver;
    const auto result = driver.parseText(
        "unknown_type.velo",
        R"(module app;
fn broken(value: mystery): int {
    return 0;
}

fn main(): int {
    return 0;
}
)"
    );

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().code(), "SEM030");
}

TEST(DriverTest, ReturnsInterpreterExitCode) {
    Driver driver;
    const auto result = driver.parseText(
        "exit_code.velo",
        R"(module app;
fn main(): int {
    return 42;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());
    EXPECT_EQ(result.exitCode, 42);
}

TEST(DriverTest, ReturnsExitCodeOneForSemanticError) {
    Driver driver;
    const auto result = driver.parseText(
        "semantic_error.velo",
        R"(module app;
fn main(): int {
    return missing;
}
)"
    );

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.diagnostics.empty());

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ReturnsExitCodeOneWhenFileCannotBeLoaded) {
    Driver driver;
    const auto result = driver.parseFile("missing_file.velo");
    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.error.empty());
    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesArithmeticExpressionProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "arithmetic.velo",
        R"(module app;
fn main(): int {
    return (1 + 2) * 3 - 4 / 2;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 7);
}

TEST(DriverTest, ShortCircuitsLogicalAnd) {
    Driver driver;
    const auto result = driver.parseText(
        "short_circuit_and.velo",
        R"(module app;
fn explode(): bool {
    return 1 / 0 == 0;
}

fn main(): int {
    if (false && explode()) {
        return 1;
    }

    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriveTest, ShortCircuitsLogicalOr) {
    Driver driver;
    const auto result = driver.parseText(
        "short_circuit_or.velo",
        R"(module app;
fn explode(): bool {
    return 1 / 0 == 0;
}

fn main(): int {
    if (true || explode()) {
        return 7;
    }

    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 7);
}

TEST(DriverTest, EvaluatesRightSideOfLogicalAndWhenLeftIsTrue) {
    Driver driver;
    const auto result = driver.parseText(
        "logical_and_right.velo",
        R"(module app;
fn yes(): bool {
    return true;
}

fn main(): int {
    if (true && yes()) {
        return 3;
    }

    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 3);
}

TEST(DriverTest, EvaluatesRightSideOfLogicalOrWhenLeftIsFalse) {
    Driver driver;
    const auto result = driver.parseText(
        "logical_or_right.velo",
        R"(module app;
fn yes(): bool {
    return true;
}

fn main(): int {
    if (false || yes()) {
        return 4;
    }

    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 4);
}

TEST(DriverTest, CheckModeDoesNotExecuteProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "check_mode.velo",
        R"(module app;
fn main(): int {
    return 1 / 0;
}
)",
        Velo::Driver::DriverMode::Check
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 0);
    EXPECT_TRUE(result.astText.empty());
}

TEST(DriverTest, CheckModeReportsSemanticErrors) {
    Driver driver;
    const auto result = driver.parseText(
        "check_semantic_error.velo",
        R"(module app;
fn main(): int {
    return missing;
}
)",
        Velo::Driver::DriverMode::Check
    );

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.diagnostics.empty());

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, AstModeReturnsAstTextWithoutExecutingProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "ast_mode.velo",
        R"(module app;
fn main(): int {
    return 1 / 0;
}
)",
        Velo::Driver::DriverMode::Ast
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 0);
    EXPECT_FALSE(result.astText.empty());
}

TEST(DriverTest, RunModeExecutesProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "run_mode.velo",
        R"(module app;
fn main(): int {
    return 42;
}
)",
        Velo::Driver::DriverMode::Run
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 42);
}

TEST(DriverTest, IrModeReturnsIrTextWithoutExecutingProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "ir_mode.velo",
        R"(module app;
fn main(): int {
    return 1 / 0;
}
)",
        Velo::Driver::DriverMode::Ir
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 0);
    EXPECT_TRUE(result.astText.empty());
    EXPECT_FALSE(result.irText.empty());
}

TEST(DriverTest, IrModePrintsExpectedInstructions) {
    Driver driver;
    const auto result = driver.parseText(
        "ir_print.velo",
        R"(module app;
fn main(): int {
    return (1 + 2) * 3;
}
)",
        Velo::Driver::DriverMode::Ir
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_NE(result.irText.find("IRModule"), std::string::npos);
    EXPECT_NE(result.irText.find("Function main"), std::string::npos);
    EXPECT_NE(result.irText.find("PushInt 1"), std::string::npos);
    EXPECT_NE(result.irText.find("PushInt 2"), std::string::npos);
    EXPECT_NE(result.irText.find("AddInt"), std::string::npos);
    EXPECT_NE(result.irText.find("PushInt 3"), std::string::npos);
    EXPECT_NE(result.irText.find("MulInt"), std::string::npos);
    EXPECT_NE(result.irText.find("Return"), std::string::npos);
}

TEST(DriverTest, IrModuleResolvesBuiltinImportAlias) {
    Driver driver;
    const auto result = driver.parseText(
        "string_len_ir_alias.velo",
        R"(module app;
use std::string as str;

fn main(): int {
    return str::len("hello");
}
)",
        Velo::Driver::DriverMode::Ir
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_NE(result.irText.find("CallBuiltin string::len args=1"), std::string::npos);
    EXPECT_EQ(result.irText.find("CallBuiltin str::len args=1"), std::string::npos);
}

TEST(DriverTest, ExecutesStringLengthBuiltinProgram) {
    Driver driver;

    const auto result = driver.parseText(
        "string_len.velo",
        R"(module app;
use std::string as str;

fn main(): int {
    return str::len("hello");
}
)"
    );

    if (!result.success) {
        ADD_FAILURE() << "Driver error: " << result.error;
        for (const auto &diag : result.diagnostics) {
            ADD_FAILURE() << diag.code() << ": " << diag.message();
        }
    }

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 5);
}

TEST(DriverTest, ExecutesStringLengthBuiltinThroughAlias) {
    Driver driver;

    const auto result = driver.parseText(
        "string_len.velo",
        R"(module app;
use std::string as text;

fn main(): int {
    return text::len("hello");
}
)"
    );

    if (!result.success) {
        ADD_FAILURE() << "Driver error: " << result.error;
        for (const auto &diag : result.diagnostics) {
            ADD_FAILURE() << diag.code() << ": " << diag.message();
        }
    }

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 5);
}

TEST(DriverTest, ReportsStringLengthArgumentTypeMismatch) {
    Driver driver;

    const auto result = driver.parseText(
        "string_len_type_mismatch.velo",
        R"(module app;
use std::string as str;

fn main(): int {
    return str::len(123);
}
)"
    );

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.diagnostics.empty());

    EXPECT_EQ(result.diagnostics.front().code(), "SEM033");
    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesIntToStringBuiltinProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "int_to_string.velo",
        R"(module app;
use std::int as ints;
use std::string as str;

fn main(): int {
    return str::len(ints::toString(123));
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 3);
}

TEST(DriverTest, ExecutesBoolToStringBuiltinProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "bool_to_string.velo",
        R"(module app;
use std::bool as bools;
use std::string as str;

fn main(): int {
    return str::len(bools::toString(false));
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 5);
}

TEST(DriverTest, IrModuleResolvesStdIntAlias) {
    Driver driver;
    const auto result = driver.parseText(
        "int_to_string_ir_alias.velo",
        R"(module app;
use std::int as ints;
use std::string as str;

fn main(): int {
    return str::len(ints::toString(123));
}
)",
        Velo::Driver::DriverMode::Ir
    );

    if (!result.success) {
        ADD_FAILURE() << "Driver error: " << result.error;
        for (const auto &diag : result.diagnostics) {
            ADD_FAILURE() << diag.code() << ": " << diag.message();
        }
    }

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_NE(result.irText.find("CallBuiltin int::toString args=1"), std::string::npos);
    EXPECT_EQ(result.irText.find("CallBuiltin ints::toString args=1"), std::string::npos);
}

TEST(DriverTest, CheckModeAcceptsStructTypeUsageInFunctionSignature) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_type_usage.velo",
        R"(module app;
struct User {
    id: int;
}

fn identity(user: User): User {
    return user;
}

fn main(): int {
    return 0;
}
)",
        Velo::Driver::DriverMode::Check
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, CheckModeReportsStructFunctionArgumentMismatch) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_argument_mismatch.velo",
        R"(module app;
struct User {
    id: int;
}

fn accept(user: User): int {
    return 1;
}

fn main(): int {
    return accept(123);
}
)",
        Velo::Driver::DriverMode::Check
    );

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.diagnostics.empty());

    EXPECT_EQ(result.exitCode, 1);
    EXPECT_EQ(result.diagnostics.front().code(), "SEM037");
}

TEST(DriverTest, CheckModeAcceptsStructLiteral) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_literal.velo",
        R"(module app;
struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: 1,
        name: "Alex"
    };

    return 0;
}
)",
        Velo::Driver::DriverMode::Check
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, IrModePrintsStructLiteralBuildInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_literal_ir.velo",
        R"(module app;
struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: 1,
        name: "Alex"
    };

    return 0;
}
)",
        Velo::Driver::DriverMode::Ir
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_NE(result.irText.find("BuildStruct User:id,name fields=2"), std::string::npos);
}

TEST(DriverTest, RunModeExecutesStructLiteralProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_literal_run.velo",
        R"(module app;
use std::console;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: 1,
        name: "Alex"
    };

    console::println("struct literal works");

    return 0;
}
)"
    );

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, ExecutesStructFieldAccess) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_field_access.velo",
        R"(module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: 42,
        name: "Alex"
    };

    return user.id;
}
)"
    );

    if (!result.success) {
        ADD_FAILURE() << "Driver error: " << result.error;
        for (const auto &diag : result.diagnostics) {
            ADD_FAILURE() << diag.code() << ": " << diag.message();
        }
    }

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_EQ(result.exitCode, 42);
}

TEST(DriverTest, IrModePrintsLoadFieldInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_field_access_ir.velo",
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    let user: User = User {
        id: 7
    };

    return user.id;
}
)",
        Velo::Driver::DriverMode::Ir
    );

    if (!result.success) {
        ADD_FAILURE() << "Driver error: " << result.error;
        for (const auto &diag : result.diagnostics) {
            ADD_FAILURE() << diag.code() << ": " << diag.message();
        }
    }

    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.diagnostics.empty());
    ASSERT_TRUE(result.error.empty());

    EXPECT_NE(result.irText.find("LoadField id"), std::string::npos);
}