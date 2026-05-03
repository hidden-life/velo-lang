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

                case ExpressionKind::Binary: {
                    const auto &binaryExpression = static_cast<const BinaryExpression&>(expression);
                    writeIndent(stream, indentLevel);
                    stream << "Binary: +\n";

                    printExpression(stream, *binaryExpression.left, indentLevel + 1U);
                    printExpression(stream, *binaryExpression.right, indentLevel + 1U);
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
