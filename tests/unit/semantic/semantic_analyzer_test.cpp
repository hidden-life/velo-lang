#include <gtest/gtest.h>

#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/lexer/lexer.h"
#include "velo/parser/parser.h"
#include "velo/semantic/semantic_analyzer.h"

using Velo::Diagnostic::DiagnosticEngine;
using Velo::Lexer::Lexer;
using Velo::Parser::Parser;
using Velo::Semantic::SematicAnalyzer;
using Velo::Source::SourceFile;

namespace {
    auto parseProgram(std::string_view source, DiagnosticEngine &engine) -> std::unique_ptr<Velo::AST::Program> {
        const SourceFile file("inline.velo", std::string(source));
        Lexer lexer(file, engine);
        Parser parser(lexer.lexAll(), engine);

        return parser.parse();
    }
}

TEST(SemanticAnalyzerTests, AcceptsValidHelloWorldProgram) {
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

    SematicAnalyzer analyzer(*program, engine);
    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTests, ReportMissingMainFunction) {
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

    SematicAnalyzer analyzer(*program, engine);
    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.size(), 1U);
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM003");
}

TEST(SemanticAnalyzerTests, ReportsUnknownModuleQualifier) {
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

    SematicAnalyzer analyzer(*program, engine);
    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.size(), 1U);
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM008");
}

TEST(SemanticAnalyzerTests, ReportsDuplicateVisibleImportName) {
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

    SematicAnalyzer analyzer(*program, engine);
    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.diagnostics().front().code(), "SEM001");
}