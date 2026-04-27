#include <gtest/gtest.h>

#include "velo/lexer/lexer.h"
#include "velo/diagnostic/diagnostic_engine.h"

using Velo::Diagnostic::DiagnosticEngine;
using Velo::Lexer::Lexer;
using Velo::Lexer::Token;
using Velo::Lexer::TokenKind;
using Velo::Source::SourceFile;

namespace {
    auto collectKinds(const std::vector<Token> &tokens) -> std::vector<TokenKind> {
        std::vector<TokenKind> kinds;
        kinds.reserve(tokens.size());
        for (const auto &token : tokens) {
            kinds.push_back(token.kind());
        }

        return kinds;
    }
}

TEST(LexerTest, LexesHelloWorldProgram) {
    const SourceFile file(
        "hello.velo",
        R"(module app;

use std::console;

fn main(): int {
    console::println("Hello, Velo!");
    return 0;
}
)"
    );

    DiagnosticEngine engine;
    Lexer lexer(file, engine);

    const auto tokens = lexer.lexAll();
    const auto kinds = collectKinds(tokens);

    const std::vector<TokenKind> expected {
        TokenKind::KwModule,
        TokenKind::Identifier,
        TokenKind::Semicolon,

        TokenKind::KwUse,
        TokenKind::Identifier,
        TokenKind::DoubleColon,
        TokenKind::Identifier,
        TokenKind::Semicolon,

        TokenKind::KwFn,
        TokenKind::Identifier,
        TokenKind::OpenParen,
        TokenKind::CloseParen,
        TokenKind::Colon,
        TokenKind::Identifier,
        TokenKind::OpenBrace,

        TokenKind::Identifier,
        TokenKind::DoubleColon,
        TokenKind::Identifier,
        TokenKind::OpenParen,
        TokenKind::StringLiteral,
        TokenKind::CloseParen,
        TokenKind::Semicolon,

        TokenKind::KwReturn,
        TokenKind::IntegerLiteral,
        TokenKind::Semicolon,
        TokenKind::CloseBrace,
        TokenKind::EndOfFile,
    };

    EXPECT_EQ(kinds, expected);
    ASSERT_FALSE(engine.hasErrors());

    ASSERT_GE(tokens.size(), 20U);
    EXPECT_EQ(tokens[1].text(), "app");
    EXPECT_EQ(tokens[6].text(), "console");
    EXPECT_EQ(tokens[19].text(), "Hello, Velo!");
    EXPECT_EQ(tokens[23].text(), "0");
}

TEST(LexerTest, SupportsUseAliasAndSkipsLineComments) {
    const SourceFile file(
        "alias.velo",
        R"(//one comment
use std::console as out;
// another comment
)"
        );

    DiagnosticEngine engine;
    Lexer lexer(file, engine);

    const auto tokens = lexer.lexAll();
    const auto kinds = collectKinds(tokens);

    const std::vector<TokenKind> expected {
        TokenKind::KwUse,
        TokenKind::Identifier,
        TokenKind::DoubleColon,
        TokenKind::Identifier,
        TokenKind::KwAs,
        TokenKind::Identifier,
        TokenKind::Semicolon,
        TokenKind::EndOfFile,
    };

    EXPECT_EQ(kinds, expected);
    ASSERT_FALSE(engine.hasErrors());
    EXPECT_EQ(tokens[5].text(), "out");
}

TEST(LexerTest, ReportsUnexpectedCharacter) {
    const SourceFile file(
        "invalid.velo",
        "@"
    );

    DiagnosticEngine engine;
    Lexer lexer(file, engine);

    const auto tokens = lexer.lexAll();

    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0].kind(), TokenKind::Invalid);
    EXPECT_EQ(tokens[1].kind(), TokenKind::EndOfFile);

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.size(), 1U);
    EXPECT_EQ(engine.diagnostics().front().code(), "LEX001");
}

TEST(LexerTest, ReportsUnterminatedStringLiteral) {
    const SourceFile file(
        "string.velo",
        "\"Hello"
    );

    DiagnosticEngine engine;
    Lexer lexer(file, engine);

    const auto tokens = lexer.lexAll();

    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0].kind(), TokenKind::Invalid);
    EXPECT_EQ(tokens[1].kind(), TokenKind::EndOfFile);

    ASSERT_TRUE(engine.hasErrors());
    ASSERT_EQ(engine.size(), 1U);
    EXPECT_EQ(engine.diagnostics().front().code(), "LEX002");
}

TEST(LexerTest, LexesPlusToken) {
    const SourceFile file("plus.velo", "a + b");
    DiagnosticEngine engine;
    Lexer lexer(file, engine);

    const auto tokens = lexer.lexAll();
    const auto kinds = collectKinds(tokens);

    const std::vector<TokenKind> expected {
        TokenKind::Identifier,
        TokenKind::Plus,
        TokenKind::Identifier,
        TokenKind::EndOfFile,
    };

    EXPECT_EQ(kinds, expected);
    EXPECT_FALSE(engine.hasErrors());
}