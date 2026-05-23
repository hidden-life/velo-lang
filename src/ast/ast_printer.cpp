#include "velo/ast/ast_printer.h"

#include <sstream>

namespace Velo::AST {
    namespace {
        void writeIndent(std::ostringstream &stream, std::size_t indentLevel) {
            for (std::size_t idx = 0; idx < indentLevel; ++idx) {
                stream << "  ";
            }
        }

        auto joinQualifiedName(const QualifiedName &name) -> std::string {
            std::ostringstream stream;

            for (std::size_t idx = 0; idx < name.segments.size(); ++idx) {
                if (idx > 0) {
                    stream << "::";
                }

                stream << name.segments[idx];
            }

            return stream.str();
        }

        auto joinTypeName(const TypeName &typeName) -> std::string {
            return joinQualifiedName(typeName.name);
        }

        void printExpression(std::ostringstream &stream, const Expression &expression, std::size_t indentLevel);

        void printStatement(std::ostringstream &stream, const Statement &statement, std::size_t indentLevel) {
            switch (statement.kind) {
                case StatementKind::Expression: {
                    const auto &expressionStatement = static_cast<const ExpressionStatement&>(statement);
                    writeIndent(stream, indentLevel);
                    stream << "ExprStmt\n";
                    printExpression(stream, *expressionStatement.expression, indentLevel + 1U);
                    break;
                }

                case StatementKind::Return: {
                    const auto &returnStatement = static_cast<const ReturnStatement&>(statement);
                    writeIndent(stream, indentLevel);
                    stream << "Return\n";
                    if (returnStatement.expression != nullptr) {
                        printExpression(stream, *returnStatement.expression, indentLevel + 1U);
                    }
                    break;
                }

                case StatementKind::VariableDeclaration: {
                    const auto &varDecl = static_cast<const VariableDeclarationStatement&>(statement);
                    writeIndent(stream, indentLevel);
                    stream << (varDecl.isMutable ? "Var " : "Let ") << varDecl.name << " : ";

                    for (std::size_t idx = 0; idx < varDecl.type.name.segments.size(); ++idx) {
                        if (idx > 0U) {
                            stream << "::";
                        }

                        stream << varDecl.type.name.segments[idx];
                    }

                    stream << "\n";

                    printExpression(stream, *varDecl.initializer, indentLevel + 1U);
                    break;
                }

                case StatementKind::Assignment: {
                    const auto &assignment = static_cast<const AssignmentStatement&>(statement);
                    writeIndent(stream, indentLevel);
                    stream << "Assign " << assignment.name << "\n";

                    printExpression(stream, *assignment.value, indentLevel + 1U);
                    break;
                }

                case StatementKind::FieldAssignment: {
                    const auto &fieldAssignment = static_cast<const FieldAssignmentStatement&>(statement);
                    writeIndent(stream, indentLevel);
                    stream << "FieldAssignment\n";

                    writeIndent(stream, indentLevel + 1U);
                    stream << "Target\n";
                    printExpression(stream, *fieldAssignment.target, indentLevel + 2U);

                    writeIndent(stream, indentLevel + 1U);
                    stream << "Value\n";
                    printExpression(stream, *fieldAssignment.value, indentLevel + 2U);
                    break;
                }

                case StatementKind::If: {
                    const auto &ifStmt = static_cast<const IfStatement&>(statement);
                    writeIndent(stream, indentLevel);
                    stream << "If\n";
                    writeIndent(stream, indentLevel + 1U);
                    stream << "Condition\n";
                    printExpression(stream, *ifStmt.condition, indentLevel + 2U);
                    writeIndent(stream, indentLevel + 1U);
                    stream << "Then\n";

                    for (const auto &nested : ifStmt.thenBranch) {
                        printStatement(stream, *nested, indentLevel + 2U);
                    }

                    if (!ifStmt.elseBranch.empty()) {
                        writeIndent(stream, indentLevel + 1U);
                        stream << "Else\n";
                        for (const auto &nested : ifStmt.elseBranch) {
                            printStatement(stream, *nested, indentLevel + 2U);
                        }
                    }

                    break;
                }

                case StatementKind::While: {
                    const auto &whileStmt = static_cast<const WhileStatement&>(statement);
                    writeIndent(stream, indentLevel);
                    stream << "While\n";
                    writeIndent(stream, indentLevel + 1U);
                    stream << "Condition\n";
                    printExpression(stream, *whileStmt.condition, indentLevel + 2U);
                    writeIndent(stream, indentLevel + 1U);
                    stream << "Body\n";

                    for (const auto &nested : whileStmt.body) {
                        printStatement(stream, *nested, indentLevel + 2U);
                    }

                    break;
                }

                case StatementKind::Break: {
                    writeIndent(stream, indentLevel);
                    stream << "Break\n";
                    break;
                }

                case StatementKind::Continue: {
                    writeIndent(stream, indentLevel);
                    stream << "Continue\n";
                    break;
                }
            }
        }

        void printExpression(std::ostringstream &stream, const Expression &expression, std::size_t indentLevel) {
            switch (expression.kind) {
                case ExpressionKind::IntegerLiteral: {
                    const auto &integerLiteral = static_cast<const IntegerLiteralExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "Integer " << integerLiteral.value << '\n';
                    break;
                }

                case ExpressionKind::StringLiteral: {
                    const auto &stringLiteral = static_cast<const StringLiteralExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "String \"" << stringLiteral.value << "\"\n";
                    break;
                }

                case ExpressionKind::BooleanLiteral: {
                    const auto &booleanLiteral = static_cast<const BooleanLiteralExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "Bool " << (booleanLiteral.value ? "true" : "false") << "\n";
                    break;
                }

                case ExpressionKind::Name: {
                    const auto &name = static_cast<const NameExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "Name " << joinQualifiedName(name.name) << "\n";
                    break;
                }

                case ExpressionKind::Call: {
                    const auto &callExpression = static_cast<const CallExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "Call " << joinQualifiedName(callExpression.callee) << "\n";

                    for (const auto &arg : callExpression.arguments) {
                        printExpression(stream, *arg, indentLevel + 1U);
                    }

                    break;
                }

                case ExpressionKind::StructLiteral: {
                    const auto &structLiteral = static_cast<const StructLiteralExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "StructLiteral " << joinTypeName(structLiteral.type) << "\n";

                    for (const auto &field : structLiteral.fields) {
                        writeIndent(stream, indentLevel + 1U);
                        stream << "Field " << field.name << "\n";
                        printExpression(stream, *field.value, indentLevel + 2U);
                    }

                    break;
                }

                case ExpressionKind::FieldAccess: {
                    const auto &fieldAccess = static_cast<const FieldAccessExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "FieldAccess " << fieldAccess.fieldName << "\n";
                    printExpression(stream, *fieldAccess.object, indentLevel + 1U);

                    break;
                }

                case ExpressionKind::Binary: {
                    const auto &binaryExpression = static_cast<const BinaryExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "Binary: ";

                    switch (binaryExpression.op) {
                        case BinaryOperator::Add:
                            stream << "+";
                            break;
                        case BinaryOperator::Equal:
                            stream << "==";
                            break;
                        case BinaryOperator::NotEqual:
                            stream << "!=";
                            break;
                        case BinaryOperator::Less:
                            stream << "<";
                            break;
                        case BinaryOperator::LessEqual:
                            stream << "<=";
                            break;
                        case BinaryOperator::Greater:
                            stream << ">";
                            break;
                        case BinaryOperator::GreaterEqual:
                            stream << ">=";
                            break;
                        case BinaryOperator::LogicalAnd:
                            stream << "&&";
                            break;
                        case BinaryOperator::LogicalOr:
                            stream << "||";
                            break;
                        case BinaryOperator::Subtract:
                            stream << "-";
                            break;
                        case BinaryOperator::Multiply:
                            stream << "*";
                            break;
                        case BinaryOperator::Divide:
                            stream << "/";
                            break;
                        case BinaryOperator::Modulo:
                            stream << "%";
                            break;
                    }

                    stream << "\n";

                    printExpression(stream, *binaryExpression.left, indentLevel + 1U);
                    printExpression(stream, *binaryExpression.right, indentLevel + 1U);

                    break;
                }

                case ExpressionKind::Unary: {
                    const auto &unaryExpression = static_cast<const UnaryExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "Unary ";
                    switch (unaryExpression.op) {
                        case UnaryOperator::Not:
                            stream << "!";
                            break;
                        case UnaryOperator::Negate:
                            stream << "-";
                            break;
                    }

                    stream << "\n";

                    printExpression(stream, *unaryExpression.operand, indentLevel + 1U);
                    break;
                }
            }
        }
    }

    auto ASTPrinter::print(const Program &program) const -> std::string {
        std::ostringstream stream;

        stream << "Program\n";

        if (program.module.has_value()) {
            writeIndent(stream, 1U);
            stream << "Module " << joinQualifiedName(program.module->name) << "\n";
        }

        for (const auto &useDeclaration : program.uses) {
            writeIndent(stream, 1U);
            stream << "Use " << joinQualifiedName(useDeclaration.path);

            if (useDeclaration.alias.has_value()) {
                stream << " as " << useDeclaration.alias.value();
            }

            stream << "\n";
        }

        for (const auto &structDecl : program.structs) {
            writeIndent(stream, 1U);
            stream << "Struct ";

            if (structDecl.isPublic) {
                stream << "pub ";
            }

            stream << structDecl.name << "\n";

            for (const auto &field : structDecl.fields) {
                writeIndent(stream, 2U);
                stream << "Field ";
                if (field.isPublic) {
                    stream << "pub ";
                }

                stream << field.name << " : " << joinTypeName(field.type) << "\n";
            }
        }

        for (const auto &func : program.functions) {
            writeIndent(stream, 1U);
            stream << "Function ";

            if (func.isPublic) {
                stream << "pub ";
            }

            stream << func.name << " -> " << joinQualifiedName(func.returnType.name) << "\n";

            for (const auto &stmt : func.statements) {
                printStatement(stream, *stmt, 2U);
            }
        }

        return stream.str();
    }
}
