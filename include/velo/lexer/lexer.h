#ifndef INC_VELO_LEXER_LEXER_H
#define INC_VELO_LEXER_LEXER_H

#include "token.h"
#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/source/source_file.h"

namespace Velo::Lexer {
    // Handwritten Velo lexer. It reads source text and emits a token stream.
    class Lexer final {
    public:
        Lexer(const Source::SourceFile &file, Diagnostic::DiagnosticEngine &engine);

        [[nodiscard]] auto nextToken() -> Token;
        [[nodiscard]] auto lexAll() -> std::vector<Token>;

    private:
        [[nodiscard]] auto isAtEnd() const -> bool;
        [[nodiscard]] auto peek() const -> char;
        [[nodiscard]] auto peekNext() const -> char;
        auto advance() -> char;

        void skipTrivia();

        [[nodiscard]] auto lexIdentifierOrKeyword() -> Token;
        [[nodiscard]] auto lexIntegerLiteral() -> Token;
        [[nodiscard]] auto lexStringLiteral() -> Token;

        [[nodiscard]] auto makeToken(
            TokenKind kind,
            std::string text,
            std::size_t beginOffset,
            std::size_t endOffset
        ) const -> Token;

        void reportInvalidCharacter(std::size_t offset);
        void reportUnterminatedString(std::size_t beginOffset, std::size_t endOffset);
        void reportInvalidEscape(std::size_t escapeOffset);

        [[nodiscard]] static auto isIdentifierStart(char c) -> bool;
        [[nodiscard]] static auto isIdentifierPart(char c) -> bool;
        [[nodiscard]] static auto isDigit(char c) -> bool;
        [[nodiscard]] static auto keywordKind(std::string_view text) -> TokenKind;

        const Source::SourceFile &_sourceFile;
        Diagnostic::DiagnosticEngine &_diagEngine;
        std::size_t _offset {0};
    };
}

#endif //INC_VELO_LEXER_LEXER_H
