#include <velo/lexer/token.h>

namespace Velo::Lexer {
    Token::Token(TokenKind kind, std::string text, Source::SourceRange range) : _kind(kind), _text(std::move(text)), _range(range) {
    }

    auto Token::kind() const -> TokenKind {
        return _kind;
    }

    auto Token::text() const -> std::string_view {
        return _text;
    }

    auto Token::range() const -> const Source::SourceRange& {
        return _range;
    }

    auto Token::is(const TokenKind kind) const -> bool {
        return _kind == kind;
    }
}
