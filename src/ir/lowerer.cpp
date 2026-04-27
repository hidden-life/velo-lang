#include "velo/ir/lowerer.h"

namespace Velo::IR {
    auto Lowerer::lower(const AST::Program &program) -> Module {
        Module module;

        for (const auto &fn : program.functions) {
            module.functions.push_back(lowerFunction(fn));
        }

        return module;
    }

    Function Lowerer::lowerFunction(const AST::FunctionDeclaration &func) {
        Function f;
        f.name = func.name;

        for (const auto &param : func.parameters) {
            f.parameters.push_back(param.name);
        }

        for (const auto &stmt : func.statements) {
            lowerStatement(*stmt, f);
        }

        return f;
    }

    void Lowerer::lowerStatement(const AST::Statement &stmt, Function &func) {
        using namespace AST;

        if (stmt.kind == StatementKind::Return) {
            const auto &r = static_cast<const ReturnStatement&>(stmt);
            lowerExpression(*r.expression, func);
            func.instructions.push_back({OpCode::Return});
            return;
        }

        if (stmt.kind == StatementKind::Expression) {
            const auto &expr = static_cast<const ExpressionStatement&>(stmt);
            lowerExpression(*expr.expression, func);
            // Expression statement result is unused -> remove it from the stack.
            func.instructions.push_back({
                .code = OpCode::Pop,
            });

            return;
        }
    }

    void Lowerer::lowerExpression(const AST::Expression &expr, Function &func) {
        using namespace AST;

        switch (expr.kind) {
            case ExpressionKind::IntegerLiteral: {
                const auto &literal = static_cast<const IntegerLiteralExpression&>(expr);
                Instruction i;
                i.code = OpCode::PushInt;
                i.intOperand = std::stoi(literal.value);

                func.instructions.push_back(i);
                return;
            }

            case ExpressionKind::StringLiteral: {
                const auto &literal = static_cast<const StringLiteralExpression&>(expr);
                Instruction i;
                i.code = OpCode::PushString;
                i.stringOperand = literal.value;

                func.instructions.push_back(i);
                return;
            }

            case ExpressionKind::Call: {
                const auto &call = static_cast<const CallExpression&>(expr);
                for (const auto &arg : call.arguments) {
                    lowerExpression(*arg, func);
                }

                Instruction i;
                // Qualified calls like console::println() are treated as builtin calls.
                // Single-segment calls like message() are treated as user-defined function calls.
                i.code = call.callee.segments.size() > 1U ? OpCode::CallBuiltin : OpCode::CallFunction;
                i.argsCount = call.arguments.size();

                // concat names currently
                std::string name;
                for (size_t it = 0; it < call.callee.segments.size(); ++it) {
                    if (it > 0U) name += "::";
                    name += call.callee.segments[it];
                }

                i.stringOperand = name;

                func.instructions.push_back(i);
                return;
            }

            default:
                return;
        }
    }
}
