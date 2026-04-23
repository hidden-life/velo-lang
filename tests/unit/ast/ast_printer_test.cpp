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