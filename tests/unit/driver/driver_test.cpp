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

TEST(DriverTest, ExecutesStructValueCopiesThroughLocalsParametersAndReturnValues) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_value_Copy.velo",
        R"(module app;
struct User {
    id: int;
}

fn identity(u: User): User {
    return u;
}

fn getId(u: User): int {
    return u.id;
}

fn main(): int {
    let a: User = User {
        id: 42
    };

    let b: User = a;
    let c: User = identity(b);

    return getId(c);
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

TEST(DriverTest, ExecutesStructFieldAssignment) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_field_assignment.velo",
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    var user: User = User {
        id: 1
    };

    user.id = 42;

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

TEST(DriverTest, ExecutesNestedStructFieldAssignment) {
    Driver driver;
    const auto result = driver.parseText(
        "nested_struct_field_assignment.velo",
        R"(module app;

struct User {
    id: int;
}

struct Box {
    user: User;
}

fn main(): int {
    var box: Box = Box {
        user: User {
            id: 1
        }
    };

    box.user.id = 42;

    return box.user.id;
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

TEST(DriverTest, StructFieldAssignmentPreservesCopiedStructValueSemantics) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_field_assignment_value_semantics.velo",
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    let a: User = User {
        id: 1
    };

    var b: User = a;

    b.id = 2;

    return a.id;
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

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, IrModePrintsStoreFieldPathInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_field_assignment_ir.velo",
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    var user: User = User {
        id: 1
    };

    user.id = 42;

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

    EXPECT_NE(result.irText.find("StoreFieldPath id"), std::string::npos);
}

TEST(DriverTest, ExecutesStringEquality) {
    Driver driver;
    const auto result = driver.parseText(
        "string_equality.velo",
        R"(module app;

fn main(): int {
    if ("Alex" == "Alex") {
        return 1;
    }

    return 0;
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

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesStringInequality) {
    Driver driver;
    const auto result = driver.parseText(
        "string_inequality.velo",
        R"(module app;

fn main(): int {
    if ("Alex" != "Bob") {
        return 1;
    }

    return 0;
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

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesBoolEquality) {
    Driver driver;
    const auto result = driver.parseText(
        "bool_equality.velo",
        R"(module app;

fn main(): int {
    if (true == true) {
        return 1;
    }

    return 0;
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

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesBoolInequality) {
    Driver driver;
    const auto result = driver.parseText(
        "bool_inequality.velo",
        R"(module app;

fn main(): int {
    if (false != true) {
        return 1;
    }

    return 0;
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

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesStructStringFieldEquality) {
    Driver driver;
    const auto result = driver.parseText(
        "struct_string_field_equality.velo",
        R"(module app;

struct User {
    name: string;
}

fn main(): int {
    let user: User = User {
        name: "Alex"
    };

    if (user.name == "Alex") {
        return 1;
    }

    return 0;
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

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, IrModePrintsGenericEqualityInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "string_equality_ir.velo",
        R"(module app;

fn main(): int {
    if ("Alex" == "Alex") {
        return 1;
    }

    return 0;
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

    EXPECT_NE(result.irText.find("CompareEqual"), std::string::npos);
}

TEST(DriverTest, ExecutesLocalShadowingInsideIfBlock) {
    Driver driver;
    const auto result = driver.parseText(
        "local_shadowing_if.velo",
        R"(module app;

fn main(): int {
    let value: int = 1;

    if (true) {
        let value: int = 2;
        return value;
    }

    return value;
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

    EXPECT_EQ(result.exitCode, 2);
}

TEST(DriverTest, ExecutesOuterLocalAfterIfBlockShadowing) {
    Driver driver;
    const auto result = driver.parseText(
        "outer_local_after_if_shadowing.velo",
        R"(module app;

fn main(): int {
    let value: int = 1;

    if (true) {
        let value: int = 2;
    }

    return value;
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

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesLocalShadowingInsideWhileBlock) {
    Driver driver;
    const auto result = driver.parseText(
        "local_shadowing_while.velo",
        R"(module app;

fn main(): int {
    var counter: int = 0;

    while (counter == 0) {
        let counter: int = 7;
        return counter;
    }

    return counter;
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

    EXPECT_EQ(result.exitCode, 7);
}

TEST(DriverTest, ExecutesAssignmentToOuterLocalAfterBlockScope) {
    Driver driver;
    const auto result = driver.parseText(
        "assign_outer_after_block_scope.velo",
        R"(module app;

fn main(): int {
    var value: int = 1;

    if (true) {
        let value: int = 2;
    }

    value = 3;

    return value;
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

    EXPECT_EQ(result.exitCode, 3);
}

TEST(DriverTest, IrModeUsesDifferentLocalSlotsForShadowedLocals) {
    Driver driver;
    const auto result = driver.parseText(
        "local_shadowing_ir.velo",
        R"(module app;

fn main(): int {
    let value: int = 1;

    if (true) {
        let value: int = 2;
        return value;
    }

    return value;
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

    EXPECT_NE(result.irText.find("StoreLocal local[0]"), std::string::npos);
    EXPECT_NE(result.irText.find("StoreLocal local[1]"), std::string::npos);
    EXPECT_NE(result.irText.find("LoadLocal local[1]"), std::string::npos);
}

TEST(DriverTest, ExecutesArrayLiteralProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "array_literal.velo",
        R"(module app;

fn main(): int {
    let ids: []int = [1, 2, 3];

    return 0;
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

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, ExecutesArrayLiteralOfStructValues) {
    Driver driver;
    const auto result = driver.parseText(
        "array_literal_structs.velo",
        R"(module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    let users: []User = [
        User { id: 1, name: "Alex" },
        User { id: 2, name: "Bob" }
    ];

    return 0;
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

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, IrModePrintsBuildArrayInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "array_literal_ir.velo",
        R"(module app;

fn main(): int {
    let ids: []int = [1, 2, 3];

    return 0;
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

    EXPECT_NE(result.irText.find("BuildArray elements=3"), std::string::npos);
}

TEST(DriverTest, ExecutesArrayIndexRead) {
    Driver driver;
    const auto result = driver.parseText(
        "array_index_read.velo",
        R"(module app;

fn main(): int {
    let ids: []int = [10, 20, 30];

    return ids[1];
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

    EXPECT_EQ(result.exitCode, 20);
}

TEST(DriverTest, ExecutesArrayIndexReadOnStructArray) {
    Driver driver;
    const auto result = driver.parseText(
        "array_index_struct_read.velo",
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    let users: []User = [
        User { id: 1 },
        User { id: 2 }
    ];

    return users[1].id;
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

    EXPECT_EQ(result.exitCode, 2);
}

TEST(DriverTest, ExecutesNestedArrayIndexRead) {
    Driver driver;
    const auto result = driver.parseText(
        "nested_array_index_read.velo",
        R"(module app;

fn main(): int {
    let matrix: [][]int = [
        [1, 2],
        [3, 4]
    ];

    return matrix[1][0];
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

    EXPECT_EQ(result.exitCode, 3);
}

TEST(DriverTest, ReportsRuntimeErrorForArrayIndexOutOfRange) {
    Driver driver;
    const auto result = driver.parseText(
        "array_index_out_of_range.velo",
        R"(module app;

fn main(): int {
    let ids: []int = [10, 20, 30];

    return ids[3];
}
)"
    );

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_NE(result.error.find("Array index out of range"), std::string::npos);
}

TEST(DriverTest, IrModePrintsLoadIndexInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "array_index_ir.velo",
        R"(module app;

fn main(): int {
    let ids: []int = [10, 20, 30];

    return ids[1];
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

    EXPECT_NE(result.irText.find("LoadIndex"), std::string::npos);
}

TEST(DriverTest, ExecutesArrayElementAssignment) {
    Driver driver;
    const auto result = driver.parseText(
        "array_element_assignment.velo",
        R"(module app;

fn main(): int {
    var ids: []int = [1, 2, 3];

    ids[0] = 42;

    return ids[0];
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

TEST(DriverTest, ExecutesNestedArrayElementAssignment) {
    Driver driver;
    const auto result = driver.parseText(
        "nested_array_element_assignment.velo",
        R"(module app;

fn main(): int {
    var matrix: [][]int = [
        [1, 2],
        [3, 4]
    ];

    matrix[1][0] = 99;

    return matrix[1][0];
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

    EXPECT_EQ(result.exitCode, 99);
}

TEST(DriverTest, ArrayElementAssignmentPreservesCopiedArrayValueSemantics) {
    Driver driver;
    const auto result = driver.parseText(
        "array_assignment_value_semantics.velo",
        R"(module app;

fn main(): int {
    let original: []int = [1, 2, 3];
    var copy: []int = original;

    copy[0] = 42;

    return original[0];
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

    EXPECT_EQ(result.exitCode, 1);
}

TEST(DriverTest, ExecutesArrayElementAssignmentWithStructValue) {
    Driver driver;
    const auto result = driver.parseText(
        "array_element_assignment_struct.velo",
        R"(module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    var users: []User = [
        User { id: 1, name: "Alex" }
    ];

    users[0] = User { id: 2, name: "Bob" };

    return users[0].id;
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

    EXPECT_EQ(result.exitCode, 2);
}

TEST(DriverTest, ReportsRuntimeErrorForArrayAssignmentIndexOutOfRange) {
    Driver driver;
    const auto result = driver.parseText(
        "array_assignment_out_of_range.velo",
        R"(module app;

fn main(): int {
    var ids: []int = [1, 2, 3];

    ids[3] = 42;

    return 0;
}
)"
    );

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_NE(result.error.find("Array index out of range"), std::string::npos);
}

TEST(DriverTest, IrModePrintsStoreIndexPathInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "array_assignment_ir.velo",
        R"(module app;

fn main(): int {
    var ids: []int = [1, 2, 3];

    ids[0] = 42;

    return ids[0];
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

    EXPECT_NE(result.irText.find("StoreIndexPath indexes=1"), std::string::npos);
}

TEST(DriverTest, ExecutesArrayLenBuiltinForIntArray) {
    Driver driver;
    const auto result = driver.parseText(
        "array_len_int.velo",
        R"(module app;

use std::array;

fn main(): int {
    let ids: []int = [1, 2, 3];

    return array::len(ids);
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

    EXPECT_EQ(result.exitCode, 3);
}

TEST(DriverTest, ExecutesArrayLenBuiltinForEmptyArray) {
    Driver driver;
    const auto result = driver.parseText(
        "array_len_empty.velo",
        R"(module app;

use std::array;

fn main(): int {
    let ids: []int = [];

    return array::len(ids);
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

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, ExecutesArrayLenBuiltinForStructArray) {
    Driver driver;
    const auto result = driver.parseText(
        "array_len_struct.velo",
        R"(module app;

use std::array;

struct User {
    id: int;
}

fn main(): int {
    let users: []User = [
        User { id: 1 },
        User { id: 2 }
    ];

    return array::len(users);
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

    EXPECT_EQ(result.exitCode, 2);
}

TEST(DriverTest, IrModePrintsArrayLenBuiltinCall) {
    Driver driver;
    const auto result = driver.parseText(
        "array_len_ir.velo",
        R"(module app;

use std::array;

fn main(): int {
    let ids: []int = [1, 2, 3];

    return array::len(ids);
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

    EXPECT_NE(result.irText.find("CallBuiltin array::len args=1"), std::string::npos);
}

TEST(DriverTest, BytecodeModePrintsBytecodeDisassembly) {
    Driver driver;
    const auto result = driver.parseText(
        "bytecode_mode.velo",
        R"(module app;

fn main(): int {
    return 42;
}
)",
        Velo::Driver::DriverMode::Bytecode
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

    EXPECT_NE(result.bytecodeText.find("fn main"), std::string::npos);
    EXPECT_NE(result.bytecodeText.find("PushInt 42"), std::string::npos);
    EXPECT_NE(result.bytecodeText.find("Return"), std::string::npos);
}

TEST(DriverTest, BytecodeModePrintsArrayInstructions) {
    Driver driver;
    const auto result = driver.parseText(
        "bytecode_array_mode.velo",
        R"(module app;

use std::array;

fn main(): int {
    let ids: []int = [1, 2, 3];

    return array::len(ids);
}
)",
        Velo::Driver::DriverMode::Bytecode
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

    EXPECT_NE(result.bytecodeText.find("BuildArray elements=3"), std::string::npos);
    EXPECT_NE(result.bytecodeText.find("CallBuiltin array::len args=1"), std::string::npos);
}

TEST(DriverTest, ExecutesMapLiteralProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "map_literal.velo",
        R"(module app;

use std::console;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10,
        "Bob": 20
    };

    console::println(scores);

    return 0;
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

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, IrModePrintsBuildMapInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "map_literal_ir.velo",
        R"(module app;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10,
        "Bob": 20
    };

    return 0;
}
)",
        Velo::Driver::DriverMode::Ir
    );

    ASSERT_TRUE(result.success);
    EXPECT_NE(result.irText.find("BuildMap entries=2"), std::string::npos);
}

TEST(DriverTest, BytecodeModePrintsBuildMapInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "map_literal_bytecode.velo",
        R"(module app;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10,
        "Bob": 20
    };

    return 0;
}
)",
        Velo::Driver::DriverMode::Bytecode
    );

    ASSERT_TRUE(result.success);
    EXPECT_NE(result.bytecodeText.find("BuildMap entries=2"), std::string::npos);
}

TEST(DriverTest, ExecutesMapIndexRead) {
    Driver driver;
    const auto result = driver.parseText(
        "map_index_read.velo",
        R"(module app;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10,
        "Bob": 20
    };

    return scores["Bob"];
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

    EXPECT_EQ(result.exitCode, 20);
}

TEST(DriverTest, ExecutesMapIndexReadOnStructMap) {
    Driver driver;
    const auto result = driver.parseText(
        "map_index_struct_read.velo",
        R"(module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    let users: map<string, User> = map {
        "alex": User { id: 1, name: "Alex" },
        "bob": User { id: 2, name: "Bob" }
    };

    return users["bob"].id;
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

    EXPECT_EQ(result.exitCode, 2);
}

TEST(DriverTest, ExecutesMapIndexReadWithArrayValue) {
    Driver driver;
    const auto result = driver.parseText(
        "map_index_array_value.velo",
        R"(module app;

fn main(): int {
    let grouped: map<string, []int> = map {
        "a": [1, 2],
        "b": [3, 4]
    };

    return grouped["b"][1];
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

    EXPECT_EQ(result.exitCode, 4);
}

TEST(DriverTest, ReportsRuntimeErrorForMissingMapKey) {
    Driver driver;
    const auto result = driver.parseText(
        "missing_map_key.velo",
        R"(module app;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10
    };

    return scores["Bob"];
}
)"
    );

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_NE(result.error.find("Map key not found"), std::string::npos);
}

TEST(DriverTest, IrModePrintsMapIndexLoadIndexInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "map_index_ir.velo",
        R"(module app;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10
    };

    return scores["Alex"];
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

    EXPECT_NE(result.irText.find("BuildMap entries=1"), std::string::npos);
    EXPECT_NE(result.irText.find("LoadIndex"), std::string::npos);
}

TEST(DriverTest, BytecodeModePrintsMapIndexLoadIndexInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "map_index_bytecode.velo",
        R"(module app;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10
    };

    return scores["Alex"];
}
)",
        Velo::Driver::DriverMode::Bytecode
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

    EXPECT_NE(result.bytecodeText.find("BuildMap entries=1"), std::string::npos);
    EXPECT_NE(result.bytecodeText.find("LoadIndex"), std::string::npos);
}

TEST(DriverTest, ExecutesMapElementAssignment) {
    Driver driver;
    const auto result = driver.parseText(
        "map_element_assignment.velo",
        R"(module app;

fn main(): int {
    var scores: map<string, int> = map {
        "Alex": 10,
        "Bob": 20
    };

    scores["Bob"] = 42;

    return scores["Bob"];
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

TEST(DriverTest, ExecutesMapElementInsertAssignment) {
    Driver driver;
    const auto result = driver.parseText(
        "map_element_insert_assignment.velo",
        R"(module app;

fn main(): int {
    var scores: map<string, int> = map {
        "Alex": 10
    };

    scores["Carol"] = 30;

    return scores["Carol"];
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

    EXPECT_EQ(result.exitCode, 30);
}

TEST(DriverTest, ExecutesAssignmentThroughMapIndexThenArrayIndex) {
    Driver driver;
    const auto result = driver.parseText(
        "map_array_value_assignment.velo",
        R"(module app;

fn main(): int {
    var grouped: map<string, []int> = map {
        "a": [1, 2]
    };

    grouped["a"][0] = 42;

    return grouped["a"][0];
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

TEST(DriverTest, ExecutesNestedMapElementAssignment) {
    Driver driver;
    const auto result = driver.parseText(
        "nested_map_element_assignment.velo",
        R"(module app;

fn main(): int {
    var nested: map<string, map<string, int>> = map {
        "outer": map {
            "inner": 1
        }
    };

    nested["outer"]["inner"] = 42;

    return nested["outer"]["inner"];
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

TEST(DriverTest, ReportsRuntimeErrorForMissingMapKeyInNestedAssignment) {
    Driver driver;
    const auto result = driver.parseText(
        "missing_map_key_nested_assignment.velo",
        R"(module app;

fn main(): int {
    var nested: map<string, map<string, int>> = map {};

    nested["outer"]["inner"] = 42;

    return 0;
}
)"
    );

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_NE(result.error.find("Map key not found"), std::string::npos);
}

TEST(DriverTest, IrModePrintsMapAssignmentStoreIndexPathInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "map_assignment_ir.velo",
        R"(module app;

fn main(): int {
    var scores: map<string, int> = map {
        "Alex": 10
    };

    scores["Alex"] = 42;

    return scores["Alex"];
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

    EXPECT_NE(result.irText.find("StoreIndexPath indexes=1"), std::string::npos);
}

TEST(DriverTest, BytecodeModePrintsMapAssignmentStoreIndexPathInstruction) {
    Driver driver;
    const auto result = driver.parseText(
        "map_assignment_bytecode.velo",
        R"(module app;

fn main(): int {
    var scores: map<string, int> = map {
        "Alex": 10
    };

    scores["Alex"] = 42;

    return scores["Alex"];
}
)",
        Velo::Driver::DriverMode::Bytecode
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

    EXPECT_NE(result.bytecodeText.find("StoreIndexPath indexes=1"), std::string::npos);
}

TEST(DriverTest, ExecutesMapLenBuiltinForIntMap) {
    Driver driver;
    const auto result = driver.parseText(
        "map_len_int.velo",
        R"(module app;

use std::map;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10,
        "Bob": 20
    };

    return map::len(scores);
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

    EXPECT_EQ(result.exitCode, 2);
}

TEST(DriverTest, ExecutesMapLenBuiltinForEmptyMap) {
    Driver driver;
    const auto result = driver.parseText(
        "map_len_empty.velo",
        R"(module app;

use std::map;

fn main(): int {
    let scores: map<string, int> = map {};

    return map::len(scores);
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

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, ExecutesMapLenBuiltinAfterMapInsertAssignment) {
    Driver driver;
    const auto result = driver.parseText(
        "map_len_after_insert.velo",
        R"(module app;

use std::map;

fn main(): int {
    var scores: map<string, int> = map {
        "Alex": 10
    };

    scores["Bob"] = 20;
    scores["Carol"] = 30;

    return map::len(scores);
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

    EXPECT_EQ(result.exitCode, 3);
}

TEST(DriverTest, IrModePrintsMapLenBuiltinCall) {
    Driver driver;
    const auto result = driver.parseText(
        "map_len_ir.velo",
        R"(module app;

use std::map;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10
    };

    return map::len(scores);
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

    EXPECT_NE(result.irText.find("CallBuiltin map::len args=1"), std::string::npos);
}

TEST(DriverTest, BytecodeModePrintsMapLenBuiltinCall) {
    Driver driver;
    const auto result = driver.parseText(
        "map_len_bytecode.velo",
        R"(module app;

use std::map;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10
    };

    return map::len(scores);
}
)",
        Velo::Driver::DriverMode::Bytecode
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

    EXPECT_NE(result.bytecodeText.find("CallBuiltin map::len args=1"), std::string::npos);
}

TEST(DriverTest, ExecutesJsonStringifyInt) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_int.velo",
        R"(module app;

use std::json;
use std::string;

fn main(): int {
    return string::len(json::stringify(42));
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

    EXPECT_EQ(result.exitCode, 2);
}

TEST(DriverTest, ExecutesJsonStringifyString) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_string.velo",
        R"(module app;

use std::json;
use std::string;

fn main(): int {
    return string::len(json::stringify("Alex"));
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

    EXPECT_EQ(result.exitCode, 6);
}

TEST(DriverTest, ExecutesJsonStringifyBool) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_bool.velo",
        R"(module app;

use std::json;
use std::string;

fn main(): int {
    return string::len(json::stringify(true));
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

    EXPECT_EQ(result.exitCode, 4);
}

TEST(DriverTest, IrModePrintsJsonStringifyBuiltinCall) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_ir.velo",
        R"(module app;

use std::json;

fn main(): int {
    let text: string = json::stringify(42);

    return 0;
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

    EXPECT_NE(result.irText.find("CallBuiltin json::stringify args=1"), std::string::npos);
}

TEST(DriverTest, BytecodeModePrintsJsonStringifyBuiltinCall) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_bytecode.velo",
        R"(module app;

use std::json;

fn main(): int {
    let text: string = json::stringify(42);

    return 0;
}
)",
        Velo::Driver::DriverMode::Bytecode
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

    EXPECT_NE(result.bytecodeText.find("CallBuiltin json::stringify args=1"), std::string::npos);
}

TEST(DriverTest, ExecutesJsonStringifyIntArray) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_int_array.velo",
        R"(module app;

use std::json;
use std::string;

fn main(): int {
    let ids: []int = [1, 2, 3];

    return string::len(json::stringify(ids));
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

    EXPECT_EQ(result.exitCode, 7);
}

TEST(DriverTest, ExecutesJsonStringifyStringArray) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_string_array.velo",
        R"(module app;

use std::json;
use std::string;

fn main(): int {
    let names: []string = ["Alex", "Bob"];

    return string::len(json::stringify(names));
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

    EXPECT_EQ(result.exitCode, 14);
}

TEST(DriverTest, ExecutesJsonStringifyIntMap) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_int_map.velo",
        R"(module app;

use std::json;
use std::string;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10,
        "Bob": 20
    };

    return string::len(json::stringify(scores));
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

    EXPECT_EQ(result.exitCode, 20);
}

TEST(DriverTest, ExecutesJsonStringifyMapWithArrayValues) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_map_array_values.velo",
        R"(module app;

use std::json;
use std::string;

fn main(): int {
    let grouped: map<string, []int> = map {
        "a": [1, 2],
        "b": [3, 4]
    };

    return string::len(json::stringify(grouped));
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

    EXPECT_EQ(result.exitCode, 21);
}

TEST(DriverTest, IrModePrintsJsonStringifyCollectionBuiltinCall) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_collection_ir.velo",
        R"(module app;

use std::json;

fn main(): int {
    let ids: []int = [1, 2, 3];
    let text: string = json::stringify(ids);

    return 0;
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

    EXPECT_NE(result.irText.find("CallBuiltin json::stringify args=1"), std::string::npos);
}

TEST(DriverTest, BytecodeModePrintsJsonStringifyCollectionBuiltinCall) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_collection_bytecode.velo",
        R"(module app;

use std::json;

fn main(): int {
    let ids: []int = [1, 2, 3];
    let text: string = json::stringify(ids);

    return 0;
}
)",
        Velo::Driver::DriverMode::Bytecode
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

    EXPECT_NE(result.bytecodeText.find("CallBuiltin json::stringify args=1"), std::string::npos);
}

TEST(DriverTest, ExecutesJsonStringifyStruct) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_struct.velo",
        R"(module app;

use std::json;
use std::string;

struct User {
    id: int;
}

fn main(): int {
    let user: User = User { id: 1 };

    return string::len(json::stringify(user));
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

    // {"id":1}
    EXPECT_EQ(result.exitCode, 8);
}

TEST(DriverTest, ExecutesJsonStringifyStructWithStringField) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_struct_string_field.velo",
        R"(module app;

use std::json;
use std::string;

struct User {
    name: string;
}

fn main(): int {
    let user: User = User { name: "Alex" };

    return string::len(json::stringify(user));
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

    // {"name":"Alex"}
    EXPECT_EQ(result.exitCode, 15);
}

TEST(DriverTest, ExecutesJsonStringifyStructArray) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_struct_array.velo",
        R"(module app;

use std::json;
use std::string;

struct User {
    id: int;
}

fn main(): int {
    let users: []User = [
        User { id: 1 },
        User { id: 2 }
    ];

    return string::len(json::stringify(users));
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

    // [{"id":1},{"id":2}]
    EXPECT_EQ(result.exitCode, 19);
}

TEST(DriverTest, ExecutesJsonStringifyStructMap) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_struct_map.velo",
        R"(module app;

use std::json;
use std::string;

struct User {
    id: int;
}

fn main(): int {
    let users: map<string, User> = map {
        "a": User { id: 1 },
        "b": User { id: 2 }
    };

    return string::len(json::stringify(users));
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

    // {"a":{"id":1},"b":{"id":2}}
    EXPECT_EQ(result.exitCode, 27);
}

TEST(DriverTest, ExecutesJsonStringifyStructProgram) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_struct_program.velo",
        R"(module app;

use std::console;
use std::json;

struct User {
    id: int;
    name: string;
    active: bool;
}

fn main(): int {
    let user: User = User {
        id: 1,
        name: "Alex",
        active: true
    };

    console::println(json::stringify(user));

    return 0;
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

    EXPECT_EQ(result.exitCode, 0);
}

TEST(DriverTest, IrModePrintsJsonStringifyStructBuiltinCall) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_struct_ir.velo",
        R"(module app;

use std::json;

struct User {
    id: int;
}

fn main(): int {
    let user: User = User { id: 1 };
    let text: string = json::stringify(user);

    return 0;
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

    EXPECT_NE(result.irText.find("CallBuiltin json::stringify args=1"), std::string::npos);
}

TEST(DriverTest, BytecodeModePrintsJsonStringifyStructBuiltinCall) {
    Driver driver;
    const auto result = driver.parseText(
        "json_stringify_struct_bytecode.velo",
        R"(module app;

use std::json;

struct User {
    id: int;
}

fn main(): int {
    let user: User = User { id: 1 };
    let text: string = json::stringify(user);

    return 0;
}
)",
        Velo::Driver::DriverMode::Bytecode
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

    EXPECT_NE(result.bytecodeText.find("CallBuiltin json::stringify args=1"), std::string::npos);
}