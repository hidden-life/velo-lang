#ifndef INC_VELO_AST_AST_H
#define INC_VELO_AST_AST_H

#include <memory>
#include <vector>
#include <string>

#include "velo/source/source_range.h"

namespace Velo::AST {
    struct QualifiedName final {
        std::vector<std::string> segments {};
        Source::SourceRange range {};
    };

    struct TypeName final {
        QualifiedName name {};
        Source::SourceRange range {};
    };

    enum class ExpressionKind {
        IntegerLiteral,
        StringLiteral,
        BooleanLiteral,
        Name,
        Call,
        Binary,
    };

    enum class BinaryOperator {
        Add
    };

    struct Expression {
        explicit Expression(ExpressionKind expressionKind, Source::SourceRange expressionRange) : kind(expressionKind), range(expressionRange) {}

        virtual ~Expression() = default;

        ExpressionKind kind;
        Source::SourceRange range;
    };

    struct IntegerLiteralExpression final : Expression {
        explicit IntegerLiteralExpression(std::string literalValue, Source::SourceRange literalRange) :
            Expression(ExpressionKind::IntegerLiteral, literalRange), value(std::move(literalValue)) {}

        std::string value;
    };

    struct StringLiteralExpression final : Expression {
        explicit StringLiteralExpression(std::string literalValue, Source::SourceRange literalRange) :
            Expression(ExpressionKind::StringLiteral, literalRange), value(std::move(literalValue)) {}

        std::string value;
    };

    struct BooleanLiteralExpression final : Expression {
        explicit BooleanLiteralExpression(bool literalValue, Source::SourceRange literalRange):
            Expression(ExpressionKind::BooleanLiteral, literalRange), value(literalValue) {}

        bool value {false};
    };

    struct NameExpression final : Expression {
        explicit NameExpression(QualifiedName expressionName, Source::SourceRange expressionRange) :
            Expression(ExpressionKind::Name, expressionRange), name(std::move(expressionName)) {}

        QualifiedName name;
    };

    struct CallExpression final : Expression {
        CallExpression(
            QualifiedName expressionCallee,
            std::vector<std::unique_ptr<Expression>> expressionArgs,
            Source::SourceRange expressionRange) :
        Expression(ExpressionKind::Call, expressionRange), callee(std::move(expressionCallee)), arguments(std::move(expressionArgs)) {}

        QualifiedName callee;
        std::vector<std::unique_ptr<Expression>> arguments {};
    };

    struct BinaryExpression final : Expression {
        BinaryExpression(
            std::unique_ptr<Expression> expressionLeft,
            BinaryOperator expressionOperator,
            std::unique_ptr<Expression> expressionRight,
            Source::SourceRange expressionRange
        ) : Expression(ExpressionKind::Binary, expressionRange),
            left(std::move(expressionLeft)),
            op(expressionOperator),
            right(std::move(expressionRight)) {}

        std::unique_ptr<Expression> left;
        BinaryOperator op;
        std::unique_ptr<Expression> right;
    };

    enum class StatementKind {
        Expression,
        Return,
        VariableDeclaration,
        Assignment,
        If,
    };

    struct Statement {
        explicit Statement(StatementKind statementKind, Source::SourceRange statementRange) : kind(statementKind), range(statementRange) {}
        virtual ~Statement() = default;

        StatementKind kind;
        Source::SourceRange range;
    };

    struct ExpressionStatement final : Statement {
        explicit ExpressionStatement(std::unique_ptr<Expression> statementExpression, Source::SourceRange statementRange) :
            Statement(StatementKind::Expression, statementRange), expression(std::move(statementExpression)) {}

        std::unique_ptr<Expression> expression;
    };

    struct ReturnStatement final : Statement {
        ReturnStatement(std::unique_ptr<Expression> statementExpression, Source::SourceRange statementRange) :
            Statement(StatementKind::Return, statementRange), expression(std::move(statementExpression)) {}
        // nullptr means `return;` without a value
        std::unique_ptr<Expression> expression;
    };

    struct VariableDeclarationStatement final : Statement {
        VariableDeclarationStatement(
            bool variableIsMutable,
            std::string variableName,
            TypeName variableType,
            std::unique_ptr<Expression> initExpression,
            Source::SourceRange statementRange) : Statement(StatementKind::VariableDeclaration, statementRange),
            isMutable(variableIsMutable), name(std::move(variableName)), type(std::move(variableType)), initializer(std::move(initExpression))
        {}

        bool isMutable {false};
        std::string name;
        TypeName type;
        std::unique_ptr<Expression> initializer;
    };

    struct AssignmentStatement final : Statement {
        AssignmentStatement(
            std::string variableName,
            std::unique_ptr<Expression> valueExpression,
            Source::SourceRange statementRange
        ): Statement(StatementKind::Assignment, statementRange),
            name(std::move(variableName)), value(std::move(valueExpression))
        {}

        std::string name;
        std::unique_ptr<Expression> value;
    };

    struct IfStatement final : Statement {
        IfStatement(std::unique_ptr<Expression> conditionExpression,
            std::vector<std::unique_ptr<Statement>> thenStatements,
            std::vector<std::unique_ptr<Statement>> elseStatements,
            Source::SourceRange statementRange
            ) :
            Statement(StatementKind::If, statementRange),
            condition(std::move(conditionExpression)),
            thenBranch(std::move(thenStatements)), elseBranch(std::move(elseStatements))
        {}

        std::unique_ptr<Expression> condition;
        std::vector<std::unique_ptr<Statement>> thenBranch {};
        std::vector<std::unique_ptr<Statement>> elseBranch {};
    };

    struct Parameter final {
        std::string name;
        TypeName type {};
        Source::SourceRange range;
    };

    struct ModuleDeclaration final {
        QualifiedName name;
        Source::SourceRange range;
    };

    struct UseDeclaration final {
        QualifiedName path {};
        std::optional<std::string> alias {};
        Source::SourceRange range;
    };

    struct FunctionDeclaration final {
        bool isPublic {false};
        std::string name;
        std::vector<Parameter> parameters {};
        TypeName returnType {};
        std::vector<std::unique_ptr<Statement>> statements {};
        Source::SourceRange range;
    };

    struct Program final {
        std::optional<ModuleDeclaration> module {};
        std::vector<UseDeclaration> uses {};
        std::vector<FunctionDeclaration> functions {};
        Source::SourceRange range {};
    };
}

#endif //INC_VELO_AST_AST_H
