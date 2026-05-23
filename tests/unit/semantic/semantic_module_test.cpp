#include <gtest/gtest.h>
#include "velo/driver/driver.h"

using Velo::Driver::Driver;

TEST(SemanticModuleTest, ReportsUnknownFunctionInsideModule) {
    Driver driver;
    const auto result = driver.parseText(
        "bad.velo",
        R"(module app;
use std::console;

fn main(): int {
    console::missing("Hello");
    return 0;
}
)"
    );

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().code(), "SEM009");
}

TEST(SemanticModuleTest, ReportsWrongBuiltinArgumentCount) {
    Driver driver;

    const auto result = driver.parseText(
        "bad_arity.velo",
        R"(module app;
use std::console;

fn main(): int {
    console::println("one", "two");
    return 0;
}
)"
    );

    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().code(), "SEM010");
}