#include <gtest/gtest.h>

#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/ir/ir_printer.h"
#include "velo/ir/lowerer.h"
#include "velo/lexer/lexer.h"
#include "velo/parser/parser.h"
#include "velo/runtime/runtime.h"
#include "velo/semantic/semantic_analyzer.h"

using Velo::Diagnostic::DiagnosticEngine;
using Velo::IR::AnnotationArgumentKind;
using Velo::IR::IRPrinter;
using Velo::IR::Lowerer;
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

    auto lowerCheckedProgram(std::string_view source, DiagnosticEngine &engine) -> Velo::IR::Module {
        auto program = parseProgram(source, engine);
        EXPECT_NE(program, nullptr);
        EXPECT_FALSE(engine.hasErrors());

        Velo::Runtime::Runtime runtime;
        SemanticAnalyzer analyzer(*program, engine, runtime.modules());
        EXPECT_TRUE(analyzer.analyze());
        EXPECT_FALSE(engine.hasErrors());

        Lowerer lowerer;
        return lowerer.lower(*program);
    }

    auto findFunction(const Velo::IR::Module &module, std::string_view name) -> const Velo::IR::Function* {
        for (const auto &function : module.functions) {
            if (function.name == name) {
                return &function;
            }
        }

        return nullptr;
    }
}

TEST(IRAnnotationTest, LowersFunctionAnnotations) {
    DiagnosticEngine engine;
    const auto module = lowerCheckedProgram(
        R"(module app;

@auth(true, 10)
fn main(): int {
    return 0;
}
)",
        engine
    );

    const auto *mainFunction = findFunction(module, "main");
    ASSERT_NE(mainFunction, nullptr);

    ASSERT_EQ(mainFunction->annotations.size(), 1U);

    const auto &annotation = mainFunction->annotations.front();
    EXPECT_EQ(annotation.name, "auth");

    ASSERT_EQ(annotation.arguments.size(), 2U);
    EXPECT_EQ(annotation.arguments[0].kind, AnnotationArgumentKind::BooleanLiteral);
    EXPECT_EQ(annotation.arguments[0].value, "true");
    EXPECT_EQ(annotation.arguments[1].kind, AnnotationArgumentKind::IntegerLiteral);
    EXPECT_EQ(annotation.arguments[1].value, "10");
}

TEST(IRAnnotationTest, NormalizesAnnotationModuleAlias) {
    DiagnosticEngine engine;
    const auto module = lowerCheckedProgram(
        R"(module app;

use std::http as web;

@web::get("/health")
fn health(req: http_request): http_response {
    return web::text_response(200, "OK");
}

fn main(): int {
    return 0;
}
)",
        engine
    );

    const auto *healthFunction = findFunction(module, "health");
    ASSERT_NE(healthFunction, nullptr);

    ASSERT_EQ(healthFunction->annotations.size(), 1U);

    const auto &annotation = healthFunction->annotations.front();
    EXPECT_EQ(annotation.name, "http::get");

    ASSERT_EQ(annotation.arguments.size(), 1U);
    EXPECT_EQ(annotation.arguments[0].kind, AnnotationArgumentKind::StringLiteral);
    EXPECT_EQ(annotation.arguments[0].value, "/health");
}

TEST(IRAnnotationTest, PrintsFunctionAnnotations) {
    DiagnosticEngine engine;
    const auto module = lowerCheckedProgram(
        R"(module app;

use std::http;

@http::get("/health")
fn health(req: http_request): http_response {
    return http::text_response(200, "OK");
}

fn main(): int {
    return 0;
}
)",
        engine
    );

    IRPrinter printer;
    const std::string output = printer.print(module);

    EXPECT_NE(output.find("Function health(req)"), std::string::npos);
    EXPECT_NE(output.find("Annotation http::get(\"/health\")"), std::string::npos);
}