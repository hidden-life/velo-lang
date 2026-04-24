#include <gtest/gtest.h>

#include "velo/runtime/runtime.h"

using Velo::Runtime::Runtime;

TEST(RuntimeTest, BuildsModulesFromBuiltins) {
    Runtime runtime;
    const auto &modules = runtime.modules();
    const auto *console = modules.find("console");

    ASSERT_NE(console, nullptr);
    EXPECT_TRUE(console->hasFunction("println"));
}