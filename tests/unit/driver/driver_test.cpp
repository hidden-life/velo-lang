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