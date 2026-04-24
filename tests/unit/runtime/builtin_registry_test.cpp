#include <gtest/gtest.h>

#include "velo/runtime/builtin_registry.h"

using Velo::Runtime::BuiltinFunction;
using Velo::Runtime::BuiltinRegistry;
using Velo::Runtime::ExecutionResult;
using Velo::Runtime::Value;

TEST(BuiltinRegistryTest, RegistersAndFindsFunction) {
    BuiltinRegistry registry;

    registry.registerFunc(
        BuiltinFunction {
            "test::echo",
            1U,
            [](const std::vector<Value>&) -> ExecutionResult {
                return {};
            }
        }
    );

    ASSERT_TRUE(registry.contains("test::echo"));

    const auto *func = registry.find("test::echo");
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->name(), "test::echo");
    EXPECT_EQ(func->arity(), 1U);
}

TEST(BuiltinRegistryTest, ReturnsNullForUnknownFunction) {
    BuiltinRegistry registry;

    EXPECT_FALSE(registry.contains("missing::func"));
    EXPECT_EQ(registry.find("missing::func"), nullptr);
}