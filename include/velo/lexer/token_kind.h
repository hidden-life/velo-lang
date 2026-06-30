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
        BooleanLiteral, // <bool>

        KwModule, // "module"
        KwUse, // "use"
        KwAs, // "as"
        KwFn, // "fn"
        KwReturn, // "return"
        KwPub, // "pub"
        KwWhile, // "while"

        DoubleColon, // ::
        Colon, // :
        Semicolon, // ;
        Comma, // ,
        Dot, // .
        OpenParen, // (
        CloseParen, // )
        OpenBrace, // {
        CloseBrace, // }
        OpenBracket, // [
        CloseBracket, // ]
        At, // @

        Plus, // +
        Minus, // -
        Star, // *
        Slash, // /
        Percent, // %
        Equal,
        // comparison
        EqualEqual,
        BangEqual,
        Less,
        Greater,
        LessEqual,
        GreaterEqual,

        KwLet, // variables
        KwVar,

        KwIf,
        KwElse,

        KwBreak,
        KwContinue,

        // logical operators
        LogicalAnd,
        LogicalOr,
        Bang,

        KwStruct, // "struct"
    };

    [[nodiscard]] auto toString(TokenKind kind) -> std::string_view;
}

#endif //INC_VELO_LEXER_TOKEN_KIND_H
