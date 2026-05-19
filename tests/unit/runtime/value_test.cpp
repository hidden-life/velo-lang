#include <gtest/gtest.h>
#include "velo/runtime/value.h"

using Velo::Runtime::StructValue;
using Velo::Runtime::StructValuePtr;
using Velo::Runtime::Value;
using Velo::Runtime::cloneValue;

TEST(RuntimeValueTest, CloneValueReturnsPrimitiveValuesUnchanged) {
    const Value intVal = 42;
    const Value stringVal = std::string("John Doe");
    const Value boolVal = true;

    EXPECT_EQ(std::get<int>(cloneValue(intVal)), 42);
    EXPECT_EQ(std::get<std::string>(cloneValue(stringVal)), "John Doe");
    EXPECT_TRUE(std::get<bool>(cloneValue(boolVal)));
}

TEST(RuntimeValueTest, CloneValueDeepCopiesStructValues) {
    auto profile = std::make_shared<StructValue>();
    profile->typeName = "Profile";
    profile->fields.emplace("id", 7);
    auto user = std::make_shared<StructValue>();
    user->typeName = "User";
    user->fields.emplace("name", std::string("John Doe"));
    user->fields.emplace("profile", profile);

    const Value clonedVal = cloneValue(Value {user});

    ASSERT_TRUE(std::holds_alternative<StructValuePtr>(clonedVal));
    const auto cloned = std::get<StructValuePtr>(clonedVal);

    ASSERT_NE(cloned, nullptr);
    EXPECT_NE(cloned.get(), user.get());
    EXPECT_EQ(cloned->typeName, "User");
    EXPECT_EQ(std::get<std::string>(cloned->fields.at("name")), "John Doe");

    ASSERT_TRUE(std::holds_alternative<StructValuePtr>(cloned->fields.at("profile")));
    const auto clonedProfile = std::get<StructValuePtr>(cloned->fields.at("profile"));

    ASSERT_NE(clonedProfile, nullptr);
    EXPECT_NE(clonedProfile.get(), profile.get());
    EXPECT_EQ(clonedProfile->typeName, "Profile");
    EXPECT_EQ(std::get<int>(clonedProfile->fields.at("id")), 7);

    user->fields["name"] = std::string("Bob");
    profile->fields["id"] = 99;

    EXPECT_EQ(std::get<std::string>(cloned->fields.at("name")), "John Doe");
    EXPECT_EQ(std::get<int>(clonedProfile->fields.at("id")), 7);
}