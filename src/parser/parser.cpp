#include <velo/parser/parser.h>
#include <utility>

namespace Velo::Parser {
    using Lexer::Token;
    using Lexer::TokenKind;

    Parser::Parser(std::vector<Token> tokens, Diagnostic::DiagnosticEngine &engine) :
        _tokens(std::move(tokens)),
        _engine(engine) {
    }

    auto Parser::parse() -> std::unique_ptr<AST::Program> {
        auto program = std::make_unique<AST::Program>();
        if (match(TokenKind::KwModule)) {
            _position -= 1U;
            const auto module = parseModuleDeclaration();
            if (module.has_value()) {
                program->module = std::move(*module);
            }
        }

        while (match(TokenKind::KwUse)) {
            _position -= 1U;
            const auto useDeclaration = parseUseDeclaration();
            if (useDeclaration.has_value()) {
                program->uses.push_back(std::move(*useDeclaration));
            }
        }

        while (!isAtEnd()) {
            bool isPublic = false;
            if (match(TokenKind::KwPub)) {
                isPublic = true;
            }

            if (!check(TokenKind::KwFn)) {
                reportCurrent("PAR001", "Expected function declaration.");
                sync();
                continue;
            }

            auto func = parseFunctionDeclaration(isPublic);
            if (func.has_value()) {
                program->functions.push_back(std::move(*func));
            }
        }

        if (program->module.has_value()) {
            if (!program->functions.empty()) {
                program->range = makeRangeFromTokens(_tokens.front(), _tokens[_tokens.size() - 2U]);
            } else {
                program->range = program->module->range;
            }
        } else if (!_tokens.empty()) {
            program->range = _tokens.front().range();
        }

        return program;
    }

    auto Parser::isAtEnd() const -> bool {
        return current().is(TokenKind::EndOfFile);
    }

    auto Parser::current() const -> const Token& {
        return _tokens[_position];
    }

    auto Parser::previous() const -> const Token& {
        return _tokens[_position - 1U];
    }

    auto Parser::advance() -> const Token& {
        if (!isAtEnd()) {
            ++_position;
        }

        return previous();
    }

    auto Parser::check(TokenKind kind) const -> bool {
        if (isAtEnd()) {
            return kind == TokenKind::EndOfFile;
        }

        return current().is(kind);
    }

    auto Parser::match(TokenKind kind) -> bool {
        if (!check(kind)) {
            return false;
        }

        [[maybe_unused]] const auto &ignored = advance();

        return true;
    }

    auto Parser::consume(TokenKind kind, const char *code, const char *message) -> const Token* {
        if (check(kind)) {
            return &advance();
        }

        reportCurrent(code, message);

        return nullptr;
    }

    auto Parser::parseModuleDeclaration() -> std::optional<AST::ModuleDeclaration> {
        const Token *moduleKeyword = consume(TokenKind::KwModule, "PAR002", "Expected 'module'.");
        if (moduleKeyword == nullptr) {
            return std::nullopt;
        }

        const auto qualifiedName = parseQualifiedName();
        if (!qualifiedName.has_value()) {
            return std::nullopt;
        }

        const Token *semicolon = consume(TokenKind::Semicolon, "PAR003", "Expected ';' after module declaration.");
        if (semicolon == nullptr) {
            return std::nullopt;
        }

        return AST::ModuleDeclaration {
            .name = std::move(*qualifiedName),
            .range = makeRangeFromTokens(*moduleKeyword, *semicolon),
        };
    }

    auto Parser::parseUseDeclaration() -> std::optional<AST::UseDeclaration> {
        const Token *useKeyword = consume(TokenKind::KwUse, "PAR004", "Expected 'use'.");
        if (useKeyword == nullptr) {
            return std::nullopt;
        }

        const auto qualifiedName = parseQualifiedName();
        if (!qualifiedName.has_value()) {
            return std::nullopt;
        }

        std::optional<std::string> alias;
        if (match(TokenKind::KwAs)) {
            const Token *aliasToken = consume(TokenKind::Identifier, "PAR005", "Expected alias identifier after 'as'.");
            if (aliasToken == nullptr) {
                return std::nullopt;
            }

            alias = std::string(aliasToken->text());
        }

        const Token *semicolon = consume(TokenKind::Semicolon, "PAR006", "Expected ';' after use declaration.");
        if (semicolon == nullptr) {
            return std::nullopt;
        }

        return AST::UseDeclaration {
            .path = std::move(*qualifiedName),
            .alias = std::move(alias),
            .range = makeRangeFromTokens(*useKeyword, *semicolon),
        };
    }

    auto Parser::parseFunctionDeclaration(bool isPublic) -> std::optional<AST::FunctionDeclaration> {
        const Token *fnKeyword = consume(TokenKind::KwFn, "PAR007", "Expected 'fn'.");
        if (fnKeyword == nullptr) {
            return std::nullopt;
        }

        const Token *nameToken = consume(TokenKind::Identifier, "PAR008", "Expected function name.");
        if (nameToken == nullptr) {
            return std::nullopt;
        }

        std::vector<AST::Parameter> params;
        const Token *openParam = consume(TokenKind::OpenParen, "PAR009", "Expected '('.");
        if (openParam == nullptr) {
            return std::nullopt;
        }

        if (!check(TokenKind::CloseParen)) {
            while (true) {
                const Token *paramName = consume(TokenKind::Identifier, "PAR020", "Expected parameter name.");
                if (paramName == nullptr) {
                    return std::nullopt;
                }

                if (consume(TokenKind::Colon, "PAR021", "Expected ':' after parameter name.") == nullptr) {
                    return std::nullopt;
                }

                const auto paramType = parseTypeName();
                if (!paramType.has_value()) {
                    return std::nullopt;
                }

                params.push_back(AST::Parameter {
                    .name = std::string(paramName->text()),
                    .type = *paramType,
                    .range = Source::SourceRange(
                        paramName->range().begin(),
                        paramType->range.end()
                    )
                });

                if (!match(TokenKind::Comma)) {
                    break;
                }
            }
        }

        if (consume(TokenKind::CloseParen, "PAR010", "Expected ')'.") == nullptr) {
            return std::nullopt;
        }

        if (consume(TokenKind::Colon, "PAR011", "Expected ':' before return type.") == nullptr) {
            return std::nullopt;
        }

        const auto returnType = parseTypeName();
        if (!returnType.has_value()) {
            return std::nullopt;
        }

        const Token *openBrace = consume(TokenKind::OpenBrace, "PAR012", "Expected '{' before function body.");
        if (openBrace == nullptr) {
            return std::nullopt;
        }

        std::vector<std::unique_ptr<AST::Statement>> statements;
        while (!check(TokenKind::CloseBrace) && !isAtEnd()) {
            auto statement = parseStatement();
            if (statement == nullptr) {
                sync();
                continue;
            }

            statements.push_back(std::move(statement));
        }

        const Token *closeBrace = consume(TokenKind::CloseBrace, "PAR013", "Expected '}' after function body.");
        if (closeBrace == nullptr) {
            return std::nullopt;
        }

        return AST::FunctionDeclaration {
            .isPublic = isPublic,
            .name = std::string(nameToken->text()),
            .parameters = std::move(params),
            .returnType = std::move(*returnType),
            .statements = std::move(statements),
            .range = makeRangeFromTokens(*fnKeyword, *closeBrace),
        };
    }

    auto Parser::parseTypeName() -> std::optional<AST::TypeName> {
        const auto name = parseQualifiedName();
        if (!name.has_value()) {
            return std::nullopt;
        }

        AST::TypeName typeName;
        typeName.range = name->range;
        typeName.name = std::move(*name);

        return typeName;
    }

    auto Parser::parseQualifiedName() -> std::optional<AST::QualifiedName> {
        const Token *firstSegment = consume(TokenKind::Identifier, "PAR014", "Expected identifier.");
        if (firstSegment == nullptr) {
            return std::nullopt;
        }

        AST::QualifiedName qualifiedName;
        qualifiedName.segments.push_back(std::string(firstSegment->text()));
        const Token *lastSegment = firstSegment;

        while (match(TokenKind::DoubleColon)) {
            const Token *nextSegment = consume(TokenKind::Identifier, "PAR015", "Expected identifier after '::'.");
            if (nextSegment == nullptr) {
                return std::nullopt;
            }

            qualifiedName.segments.push_back(std::string(nextSegment->text()));
            lastSegment = nextSegment;
        }

        qualifiedName.range = makeRangeFromTokens(*firstSegment, *lastSegment);

        return qualifiedName;
    }

    auto Parser::parseStatement() -> std::unique_ptr<AST::Statement> {
        if (match(TokenKind::KwReturn)) {
            const Token &returnKw = previous();
            std::unique_ptr<AST::Expression> expr;

            // Support both forms: `return value;` and `return;`
            if (!check(TokenKind::Semicolon)) {
                expr = parseExpression();
                if (expr == nullptr) {
                    return nullptr;
                }
            }

            const Token *semicolon = consume(TokenKind::Semicolon, "PAR016", "Expected ';' after return statement.");
            if (semicolon == nullptr) {
                return nullptr;
            }

            return std::make_unique<AST::ReturnStatement>(
                std::move(expr),
                makeRangeFromTokens(returnKw, *semicolon)
            );
        }

        auto expression = parseExpression();
        if (expression == nullptr) {
            return nullptr;
        }

        const Source::SourceRange beginRange = expression->range;
        const Token *semicolon = consume(TokenKind::Semicolon, "PAR017", "Expected ';' after expression statement.");
        if (semicolon == nullptr) {
            return nullptr;
        }

        return std::make_unique<AST::ExpressionStatement>(
            std::move(expression),
            Source::SourceRange(beginRange.begin(), semicolon->range().end())
        );
    }

    auto Parser::parseExpression() -> std::unique_ptr<AST::Expression> {
        auto left = parsePrimaryExpression();
        if (left == nullptr) {
            return nullptr;
        }

        while (match(TokenKind::Plus)) {
            auto right = parsePrimaryExpression();
            if (right == nullptr) {
                return nullptr;
            }

            const auto range = Source::SourceRange(
                    left->range.begin(),
                    right->range.end()
                );

            left = std::make_unique<AST::BinaryExpression>(
                std::move(left),
                AST::BinaryOperator::Add,
                std::move(right),
                range
            );
        }

        return left;
    }

    auto Parser::parsePrimaryExpression() -> std::unique_ptr<AST::Expression> {
        if (check(TokenKind::StringLiteral)) {
            const Token &token = advance();
            return std::make_unique<AST::StringLiteralExpression>(std::string(token.text()), token.range());
        }

        if (check(TokenKind::IntegerLiteral)) {
            const Token &token = advance();
            return std::make_unique<AST::IntegerLiteralExpression>(std::string(token.text()), token.range());
        }

        if (check(TokenKind::Identifier)) {
            return parseCallExpressionOrName();
        }

        reportCurrent("PAR018", "Expected expression.");

        return nullptr;
    }

    auto Parser::parseCallExpressionOrName() -> std::unique_ptr<AST::Expression> {
        const auto qualifiedName = parseQualifiedName();
        if (!qualifiedName.has_value()) {
            return nullptr;
        }

        if (!match(TokenKind::OpenParen)) {
            return std::make_unique<AST::NameExpression>(*qualifiedName, qualifiedName->range);
        }

        std::vector<std::unique_ptr<AST::Expression>> arguments;
        if (!check(TokenKind::CloseParen)) {
            while (true) {
                auto argument = parseExpression();
                if (argument == nullptr) {
                    return nullptr;
                }

                arguments.push_back(std::move(argument));
                if (!match(TokenKind::Comma)) {
                    break;
                }
            }
        }

        const Token *closeParen = consume(TokenKind::CloseParen, "PAR019", "Expected ')' after argument list.");
        if (closeParen == nullptr) {
            return nullptr;
        }

        return std::make_unique<AST::CallExpression>(
            std::move(*qualifiedName),
            std::move(arguments),
            Source::SourceRange(qualifiedName->range.begin(), closeParen->range().end())
        );
    }

    auto Parser::makeRangeFromTokens(const Token &begin, const Token &end) const -> Source::SourceRange {
        return {begin.range().begin(), end.range().end()};
    }

    auto Parser::makeFallbackRange() const -> Source::SourceRange {
        return current().range();
    }

    void Parser::sync() {
        while (!isAtEnd()) {
            if (_position > 0U) {
                if (previous().is(TokenKind::Semicolon) || previous().is(TokenKind::CloseBrace)) {
                    return;
                }
            }

            if (check(TokenKind::KwFn) || check(TokenKind::KwUse) || check(TokenKind::KwModule) || check(TokenKind::KwPub)) {
                return;
            }

            [[maybe_unused]] const auto &ignored = advance();
        }
    }

    void Parser::reportCurrent(const char *code, const char *message) {
        _engine.error(code, message, makeFallbackRange());
    }
}
