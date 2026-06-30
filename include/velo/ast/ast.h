#ifndef INC_VELO_AST_AST_H
#define INC_VELO_AST_AST_H

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include <cstddef>

#include "velo/source/source_range.h"

namespace Velo::AST {
    struct QualifiedName final {
        std::vector<std::string> segments {};
        Source::SourceRange range {};
    };

    enum class TypeNameKind {
        Named,
        Map,
    };

    struct TypeName final {
        TypeNameKind kind {TypeNameKind::Named};

        QualifiedName name {};
        std::size_t arrayDepth {0U};

        // Used when kind == TypeNameKind::Map
        std::shared_ptr<TypeName> mapKeyType {};
        std::shared_ptr<TypeName> mapValueType {};

        Source::SourceRange range {};
    };

    enum class ExpressionKind {
        IntegerLiteral,
        StringLiteral,
        BooleanLiteral,
        Name,
        Call,
        StructLiteral,
        ArrayLiteral,
        MapLiteral,
        FieldAccess,
        Index,
        Binary,
        Unary,
    };

    enum class BinaryOperator {
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo,

        Equal,
        NotEqual,
        Less,
        Greater,
        LessEqual,
        GreaterEqual,

        LogicalAnd,
        LogicalOr,
    };

    enum class UnaryOperator {
        Not,
        Negate,
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

    struct StructLiteralField final {
        std::string name;
        std::unique_ptr<Expression> value;
        Source::SourceRange range;
    };

    struct StructLiteralExpression final : Expression {
        StructLiteralExpression(
            TypeName expressionType,
            std::vector<StructLiteralField> expressionFields,
            Source::SourceRange expressionRange
        ) : Expression(
            ExpressionKind::StructLiteral, expressionRange
        ), type(std::move(expressionType)), fields(std::move(expressionFields)) {}

        TypeName type {};
        std::vector<StructLiteralField> fields {};
    };

    struct ArrayLiteralExpression final : Expression {
        ArrayLiteralExpression(
            std::vector<std::unique_ptr<Expression>> literalElements,
            Source::SourceRange literalRange
        ) : Expression(ExpressionKind::ArrayLiteral, literalRange), elements(std::move(literalElements)) {}

        std::vector<std::unique_ptr<Expression>> elements {};
    };

    struct MapLiteralEntry final {
        std::string key {};
        Source::SourceRange keyRange {};
        std::unique_ptr<Expression> value {};
        Source::SourceRange range {};
    };

    struct MapLiteralExpression final : Expression {
        MapLiteralExpression(
            std::vector<MapLiteralEntry> literalEntries,
            Source::SourceRange literalRange
        ) : Expression(ExpressionKind::MapLiteral, literalRange), entries(std::move(literalEntries)) {}

        std::vector<MapLiteralEntry> entries {};
    };

    struct FieldAccessExpression final : Expression {
        FieldAccessExpression(
            std::unique_ptr<Expression> objectExpression,
            std::string expressionFieldName,
            Source::SourceRange expressionFieldRange,
            Source::SourceRange expressionRange
        ) : Expression(ExpressionKind::FieldAccess, expressionRange),
            object(std::move(objectExpression)), fieldName(std::move(expressionFieldName)), fieldRange(expressionFieldRange) {}

        std::unique_ptr<Expression> object;
        std::string fieldName;
        Source::SourceRange fieldRange;
    };

    struct IndexExpression final : Expression {
        IndexExpression(
            std::unique_ptr<Expression> indexedObj,
            std::unique_ptr<Expression> indexExpression,
            Source::SourceRange expressionRange
        ): Expression(ExpressionKind::Index, expressionRange),
            object(std::move(indexedObj)), index(std::move(indexExpression)) {}

        std::unique_ptr<Expression> object;
        std::unique_ptr<Expression> index;
    };

    struct UnaryExpression final : Expression {
        UnaryExpression(
            UnaryOperator expressionOperator,
            std::unique_ptr<Expression> operandExpression,
            Source::SourceRange expressionRange
        ) : Expression(ExpressionKind::Unary, expressionRange),
            op(expressionOperator), operand(std::move(operandExpression)) {}

        UnaryOperator op;
        std::unique_ptr<Expression> operand;
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
        FieldAssignment,
        IndexAssignment,
        If,
        While,
        Break,
        Continue,
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

    struct FieldAssignmentStatement final : Statement {
        FieldAssignmentStatement(
            std::unique_ptr<FieldAccessExpression> assignmentTarget,
            std::unique_ptr<Expression> assignmentValue,
            Source::SourceRange assignmentRange
        ) : Statement(StatementKind::FieldAssignment, assignmentRange),
            target(std::move(assignmentTarget)),
            value(std::move(assignmentValue)) {}

        std::unique_ptr<FieldAccessExpression> target;
        std::unique_ptr<Expression> value;
    };

    struct IndexAssignmentStatement final : Statement {
        IndexAssignmentStatement(
            std::unique_ptr<IndexExpression> assignmentTarget,
            std::unique_ptr<Expression> assignmentValue,
            Source::SourceRange assignmentRange
        ) : Statement(StatementKind::IndexAssignment, assignmentRange),
            target(std::move(assignmentTarget)),
            value(std::move(assignmentValue)) {}

        std::unique_ptr<IndexExpression> target;
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

    struct WhileStatement final : Statement {
        WhileStatement(
            std::unique_ptr<Expression> conditionExpression,
            std::vector<std::unique_ptr<Statement>> bodyStatement,
            Source::SourceRange statementRange
        ) : Statement(StatementKind::While, statementRange),
            condition(std::move(conditionExpression)),
            body(std::move(bodyStatement)) {}

        std::unique_ptr<Expression> condition;
        std::vector<std::unique_ptr<Statement>> body;
    };

    struct BreakStatement final : Statement {
        explicit BreakStatement(Source::SourceRange sourceRange) : Statement(StatementKind::Break, sourceRange) {}
    };

    struct ContinueStatement final : Statement {
        explicit ContinueStatement(Source::SourceRange sourceRange) : Statement(StatementKind::Continue, sourceRange) {}
    };

    enum class AnnotationArgumentKind {
        StringLiteral,
        IntegerLiteral,
        BooleanLiteral,
    };

    struct AnnotationArgument final {
        AnnotationArgumentKind kind {AnnotationArgumentKind::StringLiteral};
        std::string value {};
        Source::SourceRange range {};
    };

    struct Annotation final {
        QualifiedName name {};
        std::vector<AnnotationArgument> arguments {};
        Source::SourceRange range {};
    };

    struct Parameter final {
        std::string name;
        TypeName type {};
        Source::SourceRange range;
    };

    struct StructField final {
        bool isPublic {false};
        std::string name;
        TypeName type {};
        Source::SourceRange range;
    };

    struct StructDeclaration final {
        bool isPublic {false};
        std::string name;
        std::vector<StructField> fields {};
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
        std::vector<Annotation> annotations {};
        std::vector<Parameter> parameters {};
        TypeName returnType {};
        std::vector<std::unique_ptr<Statement>> statements {};
        Source::SourceRange range;
    };

    struct Program final {
        std::optional<ModuleDeclaration> module {};
        std::vector<UseDeclaration> uses {};
        std::vector<StructDeclaration> structs {};
        std::vector<FunctionDeclaration> functions {};
        Source::SourceRange range {};
    };
}

#endif //INC_VELO_AST_AST_H
