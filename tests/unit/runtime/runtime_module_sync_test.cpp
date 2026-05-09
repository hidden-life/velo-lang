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
    ASSERT_EQ(println->parameterTypes.size(), 1U);
    EXPECT_EQ(println->parameterTypes[0], "any");

    const auto *stringModule = modules.find("string");
    ASSERT_NE(stringModule, nullptr);

    const auto *len = stringModule->findFunction("len");
    ASSERT_NE(len, nullptr);

    EXPECT_EQ(len->name, "len");
    EXPECT_EQ(len->arity, 1U);
    EXPECT_EQ(len->returnType, "int");
    ASSERT_EQ(len->parameterTypes.size(), 1U);
    EXPECT_EQ(len->parameterTypes[0], "string");
}