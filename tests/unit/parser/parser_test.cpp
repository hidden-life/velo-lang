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

TEST(ParserTest, ParsesComparisonExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    if (42 > 10) {
        return 1;
    } else {
        return 0;
    }
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunc = program->functions[0];
    ASSERT_EQ(mainFunc.statements.size(), 1U);
    ASSERT_EQ(mainFunc.statements[0]->kind, Velo::AST::StatementKind::If);

    const auto *ifStmt = dynamic_cast<Velo::AST::IfStatement*>(mainFunc.statements[0].get());
    ASSERT_NE(ifStmt, nullptr);
    ASSERT_EQ(ifStmt->condition->kind, Velo::AST::ExpressionKind::Binary);

    const auto *cond = dynamic_cast<Velo::AST::BinaryExpression*>(ifStmt->condition.get());
    ASSERT_NE(cond, nullptr);

    EXPECT_EQ(cond->op, Velo::AST::BinaryOperator::Greater);
}

TEST(ParserTest, ParsesWhileStatement) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    var x: int = 0;
    while(x < 5) {
        x = x + 1;
    }

    return x;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    const auto &mainFunc = program->functions[0];
    ASSERT_EQ(mainFunc.statements.size(), 3U);
    EXPECT_EQ(mainFunc.statements[1]->kind, Velo::AST::StatementKind::While);

    const auto *whileStmt = dynamic_cast<Velo::AST::WhileStatement*>(mainFunc.statements[1].get());
    ASSERT_NE(whileStmt, nullptr);

    ASSERT_EQ(whileStmt->body.size(), 1U);
    EXPECT_EQ(whileStmt->body[0]->kind, Velo::AST::StatementKind::Assignment);
}

TEST(ParserTest, ParsesBreakAndContinueStatements) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    var x: int = 0;
    while(x < 10) {
        x = x + 1;
        continue;
        break;
    }

    return x;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    const auto &mainFunc = program->functions[0];
    ASSERT_EQ(mainFunc.statements.size(), 3U);
    ASSERT_EQ(mainFunc.statements[1]->kind, Velo::AST::StatementKind::While);

    const auto *whileStmt = dynamic_cast<Velo::AST::WhileStatement*>(mainFunc.statements[1].get());
    ASSERT_NE(whileStmt, nullptr);
    ASSERT_EQ(whileStmt->body.size(), 3U);

    EXPECT_EQ(whileStmt->body[0]->kind, Velo::AST::StatementKind::Assignment);
    EXPECT_EQ(whileStmt->body[1]->kind, Velo::AST::StatementKind::Continue);
    EXPECT_EQ(whileStmt->body[2]->kind, Velo::AST::StatementKind::Break);
}

TEST(ParserTest, ParsesLogicalExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    if (true && !false) {
        return 1;
    }

    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    const auto &mainFunc = program->functions[0];
    ASSERT_EQ(mainFunc.statements.size(), 2U);
    ASSERT_EQ(mainFunc.statements[0]->kind, Velo::AST::StatementKind::If);

    const auto *ifStmt = dynamic_cast<Velo::AST::IfStatement*>(mainFunc.statements[0].get());
    ASSERT_NE(ifStmt, nullptr);
    ASSERT_EQ(ifStmt->condition->kind, ExpressionKind::Binary);

    const auto *cond = dynamic_cast<Velo::AST::BinaryExpression*>(ifStmt->condition.get());
    ASSERT_NE(cond, nullptr);
    EXPECT_EQ(cond->op, Velo::AST::BinaryOperator::LogicalAnd);
}

TEST(ParserTest, ParsesArithmeticPrecedence) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    return 1 + 2 * 3;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    const auto &mainFunc = program->functions[0];
    ASSERT_EQ(mainFunc.statements.size(), 1U);
    const auto *returnStmt = dynamic_cast<ReturnStatement*>(mainFunc.statements[0].get());
    ASSERT_NE(returnStmt, nullptr);
    ASSERT_EQ(returnStmt->expression->kind, ExpressionKind::Binary);
    const auto *binary = dynamic_cast<Velo::AST::BinaryExpression*>(returnStmt->expression.get());
    ASSERT_NE(binary, nullptr);

    EXPECT_EQ(binary->op, Velo::AST::BinaryOperator::Add);
    ASSERT_EQ(binary->right->kind, ExpressionKind::Binary);

    const auto *right = dynamic_cast<Velo::AST::BinaryExpression*>(binary->right.get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->op, Velo::AST::BinaryOperator::Multiply);
}

TEST(ParserTest, ParsesGroupedArithmeticExpression) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    return (1 + 2) * 3;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto& mainFunction = program->functions[0];
    ASSERT_EQ(mainFunction.statements.size(), 1U);

    const auto* returnStatement = dynamic_cast<ReturnStatement*>(mainFunction.statements[0].get());
    ASSERT_NE(returnStatement, nullptr);

    ASSERT_EQ(returnStatement->expression->kind, ExpressionKind::Binary);

    const auto* binary = dynamic_cast<Velo::AST::BinaryExpression*>(returnStatement->expression.get());
    ASSERT_NE(binary, nullptr);

    EXPECT_EQ(binary->op, Velo::AST::BinaryOperator::Multiply);
    ASSERT_EQ(binary->left->kind, ExpressionKind::Binary);

    const auto* left = dynamic_cast<Velo::AST::BinaryExpression*>(binary->left.get());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->op, Velo::AST::BinaryOperator::Add);
}

TEST(ParserTest, ParsesStructDeclaration) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
    name: string;
    active: bool;
}

fn main(): int {
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    ASSERT_EQ(program->structs.size(), 1U);

    const auto &user = program->structs[0];
    EXPECT_FALSE(user.isPublic);
    EXPECT_EQ(user.name, "User");

    ASSERT_EQ(user.fields.size(), 3U);

    EXPECT_FALSE(user.fields[0].isPublic);
    EXPECT_EQ(user.fields[0].name, "id");
    ASSERT_EQ(user.fields[0].type.name.segments.size(), 1U);
    EXPECT_EQ(user.fields[0].type.name.segments[0], "int");

    EXPECT_EQ(user.fields[1].name, "name");
    ASSERT_EQ(user.fields[1].type.name.segments.size(), 1U);
    EXPECT_EQ(user.fields[1].type.name.segments[0], "string");

    EXPECT_EQ(user.fields[2].name, "active");
    ASSERT_EQ(user.fields[2].type.name.segments.size(), 1U);
    EXPECT_EQ(user.fields[2].type.name.segments[0], "bool");
}

TEST(ParserTest, ParsesPublicStructDeclarationAndFields) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

pub struct User {
    pub id: int;
    name: string;
}

fn main(): int {
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_EQ(program->structs.size(), 1U);

    const auto &user = program->structs[0];
    EXPECT_TRUE(user.isPublic);
    EXPECT_EQ(user.name, "User");

    ASSERT_EQ(user.fields.size(), 2U);

    EXPECT_TRUE(user.fields[0].isPublic);
    EXPECT_EQ(user.fields[0].name, "id");

    EXPECT_FALSE(user.fields[1].isPublic);
    EXPECT_EQ(user.fields[1].name, "name");
}

TEST(ParserTest, ParsesStructLiteralExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User{
        id: 1,
        name: "Alex"
    };

    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    ASSERT_EQ(program->functions.size(), 1U);
    const auto &mainFunction = program->functions[0];
    ASSERT_EQ(mainFunction.statements.size(), 2U);
    ASSERT_EQ(mainFunction.statements[0]->kind, Velo::AST::StatementKind::VariableDeclaration);

    const auto &varDecl = static_cast<const Velo::AST::VariableDeclarationStatement&>(*mainFunction.statements[0]);
    ASSERT_NE(varDecl.initializer, nullptr);
    ASSERT_EQ(varDecl.initializer->kind, Velo::AST::ExpressionKind::StructLiteral);

    const auto &literal = static_cast<const Velo::AST::StructLiteralExpression&>(*varDecl.initializer);
    ASSERT_EQ(literal.type.name.segments.size(), 1U);
    EXPECT_EQ(literal.type.name.segments[0], "User");

    ASSERT_EQ(literal.fields.size(), 2U);
    EXPECT_EQ(literal.fields[0].name, "id");
    EXPECT_EQ(literal.fields[1].name, "name");
}

TEST(ParserTest, ParsesFieldAccessExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
}

fn main(): int {
    let user: User = User{
        id: 1
    };

    return user.id;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_EQ(program->functions.size(), 1U);
    const auto &mainFunction = program->functions[0];

    ASSERT_EQ(mainFunction.statements.size(), 2U);
    ASSERT_EQ(mainFunction.statements[1]->kind, Velo::AST::StatementKind::Return);

    const auto &returnStmt = static_cast<const Velo::AST::ReturnStatement&>(*mainFunction.statements[1]);
    ASSERT_NE(returnStmt.expression, nullptr);
    ASSERT_EQ(returnStmt.expression->kind, Velo::AST::ExpressionKind::FieldAccess);

    const auto &fieldAccess = static_cast<const Velo::AST::FieldAccessExpression&>(*returnStmt.expression);
    EXPECT_EQ(fieldAccess.fieldName, "id");

    ASSERT_NE(fieldAccess.object, nullptr);
    ASSERT_EQ(fieldAccess.object->kind, Velo::AST::ExpressionKind::Name);

    const auto &object = static_cast<const Velo::AST::NameExpression&>(*fieldAccess.object);
    ASSERT_EQ(object.name.segments.size(), 1U);
    EXPECT_EQ(object.name.segments[0], "user");
}

TEST(ParserTest, ParsesNestedFieldAccessExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
}

struct Box {
    user: User;
}

fn main(): int {
    let box: Box = Box {
        user: User {
            id: 1
        }
    };

    return box.user.id;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunction = program->functions[0];
    ASSERT_EQ(mainFunction.statements[1]->kind, Velo::AST::StatementKind::Return);

    const auto &returnStmt = static_cast<const Velo::AST::ReturnStatement&>(*mainFunction.statements[1]);
    ASSERT_NE(returnStmt.expression, nullptr);
    ASSERT_EQ(returnStmt.expression->kind, Velo::AST::ExpressionKind::FieldAccess);

    const auto &idAccess = static_cast<const Velo::AST::FieldAccessExpression&>(*returnStmt.expression);
    EXPECT_EQ(idAccess.fieldName, "id");

    ASSERT_NE(idAccess.object, nullptr);
    ASSERT_EQ(idAccess.object->kind, Velo::AST::ExpressionKind::FieldAccess);

    const auto &userAccess = static_cast<const Velo::AST::FieldAccessExpression&>(*idAccess.object);
    EXPECT_EQ(userAccess.fieldName, "user");
}

TEST(ParserTest, ParsesFieldAssignmentStatement) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
}

fn main(): int {
    var user: User = User {
        id: 1
    };

    user.id = 42;

    return user.id;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    ASSERT_EQ(program->functions.size(), 1U);

    const auto &mainFunction = program->functions[0];

    ASSERT_EQ(mainFunction.statements.size(), 3U);
    ASSERT_EQ(mainFunction.statements[1]->kind, Velo::AST::StatementKind::FieldAssignment);

    const auto &assignment = static_cast<const Velo::AST::FieldAssignmentStatement&>(*mainFunction.statements[1]);
    ASSERT_NE(assignment.target, nullptr);
    EXPECT_EQ(assignment.target->fieldName, "id");
    ASSERT_NE(assignment.target->object, nullptr);
    ASSERT_EQ(assignment.target->object->kind, Velo::AST::ExpressionKind::Name);

    const auto &obj = static_cast<Velo::AST::NameExpression&>(*assignment.target->object);
    ASSERT_EQ(obj.name.segments.size(), 1U);
    EXPECT_EQ(obj.name.segments[0], "user");
}

TEST(ParserTest, ParsesNestedFieldAssignmentStatement) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
}

struct Box {
    user: User;
}

fn main(): int {
    var box: Box = Box {
        user: User {
            id: 1
        }
    };

    box.user.id = 42;

    return box.user.id;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunction = program->functions[0];
    ASSERT_EQ(mainFunction.statements.size(), 3U);
    ASSERT_EQ(mainFunction.statements[1]->kind, Velo::AST::StatementKind::FieldAssignment);

    const auto &assignment = static_cast<const Velo::AST::FieldAssignmentStatement&>(
        *mainFunction.statements[1]
    );

    ASSERT_NE(assignment.target, nullptr);
    EXPECT_EQ(assignment.target->fieldName, "id");

    ASSERT_NE(assignment.target->object, nullptr);
    ASSERT_EQ(assignment.target->object->kind, Velo::AST::ExpressionKind::FieldAccess);

    const auto &userAccess = static_cast<const Velo::AST::FieldAccessExpression&>(
        *assignment.target->object
    );

    EXPECT_EQ(userAccess.fieldName, "user");
}

TEST(ParserTest, ParsesArrayParameterType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn count(ids: []int): int {
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_EQ(program->functions.size(), 1U);
    const auto &function = program->functions[0];

    ASSERT_EQ(function.parameters.size(), 1U);
    EXPECT_EQ(function.parameters[0].type.arrayDepth, 1U);
    ASSERT_EQ(function.parameters[0].type.name.segments.size(), 1U);
    EXPECT_EQ(function.parameters[0].type.name.segments[0], "int");
}

TEST(ParserTest, ParsesNestedArrayReturnType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn matrix(): [][]int {
    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_EQ(program->functions.size(), 1U);
    const auto &function = program->functions[0];

    EXPECT_EQ(function.returnType.arrayDepth, 2U);
    ASSERT_EQ(function.returnType.name.segments.size(), 1U);
    EXPECT_EQ(function.returnType.name.segments[0], "int");
}

TEST(ParserTest, ParsesArrayStructFieldType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct Group {
    ids: []int;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_EQ(program->structs.size(), 1U);
    const auto &structDecl = program->structs[0];

    ASSERT_EQ(structDecl.fields.size(), 1U);
    EXPECT_EQ(structDecl.fields[0].type.arrayDepth, 1U);
    ASSERT_EQ(structDecl.fields[0].type.name.segments.size(), 1U);
    EXPECT_EQ(structDecl.fields[0].type.name.segments[0], "int");
}

TEST(ParserTest, ParsesArrayLiteralExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let ids: []int = [1, 2, 3];

    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_EQ(program->functions.size(), 1U);
    const auto &mainFunction = program->functions[0];

    ASSERT_EQ(mainFunction.statements.size(), 2U);
    ASSERT_EQ(mainFunction.statements[0]->kind, Velo::AST::StatementKind::VariableDeclaration);

    const auto &varDecl = static_cast<const Velo::AST::VariableDeclarationStatement&>(
        *mainFunction.statements[0]
    );

    ASSERT_NE(varDecl.initializer, nullptr);
    ASSERT_EQ(varDecl.initializer->kind, Velo::AST::ExpressionKind::ArrayLiteral);

    const auto &arrayLiteral = static_cast<const Velo::AST::ArrayLiteralExpression&>(*varDecl.initializer);
    EXPECT_EQ(arrayLiteral.elements.size(), 3U);
}

TEST(ParserTest, ParsesArrayLiteralWithTrailingComma) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let ids: []int = [
        1,
        2,
    ];

    return 0;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunction = program->functions[0];
    const auto &varDecl = static_cast<const Velo::AST::VariableDeclarationStatement&>(
        *mainFunction.statements[0]
    );

    ASSERT_EQ(varDecl.initializer->kind, Velo::AST::ExpressionKind::ArrayLiteral);

    const auto &arrayLiteral = static_cast<const Velo::AST::ArrayLiteralExpression&>(*varDecl.initializer);
    EXPECT_EQ(arrayLiteral.elements.size(), 2U);
}

TEST(ParserTest, ParsesArrayIndexExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let ids: []int = [10, 20, 30];

    return ids[1];
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunction = program->functions[0];
    ASSERT_EQ(mainFunction.statements.size(), 2U);
    ASSERT_EQ(mainFunction.statements[1]->kind, Velo::AST::StatementKind::Return);

    const auto &returnStmt = static_cast<const Velo::AST::ReturnStatement&>(*mainFunction.statements[1]);
    ASSERT_NE(returnStmt.expression, nullptr);
    ASSERT_EQ(returnStmt.expression->kind, Velo::AST::ExpressionKind::Index);

    const auto &indexExpr = static_cast<const Velo::AST::IndexExpression&>(*returnStmt.expression);
    ASSERT_NE(indexExpr.object, nullptr);
    ASSERT_NE(indexExpr.index, nullptr);

    EXPECT_EQ(indexExpr.object->kind, Velo::AST::ExpressionKind::Name);
    EXPECT_EQ(indexExpr.index->kind, Velo::AST::ExpressionKind::IntegerLiteral);
}

TEST(ParserTest, ParsesArrayIndexFollowedByFieldAccess) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    let users: []User = [
        User { id: 1 },
        User { id: 2 }
    ];

    return users[1].id;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunction = program->functions[0];
    const auto &returnStmt = static_cast<const Velo::AST::ReturnStatement&>(*mainFunction.statements[1]);

    ASSERT_NE(returnStmt.expression, nullptr);
    ASSERT_EQ(returnStmt.expression->kind, Velo::AST::ExpressionKind::FieldAccess);

    const auto &fieldAccess = static_cast<const Velo::AST::FieldAccessExpression&>(*returnStmt.expression);
    EXPECT_EQ(fieldAccess.fieldName, "id");

    ASSERT_NE(fieldAccess.object, nullptr);
    EXPECT_EQ(fieldAccess.object->kind, Velo::AST::ExpressionKind::Index);
}

TEST(ParserTest, ParsesNestedArrayIndexExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let matrix: [][]int = [
        [1, 2],
        [3, 4]
    ];

    return matrix[1][0];
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunction = program->functions[0];
    const auto &returnStmt = static_cast<const Velo::AST::ReturnStatement&>(*mainFunction.statements[1]);

    ASSERT_NE(returnStmt.expression, nullptr);
    ASSERT_EQ(returnStmt.expression->kind, Velo::AST::ExpressionKind::Index);

    const auto &outerIndex = static_cast<const Velo::AST::IndexExpression&>(*returnStmt.expression);
    ASSERT_NE(outerIndex.object, nullptr);
    EXPECT_EQ(outerIndex.object->kind, Velo::AST::ExpressionKind::Index);
}

TEST(ParserTest, ParsesArrayElementAssignmentStatement) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    var ids: []int = [1, 2, 3];

    ids[0] = 42;

    return ids[0];
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunction = program->functions[0];
    ASSERT_EQ(mainFunction.statements.size(), 3U);
    ASSERT_EQ(mainFunction.statements[1]->kind, Velo::AST::StatementKind::IndexAssignment);

    const auto &assignment = static_cast<const Velo::AST::IndexAssignmentStatement&>(
        *mainFunction.statements[1]
    );

    ASSERT_NE(assignment.target, nullptr);
    ASSERT_NE(assignment.value, nullptr);

    EXPECT_EQ(assignment.target->kind, Velo::AST::ExpressionKind::Index);
    EXPECT_EQ(assignment.value->kind, Velo::AST::ExpressionKind::IntegerLiteral);
}

TEST(ParserTest, ParsesNestedArrayElementAssignmentStatement) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    var matrix: [][]int = [
        [1, 2],
        [3, 4]
    ];

    matrix[1][0] = 99;

    return matrix[1][0];
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());

    const auto &mainFunction = program->functions[0];
    ASSERT_EQ(mainFunction.statements.size(), 3U);
    ASSERT_EQ(mainFunction.statements[1]->kind, Velo::AST::StatementKind::IndexAssignment);

    const auto &assignment = static_cast<const Velo::AST::IndexAssignmentStatement&>(
        *mainFunction.statements[1]
    );

    ASSERT_NE(assignment.target, nullptr);
    ASSERT_EQ(assignment.target->object->kind, Velo::AST::ExpressionKind::Index);
}