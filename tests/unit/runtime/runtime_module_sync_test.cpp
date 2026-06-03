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

    const auto *intModule = modules.find("int");
    ASSERT_NE(intModule, nullptr);

    const auto *intToStr = intModule->findFunction("toString");
    ASSERT_NE(intToStr, nullptr);

    EXPECT_EQ(intToStr->name, "toString");
    EXPECT_EQ(intToStr->arity, 1U);
    EXPECT_EQ(intToStr->returnType, "string");
    ASSERT_EQ(intToStr->parameterTypes.size(), 1U);
    EXPECT_EQ(intToStr->parameterTypes[0], "int");

    const auto *boolModule = modules.find("bool");
    ASSERT_NE(boolModule, nullptr);

    const auto *boolToStr = boolModule->findFunction("toString");
    ASSERT_NE(boolToStr, nullptr);

    EXPECT_EQ(boolToStr->name, "toString");
    EXPECT_EQ(boolToStr->arity, 1U);
    EXPECT_EQ(boolToStr->returnType, "string");
    ASSERT_EQ(boolToStr->parameterTypes.size(), 1U);
    EXPECT_EQ(boolToStr->parameterTypes[0], "bool");
}

TEST(RuntimeModuleSyncTest, RegistersArrayLenBuiltinInArrayModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("array");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("len");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "len");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "int");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "array");
}