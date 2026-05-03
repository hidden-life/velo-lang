#include "velo/parser/parser.h"

#include <gtest/gtest.h>

#include "velo/ast/ast.h"
#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/lexer/lexer.h"

using Velo::AST::CallExpression;
using Velo::AST::ExpressionKind;
using Velo::AST::ExpressionStatement;
using Velo::AST::IntegerLiteralExpression;
using Velo::AST::ReturnStatement;
using Velo::AST::StringLiteralExpression;
using Velo::Diagnostic::DiagnosticEngine;
using Velo::Lexer::Lexer;
using Velo::Parser::Parser;
using Velo::Source::SourceFile;

namespace {
    auto parseProgram(std::string_view source, DiagnosticEngine &engine) -> std::unique_ptr<Velo::AST::Program> {
        const SourceFile file("inline.velo", std::string(source));
        Lexer lexer(file, engine);
        Parser parser(lexer.lexAll(), engine);

        return parser.parse();
    }
}

TEST(ParserTest, ParsesHelloWorldProgramIntoAST) {
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

    ASSERT_TRUE(program->module.has_value());
    ASSERT_EQ(program->module->name.segments.size(), 1U);
    EXPECT_EQ(program->module->name.segments[0], "app");

    ASSERT_EQ(program->uses.size(), 1U);
    ASSERT_EQ(program->uses[0].path.segments.size(), 2U);
    EXPECT_EQ(program->uses[0].path.segments[0], "std");
    EXPECT_EQ(program->uses[0].path.segments[1], "console");
    EXPECT_FALSE(program->uses[0].alias.has_value());

    ASSERT_EQ(program->functions.size(), 1U);
    const auto &func = program->functions.front();
    EXPECT_FALSE(func.isPublic);
    EXPECT_EQ(func.name, "main");
    ASSERT_EQ(func.returnType.name.segments.size(), 1U);
    EXPECT_EQ(func.returnType.name.segments[0], "int");
    ASSERT_EQ(func.statements.size(), 2U);

    const auto *expressionStmt = dynamic_cast<ExpressionStatement*>(func.statements[0].get());
    ASSERT_NE(expressionStmt, nullptr);
    ASSERT_EQ(expressionStmt->expression->kind, ExpressionKind::Call);

    const auto *callExpression = dynamic_cast<CallExpression*>(expressionStmt->expression.get());
    ASSERT_NE(callExpression, nullptr);
    ASSERT_EQ(callExpression->callee.segments.size(), 2U);
    EXPECT_EQ(callExpression->callee.segments[0], "console");
    EXPECT_EQ(callExpression->callee.segments[1], "println");
    ASSERT_EQ(callExpression->arguments.size(), 1U);
    ASSERT_EQ(callExpression->arguments[0]->kind, ExpressionKind::StringLiteral);

    const auto *stringLiteral = dynamic_cast<StringLiteralExpression*>(callExpression->arguments[0].get());
    ASSERT_NE(stringLiteral, nullptr);
    EXPECT_EQ(stringLiteral->value, "Hello, Velo!");

    const auto *returnStmt = dynamic_cast<ReturnStatement*>(func.statements[1].get());
    ASSERT_NE(returnStmt, nullptr);
    ASSERT_EQ(returnStmt->expression->kind, ExpressionKind::IntegerLiteral);

    const auto *integerLiteral = dynamic_cast<IntegerLiteralExpression*>(returnStmt->expression.get());
    ASSERT_NE(integerLiteral, nullptr);
    EXPECT_EQ(integerLiteral->value, "0");
}

TEST(ParserTest, ParsesPublicFunctionAndUseALias) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::console as out;
pub fn main(): int {
    out::println("ok");
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_EQ(program->uses.size(), 1U);
    ASSERT_TRUE(program->uses[0].alias.has_value());
    EXPECT_EQ(program->uses[0].alias.value(), "out");

    ASSERT_EQ(program->functions.size(), 1U);
    EXPECT_TRUE(program->functions[0].isPublic);
}

TEST(ParserTest, ReportsMissingSemicolonAfterUseDeclaration) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::console
fn main(): int {
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_TRUE(engine.hasErrors());
    ASSERT_GE(engine.size(), 1U);
    EXPECT_EQ(engine.diagnostics().front().code(), "PAR006");
}

TEST(ParserTest, ParsesFunctionParameters) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn add(a: int, b: int): int {
    return 0;
}

fn main(): int {
    add(1, 2);
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    if (engine.hasErrors()) {
        for (const auto &diag : engine.diagnostics()) {
            ADD_FAILURE() << diag.code() << ": " << diag.message();
        }
    }

    ASSERT_FALSE(engine.hasErrors());
    ASSERT_EQ(program->functions.size(), 2U);

    const auto &addFunc = program->functions[0];
    EXPECT_EQ(addFunc.name, "add");

    ASSERT_EQ(addFunc.parameters.size(), 2U);

    ASSERT_EQ(addFunc.parameters[0].name, "a");
    ASSERT_EQ(addFunc.parameters[0].type.name.segments.size(), 1U);
    EXPECT_EQ(addFunc.parameters[0].type.name.segments[0], "int");

    ASSERT_EQ(addFunc.parameters[1].name, "b");
    ASSERT_EQ(addFunc.parameters[1].type.name.segments.size(), 1U);
    EXPECT_EQ(addFunc.parameters[1].type.name.segments[0], "int");
}

TEST(ParserTest, ParsesBinaryAddExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn add(a: int, b: int): int {
    return a + b;
}

fn main(): int {
    return add(20, 22);
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_EQ(program->functions.size(), 2U);

    const auto &addFunc = program->functions[0];
    ASSERT_EQ(addFunc.statements.size(), 1U);

    const auto *returnStmt = dynamic_cast<ReturnStatement*>(addFunc.statements[0].get());
    ASSERT_NE(returnStmt, nullptr);

    ASSERT_EQ(returnStmt->expression->kind, ExpressionKind::Binary);

    const auto *binaryExpr = dynamic_cast<Velo::AST::BinaryExpression*>(returnStmt->expression.get());
    ASSERT_NE(binaryExpr, nullptr);
    EXPECT_EQ(binaryExpr->op, Velo::AST::BinaryOperator::Add);
}

TEST(ParserTest, ParsesReturnWithoutExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn log(): void {
    return;
}

fn main(): int {
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_EQ(program->functions.size(), 2U);

    const auto &logFunc = program->functions[0];
    ASSERT_EQ(logFunc.statements.size(), 1U);

    const auto *returnStmt = dynamic_cast<ReturnStatement*>(logFunc.statements[0].get());
    ASSERT_NE(returnStmt, nullptr);
    EXPECT_EQ(returnStmt->expression, nullptr);
}

TEST(ParserTest, ParsesLocalVariableDeclaration) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    let x: int = 42;
    return x;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    ASSERT_EQ(program->functions.size(), 1U);

    const auto &mainFunc = program->functions[0];
    ASSERT_EQ(mainFunc.statements.size(), 2U);
    EXPECT_EQ(mainFunc.statements[0]->kind, Velo::AST::StatementKind::VariableDeclaration);
}

TEST(ParserTest, ParsesMutableVariableAssignment) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    var x: int = 1;
    x = x + 41;
    return x;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunc = program->functions[0];
    ASSERT_EQ(mainFunc.statements.size(), 3U);
    EXPECT_EQ(mainFunc.statements[1]->kind, Velo::AST::StatementKind::Assignment);
}