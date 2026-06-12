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

TEST(RuntimeValueTest, CloneValueDeepCopiesArrayValues) {
    auto nested = std::make_shared<Velo::Runtime::ArrayValue>();
    nested->elements.emplace_back(7);

    auto array = std::make_shared<Velo::Runtime::ArrayValue>();
    array->elements.emplace_back(1);
    array->elements.emplace_back(std::string("Alex"));
    array->elements.emplace_back(nested);

    const Velo::Runtime::Value clonedValue = Velo::Runtime::cloneValue(
        Velo::Runtime::Value {array}
    );

    ASSERT_TRUE(std::holds_alternative<Velo::Runtime::ArrayValuePtr>(clonedValue));
    const auto clonedArray = std::get<Velo::Runtime::ArrayValuePtr>(clonedValue);

    ASSERT_NE(clonedArray, nullptr);
    EXPECT_NE(clonedArray.get(), array.get());
    ASSERT_EQ(clonedArray->elements.size(), 3U);

    EXPECT_EQ(std::get<int>(clonedArray->elements[0]), 1);
    EXPECT_EQ(std::get<std::string>(clonedArray->elements[1]), "Alex");

    ASSERT_TRUE(std::holds_alternative<Velo::Runtime::ArrayValuePtr>(clonedArray->elements[2]));
    const auto clonedNested = std::get<Velo::Runtime::ArrayValuePtr>(clonedArray->elements[2]);

    ASSERT_NE(clonedNested, nullptr);
    EXPECT_NE(clonedNested.get(), nested.get());
    ASSERT_EQ(clonedNested->elements.size(), 1U);
    EXPECT_EQ(std::get<int>(clonedNested->elements[0]), 7);

    nested->elements[0] = 99;
    array->elements[0] = 42;

    EXPECT_EQ(std::get<int>(clonedArray->elements[0]), 1);
    EXPECT_EQ(std::get<int>(clonedNested->elements[0]), 7);
}

TEST(RuntimeValueTest, CloneValueDeepCopiesMapValues) {
    auto nestedArray = std::make_shared<Velo::Runtime::ArrayValue>();
    nestedArray->elements.emplace_back(7);

    auto mapValue = std::make_shared<Velo::Runtime::MapValue>();
    mapValue->entries.emplace("answer", 42);
    mapValue->entries.emplace("name", std::string("Alex"));
    mapValue->entries.emplace("values", nestedArray);

    const Velo::Runtime::Value clonedValue = Velo::Runtime::cloneValue(
        Velo::Runtime::Value {mapValue}
    );

    ASSERT_TRUE(std::holds_alternative<Velo::Runtime::MapValuePtr>(clonedValue));

    const auto clonedMap = std::get<Velo::Runtime::MapValuePtr>(clonedValue);
    ASSERT_NE(clonedMap, nullptr);
    EXPECT_NE(clonedMap.get(), mapValue.get());

    ASSERT_EQ(clonedMap->entries.size(), 3U);
    EXPECT_EQ(std::get<int>(clonedMap->entries.at("answer")), 42);
    EXPECT_EQ(std::get<std::string>(clonedMap->entries.at("name")), "Alex");

    ASSERT_TRUE(std::holds_alternative<Velo::Runtime::ArrayValuePtr>(clonedMap->entries.at("values")));

    const auto clonedArray = std::get<Velo::Runtime::ArrayValuePtr>(clonedMap->entries.at("values"));
    ASSERT_NE(clonedArray, nullptr);
    EXPECT_NE(clonedArray.get(), nestedArray.get());

    nestedArray->elements[0] = 99;
    mapValue->entries["answer"] = 100;

    EXPECT_EQ(std::get<int>(clonedMap->entries.at("answer")), 42);
    EXPECT_EQ(std::get<int>(clonedArray->elements[0]), 7);
}