#include <gtest/gtest.h>
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