#include <gtest/gtest.h>
#include "velo/ast/ast_printer.h"
#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/lexer/lexer.h"
#include "velo/parser/parser.h"

using Velo::AST::ASTPrinter;
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

TEST(AstPrinterTest, PrintsHelloWorldProgram) {
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

    ASTPrinter printer;
    const std::string out = printer.print(*program);

    const std::string expected =
        "Program\n"
        "  Module app\n"
        "  Use std::console\n"
        "  Function main -> int\n"
        "    ExprStmt\n"
        "      Call console::println\n"
        "        String \"Hello, Velo!\"\n"
        "    Return\n"
        "      Integer 0\n"
    ;

    EXPECT_EQ(out, expected);
}

TEST(AstPrinterTest, PrintsStructDeclaration) {
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

    ASTPrinter printer;
    const std::string output = printer.print(*program);

    EXPECT_NE(output.find("Struct pub User"), std::string::npos);
    EXPECT_NE(output.find("Field pub id : int"), std::string::npos);
    EXPECT_NE(output.find("Field name : string"), std::string::npos);
}

TEST(AstPrinterTest, PrintsStructFieldWithUserDefinedType) {
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

    ASTPrinter printer;
    const std::string output = printer.print(*program);

    EXPECT_NE(output.find("Struct Profile"), std::string::npos);
    EXPECT_NE(output.find("Struct User"), std::string::npos);
    EXPECT_NE(output.find("Field profile : Profile"), std::string::npos);
}