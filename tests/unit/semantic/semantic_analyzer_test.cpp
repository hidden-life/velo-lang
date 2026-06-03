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

TEST(SemanticAnalyzerTest, UsesUserFunctionReturnTypeInReturnValidation) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn text(): string {
    return "hello";
}

fn main(): int {
    return text();
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM014");
}

TEST(SemanticAnalyzerTest, ReportsVoidBuiltinReturnMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::console;

fn main(): int {
    return console::println("hello");
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM014");
}

TEST(SemanticAnalyzerTest, UsesBuiltinReturnTypeThroughImportAlias) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::console as out;

fn main(): int {
    return out::println("hello");
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

TEST(SemanticAnalyzerTest, AcceptsVoidFunctionWithoutEmptyReturn) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn log(): void {
    return;
}

fn main(): int {
    log();
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

TEST(SemanticAnalyzerTest, AcceptsValuesReturnFromVoidFunction) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn broken(): void {
    return 1;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM016");
}

TEST(SemanticAnalyzerTest, ReportsEmptyReturnFromNonVoidFunction) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn broken(): int {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.diagnostics().front().code(), "SEM015");
}

TEST(SemanticAnalyzerTest, ReportsMissingFinalReturnInNonVoidFunction) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::console;

fn broken(): int {
    console::println("missing return");
}

fn main(): int {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM017");
}

TEST(SemanticAnalyzerTest, AcceptsFinalReturnInNonVoidFunction) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn valid(): int {
    return 42;
}

fn main(): int {
    return valid();
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

TEST(SemanticAnalyzerTest, AcceptsLocalVariableDeclaration) {
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
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsMutableVariableAssignment) {
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
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_TRUE(analyzer.analyze());
}

TEST(SemanticAnalyzerTest, AcceptsIfElseWithReturningBranches) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    if (true) {
        return 42;
    } else {
        return 0;
    }
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

TEST(SemanticAnalyzerTest, ReportsNonBoolIfCondition) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    if (1) {
        return 42;
    } else {
        return 0;
    }
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM023");
}

TEST(SemanticAnalyzerTest, AcceptsIntegerComparisonCondition) {
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
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsInvalidComparisonOperands) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    if ("a" > 10) {
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
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_FALSE(analyzer.analyze());
    ASSERT_TRUE(engine.hasErrors());
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM024");
}

TEST(SemanticAnalyzerTest, AcceptsWhileWithBoolCondition) {
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
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_TRUE(analyzer.analyze());
    ASSERT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsNonBoolWhileCondition) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    while(1) {
        return 0;
    }

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM025");
}

TEST(SemanticAnalyzerTest, ReportsUseOfVariableOutsideScope) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    if (true) {
        let x: int = 42;
    }

    return x;
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
}

TEST(SemanticAnalyzerTest, AllowsShadowingInInnerScope) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    let x: int = 1;
    if (true) {
        let x: int = 2;
        return x;
    }

    return x;
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    const bool ok = analyzer.analyze();
    if (!ok) {
        for (const auto &diag : engine.diagnostics()) {
            ADD_FAILURE() << diag.code() << ": " << diag.message();
        }
    }
    EXPECT_TRUE(ok);
}

TEST(SemanticAnalyzerTest, AcceptsBreakInsideLoop) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    while(true) {
        break;
    }

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

TEST(SemanticAnalyzerTest, AcceptsContinueInsideLoop) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    var x: int = 0;

    while(x < 1) {
        x = x + 1;
        continue;
    }

    return x;
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

TEST(SemanticAnalyzerTest, ReportsBreakOutsideLoop) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    break;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM026");
}

TEST(SemanticAnalyzerTest, ReportsContinueOutsideLoop) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    continue;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM027");
}

TEST(SemanticAnalyzerTest, AcceptsLogicalOperatorsWithBoolOperands) {
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
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    EXPECT_TRUE(analyzer.analyze());
    ASSERT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsInvalidLogicalNotOperand) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    if (!1) {
        return 1;
    }

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM028");
}

TEST(SemanticAnalyzerTest, ReportsInvalidLogicalBinaryOperands) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    if (1 && true) {
        return 1;
    }

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM029");
}

TEST(SemanticAnalyzerTest, AcceptsStringParameterReference) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn echo(val: string): string {
    return val;
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

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsBoolParameterReference) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn identity(ok: bool): bool {
    return ok;
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

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsReturnMismatchForStringParameterInIntFunction) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn broken(val: string): int {
    return val;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM014");
}

TEST(SemanticaAnalyzerTest, ReportsUnknownFunctionReturnType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn broken(): mystery {
    return 0;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM030");
}

TEST(SemanticAnalyzerTest, ReportsUnknownParameterType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn broken(value: mystery): int {
    return 0;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM030");
}

TEST(SemanticAnalyzerTest, ReportsUnknownLocalVariableType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    let val: mystery = 1;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM030");
}

TEST(SemanticAnalyzerTest, ReportsVoidParameterType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn broken(value: void): int {
    return 0;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM031");
}

TEST(SemanticAnalyzerTest, ReportsVoidLocalVariableType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    let value: void = 0;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM031");
}

TEST(SemanticAnalyzerTest, AcceptsIntegerArithmeticExpression) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    return (1 + 2) * 3 - 4 / 2;
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

TEST(SemanticAnalyzerTest, ReportsInvalidArithmeticOperands) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    return "a" - 1;
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

TEST(SemanticAnalyzerTest, ReportsInvalidUnaryMinusOperand) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
fn main(): int {
    return -true;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM032");
}

TEST(SemanticAnalyzerTest, AcceptsStringLengthBuiltinReturnType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::string as str;

fn main(): int {
    return str::len("hello");
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

TEST(SemanticAnalyzerTest, ReportsWrongStringLengthArgumentCount) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::string as str;

fn main(): int {
    return str::len("hello", "world");
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM010");
}

TEST(SemanticAnalyzerTest, ReportsStringLengthArgumentTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::string as str;

fn main(): int {
    return str::len(123);
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM033");
}

TEST(SemanticAnalyzerTest, AcceptsConsolePrintlnAnyArgumentType) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

use std::console;

fn main(): int {
    console::println(123);
    console::println("hello");
    console::println(true);

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

TEST(SemanticAnalyzerTest, AcceptsIntToStringBuiltinReturnType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::int as ints;
use std::string as str;

fn main(): int {
    return str::len(ints::toString(123));
}
)",
        engine
    );

    ASSERT_NE(program, nullptr);
    ASSERT_FALSE(engine.hasErrors());
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());
    const bool isOk = analyzer.analyze();
    if (!isOk) {
        for (const auto &diag : engine.diagnostics()) {
            ADD_FAILURE() << diag.code() << ": " << diag.message();
        }
    }
    EXPECT_TRUE(isOk);
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsBoolToStringBuiltinReturnType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::bool as bools;
use std::string as str;

fn main(): int {
    return str::len(bools::toString(true));
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

TEST(SemanticAnalyzerTest, ReportsIntToStringArgumentTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::int as ints;

fn main(): int {
    return ints::toString("bad");
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM033");
}

TEST(SemanticAnalyzerTest, ReportsBoolToStringArgumentTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
use std::bool as bools;

fn main(): int {
    return bools::toString(1);
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM033");
}

TEST(SemanticAnalyzerTest, AcceptsStructDeclarationWithBuiltinFieldTypes) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

pub struct User {
    pub id: int;
    pub name: string;
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsDuplicateStructDeclaration) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
}

struct User {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM034");
}

TEST(SemanticAnalyzerTest, ReportsDuplicateStructField) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
    id: string;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM035");
}

TEST(SemanticAnalyzerTest, ReportsUnknownStructFieldType) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

struct User {
    profile: Profile;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM030");
}

TEST(SemanticAnalyzerTest, ReportsVoidStructFieldType) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

struct User {
    value: void;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM031");
}

TEST(SemanticAnalyzerTest, AcceptsStructFieldWithUserDefinedType) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

struct Profile {
    id: int;
}

struct User {
    profile: Profile;
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

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsForwardDeclaredStructFieldType) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

struct User {
    profile: Profile;
}

struct Profile {
    id: int;
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

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsStructNameConflictingWithBuiltinType) {
    DiagnosticEngine engine;

    const auto program = parseProgram(
        R"(module app;

struct int {
    value: int;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM036");
}

TEST(SemanticAnalyzerTest, AcceptsStructTypeInFunctionParameterAndReturnType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
    name: string;
}

fn identity(user: User): User {
    return user;
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
    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsStructTypeInLocalVariableDeclaration) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
}

fn useUser(user: User): int {
    let copy: User = user;
    return 0;
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
    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsStructLocalIntializerTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
}

struct Profile {
    id: int;
}

fn useProfile(profile: Profile): int {
    let user: User = profile;
    return 0;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM019");
}

TEST(SemanticAnalyzerTest, ReportsStructReturnTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
}

struct Profile {
    id: int;
}

fn getUser(profile: Profile): User {
    return profile;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM014");
}

TEST(SemanticAnalyzerTest, AcceptsStructFunctionArgumentType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
}

fn accept(user: User): int {
    return 1;
}

fn callAccept(user: User): int {
    return accept(user);
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
    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsStructFunctionArgumentTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
}

fn accept(user: User): int {
    return 1;
}

fn main(): int {
    return accept(123);
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM037");
}

TEST(SemanticAnalyzerTest, ReportsUnknownStructTypeInFunctionParameter) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn accept(user: User): int {
    return 0;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM030");
}

TEST(SemanticAnalyzerTest, AcceptsStructLiteral) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
    name: string;
    active: bool;
}

fn main(): int {
    let user: User = User {
        id: 1,
        name: "Alex",
        active: true
    };

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

TEST(SemanticAnalyzerTest, AcceptsStructLiteralWithDifferentFieldOrder) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
    name: string;
    active: bool;
}

fn main(): int {
    let user: User = User {
        active: true,
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
    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsUnknownStructLiteralType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let user: User = User {
        id: 1,
    };

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM030");
}

TEST(SemanticAnalyzerTest, ReportsStructLiteralFieldTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: "bad",
        name: "Alex"
    };

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM041");
}

TEST(SemanticAnalyzerTest, ReportsMissingStructLiteralField) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;
struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: 1
    };

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM042");
}

TEST(SemanticAnalyzerTest, AcceptsStructFieldAccess) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: 1,
        name: "Alex"
    };

    return user.id;
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

TEST(SemanticAnalyzerTest, AcceptsNestedStructFieldAccess) {
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
            id: 42
        }
    };

    return box.user.id;
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

TEST(SemanticAnalyzerTest, ReportsFieldAccessOnNonStructValue) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let value: int = 1;
    return value.id;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM043");
}

TEST(SemanticAnalyzerTest, ReportsUnknownStructFieldAccess) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    let user: User = User {
        id: 1
    };

    return user.name;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM044");
}

TEST(SemanticAnalyzerTest, AcceptsStructFieldAssignment) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    var user: User = User {
        id: 1,
        name: "Alex"
    };

    user.id = 42;

    return user.id;
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

TEST(SemanticAnalyzerTest, AcceptsNestedStructFieldAssignment) {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsFieldAssignmentThroughImmutableLocal) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    let user: User = User {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_FALSE(analyzer.analyze());

    ASSERT_TRUE(engine.hasErrors());
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM046");
}

TEST(SemanticAnalyzerTest, ReportsStructFieldAssignmentTypeMismatch) {
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

    user.id = "bad";

    return user.id;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM047");
}

TEST(SemanticAnalyzerTest, ReportsUnknownStructFieldAssignment) {
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

    user.name = "Alex";

    return user.id;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM044");
}

TEST(SemanticAnalyzerTest, ReportsFieldAssignmentOnNonStructValue) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    var value: int = 1;

    value.id = 42;

    return value;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM043");
}

TEST(SemanticAnalyzerTest, AcceptsStringEquality) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    if ("Alex" == "Alex") {
        return 1;
    }

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

TEST(SemanticAnalyzerTest, AcceptsStringInequality) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    if ("Alex" != "Bob") {
        return 1;
    }

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

TEST(SemanticAnalyzerTest, AcceptsBoolEquality) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    if (true == true) {
        return 1;
    }

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

TEST(SemanticAnalyzerTest, AcceptsBoolInequality) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    if (true != false) {
        return 1;
    }

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

TEST(SemanticAnalyzerTest, AcceptsStructStringFieldEquality) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    name: string;
}

fn main(): int {
    let user: User = User {
        name: "Alex"
    };

    if (user.name == "Alex") {
        return 1;
    }

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

TEST(SemanticAnalyzerTest, AcceptsStructBoolFieldEquality) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    active: bool;
}

fn main(): int {
    let user: User = User {
        active: true
    };

    if (user.active == true) {
        return 1;
    }

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

TEST(SemanticAnalyzerTest, ReportsEqualityTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    if ("1" == 1) {
        return 1;
    }

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM048");
}

TEST(SemanticAnalyzerTest, ReportsStructEqualityUnsupported) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    let left: User = User {
        id: 1
    };

    let right: User = User {
        id: 1
    };

    if (left == right) {
        return 1;
    }

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM048");
}

TEST(SemanticAnalyzerTest, ReportsStringOrderingComparisonUnsupported) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    if ("a" < "b") {
        return 1;
    }

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM024");
}

TEST(SemanticAnalyzerTest, AcceptsLocalShadowingInsideIfBlock) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let value: int = 1;

    if (true) {
        let value: int = 2;
        return value;
    }

    return value;
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

TEST(SemanticAnalyzerTest, AcceptsLocalShadowingInsideWhileBlock) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    var counter: int = 0;

    while (counter == 0) {
        let counter: int = 1;
        return counter;
    }

    return counter;
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

TEST(SemanticAnalyzerTest, ReportsDuplicateLocalInSameBlockScope) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let value: int = 1;
    let value: int = 2;

    return value;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM018");
}

TEST(SemanticAnalyzerTest, ReportsIfBlockLocalUsedOutsideScope) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    if (true) {
        let value: int = 1;
    }

    return value;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM007");
}

TEST(SemanticAnalyzerTest, ReportsWhileBlockLocalUsedOutsideScope) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    while (false) {
        let value: int = 1;
    }

    return value;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM007");
}

TEST(SemanticAnalyzerTest, AcceptsArrayTypeInFunctionParameter) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn count(ids: []int): int {
    return 0;
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

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsArrayTypeInStructField) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct Group {
    ids: []int;
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

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsArrayOfUserDefinedStruct) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
}

struct Group {
    users: []User;
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

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsNestedArrayType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct MatrixHolder {
    values: [][]int;
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

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsUnknownArrayElementType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct Group {
    users: []MissingUser;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM030");
}

TEST(SemanticAnalyzerTest, ReportsVoidArrayElementType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct Bad {
    values: []void;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM031");
}

TEST(SemanticAnalyzerTest, ReportsArrayReturnTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn values(): []int {
    return 1;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM014");
}

TEST(SemanticAnalyzerTest, AcceptsIntArrayLiteral) {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsStringArrayLiteral) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let names: []string = ["Alex", "Bob"];

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

TEST(SemanticAnalyzerTest, AcceptsBoolArrayLiteral) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let flags: []bool = [true, false];

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

TEST(SemanticAnalyzerTest, AcceptsArrayLiteralOfStructValues) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    let users: []User = [
        User { id: 1, name: "Alex" },
        User { id: 2, name: "Bob" }
    ];

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

TEST(SemanticAnalyzerTest, AcceptsEmptyArrayLiteralWithDeclaredType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let ids: []int = [];

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

TEST(SemanticAnalyzerTest, ReportsArrayLiteralElementTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let values: []int = [1, "bad"];

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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM050");
}

TEST(SemanticAnalyzerTest, ReportsCannotInferEmptyArrayLiteralWithoutArrayExpectedType) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    return [];
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM049");
}

TEST(SemanticAnalyzerTest, AcceptsArrayIndexRead) {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsArrayIndexReadOnStructArray) {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsNestedArrayIndexRead) {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, ReportsIndexReadOnNonArrayValue) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let value: int = 10;

    return value[0];
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM051");
}

TEST(SemanticAnalyzerTest, ReportsNonIntArrayIndex) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let ids: []int = [10, 20, 30];

    return ids["bad"];
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM052");
}

TEST(SemanticAnalyzerTest, AcceptsArrayElementAssignment) {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsNestedArrayElementAssignment) {
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

    Velo::Runtime::Runtime runtime;
    SemanticAnalyzer analyzer(*program, engine, runtime.modules());

    EXPECT_TRUE(analyzer.analyze());
    EXPECT_FALSE(engine.hasErrors());
}

TEST(SemanticAnalyzerTest, AcceptsArrayElementAssignmentWithStructValue) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    var users: []User = [
        User { id: 1, name: "Alex" }
    ];

    users[0] = User { id: 2, name: "Bob" };

    return users[0].id;
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

TEST(SemanticAnalyzerTest, ReportsArrayElementAssignmentThroughImmutableLocal) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    let ids: []int = [1, 2, 3];

    ids[0] = 42;

    return ids[0];
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM054");
}

TEST(SemanticAnalyzerTest, ReportsArrayElementAssignmentTypeMismatch) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    var ids: []int = [1, 2, 3];

    ids[0] = "bad";

    return ids[0];
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM055");
}

TEST(SemanticAnalyzerTest, ReportsArrayElementAssignmentOnNonArrayValue) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    var value: int = 1;

    value[0] = 42;

    return value;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM051");
}

TEST(SemanticAnalyzerTest, ReportsArrayElementAssignmentWithNonIntIndex) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

fn main(): int {
    var ids: []int = [1, 2, 3];

    ids["bad"] = 42;

    return ids[0];
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM052");
}

TEST(SemanticAnalyzerTest, ReportsFieldAssignmentThroughArrayIndexUnsupported) {
    DiagnosticEngine engine;
    const auto program = parseProgram(
        R"(module app;

struct User {
    id: int;
}

fn main(): int {
    var users: []User = [
        User { id: 1 }
    ];

    users[0].id = 2;

    return users[0].id;
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
    EXPECT_EQ(engine.diagnostics().front().code(), "SEM045");
}