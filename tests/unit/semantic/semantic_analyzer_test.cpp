#include <gtest/gtest.h>

#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/lexer/lexer.h"
#include "velo/parser/parser.h"
#include "velo/semantic/semantic_analyzer.h"

#include "velo/runtime/runtime.h"
#include "velo/runtime/value.h"

using Velo::Diagnostic::DiagnosticEngine;
using Velo::Lexer::Lexer;
using Velo::Parser::Parser;
using Velo::Semantic::SemanticAnalyzer;
using Velo::Source::SourceFile;

namespace {
    auto parseProgram(std::string_view source, DiagnosticEngine &engine) -> std::unique_ptr<Velo::AST::Program> {
        const SourceFile file("inline.velo", std::string(source));
        Lexer lexer(file, engine);
        Parser parser(lexer.lexAll(), engine);

        return parser.parse();
    }
}

TEST(SemanticAnalyzerTest, AcceptsValidHelloWorldProgram) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::console;
fn main(): int {
    console::println("Hello, Velo!");
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportMissingMainFunction) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn run(): int {
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.size(), 1U);
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM003");
}

TEST(SemanticAnalyzerTest, ReportsUnknownModuleQualifier) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    console::println("Hello, Velo!");
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.size(), 1U);
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM008");
}

TEST(SemanticAnalyzerTest, ReportsDuplicateVisibleImportName) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::console;
use http::console;

fn main(): int {
    console::println("Hello, Velo!");
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.diagnostics().front().code(), "SEM001");
}

TEST(SemanticAnalyzerTest, AcceptsUserDefinedFunctionCall) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
    R"(module app;
fn helper(): int {
    return 0;
}

fn main(): int {
    helper();
    return 0;
}
)",
    engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsUnknownUserDefinedFunctionCall) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
    R"(module app;
fn main(): int {
    missing();
    return 0;
}
)",
    engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.size(), 1U);
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM007");
}

TEST(SemanticAnalyzerTest, AcceptsParameterReference) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn identity(value: int): int {
    return value;
}

fn main(): int {
    return identity(42);
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsDeuplicateParameterName) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn broken(v: int, v: int): int {
    return v;
}

fn main(): int {
    return broken(1, 2);
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_FALSE(analyzer.analyze());
    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.diagnostics().front().code(), "SEM012");
}

TEST(SemanticAnalyzerTest, ReportsInvalidBinaryOperands) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::console;

fn main(): int {
    console::println("a" + 1);
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_FALSE(analyzer.analyze());
    ASSERT_TRUE(engine.hasErrors());
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM013");
}

TEST(SemanticAnalyzerTest, ReportsReturnTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn broken(): int {
    return "hello";
}

fn main(): int {
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_FALSE(analyzer.analyze());
    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.diagnostics().front().code(), "SEM014");
}