#ifndef INC_VELO_PARSER_PARSER_H
#define INC_VELO_PARSER_PARSER_H

#include <vector>
#include <memory>

#include "velo/ast/ast.h"
#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/lexer/token.h"

namespace Velo::Parser {
    // Minimal recursive descent parser for the first executable Velo program.
    class Parser final {
    public:
        Parser(std::vector<Lexer::Token> tokens, Diagnostic::DiagnosticEngine &engine);

        [[nodiscard]] auto parse() -> std::unique_ptr<AST::Program>;

    private:
        [[nodiscard]] auto isAtEnd() const -> bool;
        [[nodiscard]] auto current() const -> const Lexer::Token&;
        [[nodiscard]] auto previous() const -> const Lexer::Token&;
        [[nodiscard]] auto advance() -> const Lexer::Token&;
        [[nodiscard]] auto check(Lexer::TokenKind kind) const -> bool;
        [[nodiscard]] auto match(Lexer::TokenKind kind) -> bool;
        [[nodiscard]] auto consume(Lexer::TokenKind kind, const char *code, const char *message) -> const Lexer::Token*;

        [[nodiscard]] auto parseModuleDeclaration() -> std::optional<AST::ModuleDeclaration>;
        [[nodiscard]] auto parseUseDeclaration() -> std::optional<AST::UseDeclaration>;
        [[nodiscard]] auto parseFunctionDeclaration(bool isPublic) -> std::optional<AST::FunctionDeclaration>;
        [[nodiscard]] auto parseTypeName() -> std::optional<AST::TypeName>;
        [[nodiscard]] auto parseQualifiedName() -> std::optional<AST::QualifiedName>;
        [[nodiscard]] auto parseStatement() -> std::unique_ptr<AST::Statement>;
        [[nodiscard]] auto parseExpression() -> std::unique_ptr<AST::Expression>;
        [[nodiscard]] auto parsePrimaryExpression() -> std::unique_ptr<AST::Expression>;
        [[nodiscard]] auto parseCallExpressionOrName() -> std::unique_ptr<AST::Expression>;

        [[nodiscard]] auto makeRangeFromTokens(
            const Lexer::Token &begin, const Lexer::Token &end
        ) const -> Source::SourceRange;

        [[nodiscard]] auto makeFallbackRange() const -> Source::SourceRange;

        [[nodiscard]] auto peekNext() const -> const Lexer::Token&;

        void sync();
        void reportCurrent(const char *code, const char *message);

        std::vector<Lexer::Token> _tokens;
        Diagnostic::DiagnosticEngine &_engine;
        std::size_t _position {0};
    };
}

#endif //INC_VELO_PARSER_PARSER_H
