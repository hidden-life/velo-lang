#ifndef INC_VELO_LEXER_TOKEN_KIND_H
#define INC_VELO_LEXER_TOKEN_KIND_H

#include <string_view>

namespace Velo::Lexer {
    // All token kinds currently supported by the Velo lexer
    enum class TokenKind {
        EndOfFile,
        Invalid,

        Identifier,
        IntegerLiteral, // <int>
        StringLiteral, // <string>

        KwModule, // "module"
        KwUse, // "use"
        KwAs, // "as"
        KwFn, // "fn"
        KwReturn, // "return"
        KwPub, // "pub"

        DoubleColon, // ::
        Colon, // :
        Semicolon, // ;
        Comma, // ,
        OpenParen, // (
        CloseParen, // )
        OpenBrace, // {
        CloseBrace, // }

        Plus,
    };

    [[nodiscard]] auto toString(TokenKind kind) -> std::string_view;
}

#endif //INC_VELO_LEXER_TOKEN_KIND_H
