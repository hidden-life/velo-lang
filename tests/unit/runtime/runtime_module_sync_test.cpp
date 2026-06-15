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

TEST(RuntimeModuleSyncTest, RegistersMapLenBuiltinInMapModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("map");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("len");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "len");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "int");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "map");
}

TEST(RuntimeModuleSyncTest, RegistersJsonStringifyBuiltinInJsonModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("json");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("stringify");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "stringify");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "string");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "json_serializable");
}

TEST(RuntimeModuleSyncTest, RegistersJsonParseBuiltinInJsonModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("json");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("parse");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "parse");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "json");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "string");
}

TEST(RuntimeModuleSyncTest, RegistersJsonHasBuiltinInJsonModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("json");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("has");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "has");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "bool");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "json");
    EXPECT_EQ(function->parameterTypes[1], "string");
}

TEST(RuntimeModuleSyncTest, RegistersJsonGetIntBuiltinInJsonModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("json");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("get_int");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "get_int");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "int");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "json");
    EXPECT_EQ(function->parameterTypes[1], "string");
}

TEST(RuntimeModuleSyncTest, RegistersJsonGetStringBuiltinInJsonModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("json");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("get_string");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "get_string");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "string");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "json");
    EXPECT_EQ(function->parameterTypes[1], "string");
}

TEST(RuntimeModuleSyncTest, RegistersJsonGetBoolBuiltinInJsonModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("json");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("get_bool");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "get_bool");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "bool");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "json");
    EXPECT_EQ(function->parameterTypes[1], "string");
}

TEST(RuntimeModuleSyncTest, RegistersJsonGetJsonBuiltinInJsonModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("json");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("get_json");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "get_json");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "json");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "json");
    EXPECT_EQ(function->parameterTypes[1], "string");
}

TEST(RuntimeModuleSyncTest, RegistersHttpResponseBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("response");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "response");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "http_response");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "int");
    EXPECT_EQ(function->parameterTypes[1], "string");
}

TEST(RuntimeModuleSyncTest, RegistersHttpTextResponseBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("text_response");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "text_response");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "http_response");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "int");
    EXPECT_EQ(function->parameterTypes[1], "string");
}

TEST(RuntimeModuleSyncTest, RegistersHttpJsonResponseBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("json_response");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "json_response");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "http_response");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "int");
    EXPECT_EQ(function->parameterTypes[1], "json");
}

TEST(RuntimeModuleSyncTest, RegistersHttpStatusBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("status");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "status");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "int");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "http_response");
}

TEST(RuntimeModuleSyncTest, RegistersHttpBodyBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("body");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "body");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "string");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "http_response");
}

TEST(RuntimeModuleSyncTest, RegistersHttpHasHeaderBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("has_header");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "has_header");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "bool");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "http_response");
    EXPECT_EQ(function->parameterTypes[1], "string");
}

TEST(RuntimeModuleSyncTest, RegistersHttpHeaderBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("header");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "header");
    EXPECT_EQ(function->arity, 2U);
    EXPECT_EQ(function->returnType, "string");

    ASSERT_EQ(function->parameterTypes.size(), 2U);
    EXPECT_EQ(function->parameterTypes[0], "http_response");
    EXPECT_EQ(function->parameterTypes[1], "string");
}

TEST(RuntimeModuleSyncTest, RegistersHttpRequestBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("request");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "request");
    EXPECT_EQ(function->arity, 3U);
    EXPECT_EQ(function->returnType, "http_request");

    ASSERT_EQ(function->parameterTypes.size(), 3U);
    EXPECT_EQ(function->parameterTypes[0], "string");
    EXPECT_EQ(function->parameterTypes[1], "string");
    EXPECT_EQ(function->parameterTypes[2], "string");
}

TEST(RuntimeModuleSyncTest, RegistersHttpMethodBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("method");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "method");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "string");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "http_request");
}

TEST(RuntimeModuleSyncTest, RegistersHttpPathBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("path");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "path");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "string");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "http_request");
}

TEST(RuntimeModuleSyncTest, RegistersHttpRequestBodyBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("request_body");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "request_body");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "string");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "http_request");
}

TEST(RuntimeModuleSyncTest, RegistersHttpJsonBodyBuiltinInHttpModule) {
    Velo::Runtime::Runtime runtime;

    const auto *module = runtime.modules().find("http");
    ASSERT_NE(module, nullptr);

    const auto *function = module->findFunction("json_body");
    ASSERT_NE(function, nullptr);

    EXPECT_EQ(function->name, "json_body");
    EXPECT_EQ(function->arity, 1U);
    EXPECT_EQ(function->returnType, "json");

    ASSERT_EQ(function->parameterTypes.size(), 1U);
    EXPECT_EQ(function->parameterTypes[0], "http_request");
}