#include <velo/lexer/lexer.h>

namespace Velo::Lexer {
    Lexer::Lexer(const Source::SourceFile &file, Diagnostic::DiagnosticEngine &engine): _sourceFile(file), _diagEngine(engine) {
    }

    auto Lexer::nextToken() -> Token {
        skipTrivia();

        const std::size_t beginOffset = _offset;
        if (isAtEnd()) {
            return makeToken(TokenKind::EndOfFile, "", beginOffset, beginOffset);
        }

        const char c = peek();
        switch (c) {
            case ':':
                if (peekNext() == ':') {
                    advance();
                    advance();

                    return makeToken(TokenKind::DoubleColon, "::", beginOffset, beginOffset + 1U);
                }

                advance();
                return makeToken(TokenKind::Colon, ":", beginOffset, beginOffset);

            case ';':
                advance();
                return makeToken(TokenKind::Semicolon, ";", beginOffset, beginOffset);

            case ',':
                advance();
                return makeToken(TokenKind::Comma, ",", beginOffset, beginOffset);

            case '+':
                advance();
                return makeToken(TokenKind::Plus, "+", beginOffset, beginOffset);

            case '(':
                advance();
                return makeToken(TokenKind::OpenParen, "(", beginOffset, beginOffset);

            case ')':
                advance();
                return makeToken(TokenKind::CloseParen, ")", beginOffset, beginOffset);

            case '{':
                advance();
                return makeToken(TokenKind::OpenBrace, "{", beginOffset, beginOffset);

            case '}':
                advance();
                return makeToken(TokenKind::CloseBrace, "}", beginOffset, beginOffset);

            case '"':
                return lexStringLiteral();

            case '=':
                advance();
                return makeToken(TokenKind::Equal, "=", beginOffset, beginOffset);

            default:
                break;
        }

        if (isIdentifierStart(c)) {
            return lexIdentifierOrKeyword();
        }

        if (isDigit(c)) {
            return lexIntegerLiteral();
        }

        advance();
        reportInvalidCharacter(beginOffset);

        return makeToken(TokenKind::Invalid, std::string(1, c), beginOffset, beginOffset);
    }

    auto Lexer::lexAll() -> std::vector<Token> {
        std::vector<Token> tokens;
        while (true) {
            Token token = nextToken();
            const bool isEndOfFile = token.is(TokenKind::EndOfFile);
            tokens.push_back(std::move(token));

            if (isEndOfFile) {
                break;
            }
        }

        return tokens;
    }

    auto Lexer::isAtEnd() const -> bool {
        return _offset >= _sourceFile.size();
    }

    auto Lexer::peek() const -> char {
        if (isAtEnd()) {
            return '\0';
        }

        return _sourceFile.content()[_offset];
    }

    auto Lexer::peekNext() const -> char {
        const std::size_t nextOffset = _offset + 1U;
        if (nextOffset >= _sourceFile.size()) {
            return '\0';
        }

        return _sourceFile.content()[nextOffset];
    }

    auto Lexer::advance() -> char {
        const char c = peek();
        if (!isAtEnd()) {
            ++_offset;
        }

        return c;
    }

    void Lexer::skipTrivia() {
        while (!isAtEnd()) {
            const char c = peek();
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                advance();
                continue;
            }

            if (c == '/' && peekNext() == '/') {
                while (!isAtEnd() && peek() != '\n') {
                    advance();
                }
                continue;
            }

            break;
        }
    }

    auto Lexer::lexIdentifierOrKeyword() -> Token {
        const std::size_t beginOffset = _offset;
        while (!isAtEnd() && isIdentifierPart(peek())) {
            advance();
        }

        const std::size_t endOffset = _offset - 1U;
        const std::size_t length = _offset - beginOffset;
        const std::string text = _sourceFile.content().substr(beginOffset, length);

        return makeToken(keywordKind(text), text, beginOffset, endOffset);
    }

    auto Lexer::lexIntegerLiteral() -> Token {
        const std::size_t beginOffset = _offset;
        while (!isAtEnd() && isDigit(peek())) {
            advance();
        }

        const std::size_t endOffset = _offset - 1U;
        const std::size_t length = _offset - beginOffset;
        const std::string text = _sourceFile.content().substr(beginOffset, length);

        return makeToken(TokenKind::IntegerLiteral, text, beginOffset, endOffset);
    }

    auto Lexer::lexStringLiteral() -> Token {
        const std::size_t beginOffset = _offset;
        advance();

        std::string value;

        while (!isAtEnd()) {
            const char c = advance();
            if (c == '"') {
                return makeToken(TokenKind::StringLiteral, value, beginOffset, _offset - 1U);
            }

            if (c == '\\') {
                if (isAtEnd()) {
                    reportUnterminatedString(beginOffset, _offset - 1U);
                    return makeToken(TokenKind::Invalid, value, beginOffset, _offset - 1U);
                }

                const std::size_t escapeOffset = _offset;
                const char escapeChar = advance();
                switch (escapeChar) {
                    case 'n':
                        value.push_back('\n');
                        break;
                    case 't':
                        value.push_back('\t');
                        break;
                    case 'r':
                        value.push_back('\r');
                        break;
                    case '\\':
                        value.push_back('\\');
                        break;
                    case '"':
                        value.push_back('"');
                        break;
                    default:
                        reportInvalidEscape(escapeOffset);
                        value.push_back(escapeChar);
                        break;
                }

                continue;
            }

            if (c == '\n') {
                reportUnterminatedString(beginOffset, _offset - 1U);
                return makeToken(TokenKind::Invalid, value, beginOffset, _offset - 1U);
            }

            value.push_back(c);
        }

        const std::size_t endOffset = _offset == 0U ? 0U : (_offset - 1U);
        reportUnterminatedString(beginOffset, endOffset);

        return makeToken(TokenKind::Invalid, value, beginOffset, endOffset);
    }

    auto Lexer::makeToken(
        TokenKind kind,
        std::string text,
        const std::size_t beginOffset,
        const std::size_t endOffset
        ) const -> Token {
        const std::optional<Source::SourceRange> range = _sourceFile.range(beginOffset, endOffset);

        return {kind, std::move(text), range.value_or(Source::SourceRange {})};
    }

    void Lexer::reportInvalidCharacter(const std::size_t offset) {
        const auto range = _sourceFile.range(offset, offset);
        if (range.has_value()) {
            _diagEngine.error("LEX001", "Unexpected character.", *range);
        }
    }

    void Lexer::reportUnterminatedString(const std::size_t beginOffset, const std::size_t endOffset) {
        const auto range = _sourceFile.range(beginOffset, endOffset);
        if (range.has_value()) {
            _diagEngine.error("LEX002", "Unterminated string literal.", *range);
        }
    }

    void Lexer::reportInvalidEscape(std::size_t escapeOffset) {
        const auto range = _sourceFile.range(escapeOffset, escapeOffset);
        if (range.has_value()) {
            _diagEngine.error("LEX003", "Invalid escape sequence.", *range);
        }
    }

    auto Lexer::isIdentifierStart(const char c) -> bool {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    auto Lexer::isIdentifierPart(const char c) -> bool {
        return isIdentifierStart(c) || isDigit(c);
    }

    auto Lexer::isDigit(const char c) -> bool {
        return c >= '0' && c <= '9';
    }

    auto Lexer::keywordKind(const std::string_view text) -> TokenKind {
        if (text == "module") {
            return TokenKind::KwModule;
        }

        if (text == "use") {
            return TokenKind::KwUse;
        }

        if (text == "as") {
            return TokenKind::KwAs;
        }

        if (text == "fn") {
            return TokenKind::KwFn;
        }

        if (text == "return") {
            return TokenKind::KwReturn;
        }

        if (text == "pub") {
            return TokenKind::KwPub;
        }

        if (text == "let") {
            return TokenKind::KwLet;
        }

        if (text == "var") {
            return TokenKind::KwVar;
        }

        if (text == "if") {
            return TokenKind::KwIf;
        }

        if (text == "else") {
            return TokenKind::KwElse;
        }

        if (text == "true" || text == "false") {
            return TokenKind::BooleanLiteral;
        }

        return TokenKind::Identifier;
    }
}
