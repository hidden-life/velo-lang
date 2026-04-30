#include <gtest/gtest.h>

#include "velo/runtime/runtime.h"

using Velo::Runtime::Runtime;

TEST(RuntimeTest, BuildsModulesFromBuiltins) {
    Runtime runtime;
    const auto &modules = runtime.modules();
    const auto *console = modules.find("console");

    ASSERT_NE(console, nullptr);
    const auto *println = console->findFunction("println");
    ASSERT_NE(println, nullptr);

    EXPECT_EQ(println->name, "println");
    EXPECT_EQ(println->arity, 1U);
    EXPECT_EQ(println->returnType, "void");
}