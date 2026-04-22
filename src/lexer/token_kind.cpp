#include <velo/lexer/token_kind.h>

namespace Velo::Lexer {
    auto toString(const TokenKind kind) -> std::string_view {
        switch (kind) {
            case TokenKind::EndOfFile:
                return "EndOfFile";
            case TokenKind::Invalid:
                return "Invalid";
            case TokenKind::Identifier:
                return "Identifier";
            case TokenKind::IntegerLiteral:
                return "IntegerLiteral";
            case TokenKind::StringLiteral:
                return "StringLiteral";
            case TokenKind::KwModule:
                return "KwModule";
            case TokenKind::KwUse:
                return "KwUse";
            case TokenKind::KwAs:
                return "KwAs";
            case TokenKind::KwFn:
                return "KwFn";
            case TokenKind::KwReturn:
                return "KwReturn";
            case TokenKind::KwPub:
                return "KwPub";
            case TokenKind::DoubleColon:
                return "DoubleColon";
            case TokenKind::Colon:
                return "Colon";
            case TokenKind::Semicolon:
                return "Semicolon";
            case TokenKind::Comma:
                return "Comma";
            case TokenKind::OpenParen:
                return "OpenParen";
            case TokenKind::CloseParen:
                return "CloseParen";
            case TokenKind::OpenBrace:
                return "OpenBrace";
            case TokenKind::CloseBrace:
                return "CloseBrace";
        }

        return "Unknown";
    }
}
