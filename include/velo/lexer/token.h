#ifndef INC_VELO_LEXER_TOKEN_H
#define INC_VELO_LEXER_TOKEN_H

#include <string>
#include <string_view>

#include "token_kind.h"
#include "velo/source/source_range.h"

namespace Velo::Lexer {
    // A single token produced from source code.
    class Token final {
    public:
        Token(TokenKind kind, std::string text, Source::SourceRange range);

        [[nodiscard]] auto kind() const -> TokenKind;
        [[nodiscard]] auto text() const -> std::string_view;
        [[nodiscard]] auto range() const -> const Source::SourceRange&;

        [[nodiscard]] auto is(TokenKind kind) const -> bool;

    private:
        TokenKind _kind {TokenKind::Invalid};
        std::string _text;
        Source::SourceRange _range;
    };
}

#endif //INC_VELO_LEXER_TOKEN_H
