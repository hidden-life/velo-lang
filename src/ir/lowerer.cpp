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

        _locals.clear();

        for (std::size_t idx = 0; idx < func.parameters.size(); ++idx) {
            const auto &param = func.parameters[idx];
            f.parameters.push_back(param.name);
            _locals.emplace(param.name, idx);
        }

        for (const auto &stmt : func.statements) {
            lowerStatement(*stmt, f);
        }

        _locals.clear();

        return f;
    }

    void Lowerer::lowerStatement(const AST::Statement &stmt, Function &func) {
        using namespace AST;

        if (stmt.kind == StatementKind::Return) {
            const auto &r = static_cast<const ReturnStatement&>(stmt);
            if (r.expression != nullptr) {
                lowerExpression(*r.expression, func);
            }
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

                func.instructions.push_back(Instruction{
                    .code = OpCode::PushInt,
                    .intOperand = std::stoi(literal.value),
                });
                return;
            }

            case ExpressionKind::StringLiteral: {
                const auto &literal = static_cast<const StringLiteralExpression&>(expr);

                func.instructions.push_back(Instruction {
                    .code = OpCode::PushString,
                    .stringOperand = literal.value
                });
                return;
            }

            case ExpressionKind::Name: {
                const auto &nameExp = static_cast<const NameExpression&>(expr);
                if (nameExp.name.segments.size() == 1U) {
                    const std::string &name = nameExp.name.segments.front();
                    const auto &localIdx = findLocalIndex(name);

                    if (localIdx != nullptr) {
                        func.instructions.push_back(Instruction {
                            .code = OpCode::LoadLocal,
                            .indexOperand = *localIdx,
                        });
                    }
                }
                return;
            }

            case ExpressionKind::Binary: {
                const auto &binaryExp = static_cast<const BinaryExpression&>(expr);

                lowerExpression(*binaryExp.left, func);
                lowerExpression(*binaryExp.right, func);

                if (binaryExp.op == BinaryOperator::Add) {
                    func.instructions.push_back(Instruction {
                        .code = OpCode::AddInt,
                    });
                }

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
                for (std::size_t idx = 0; idx < call.callee.segments.size(); ++idx) {
                    if (idx > 0U) name += "::";
                    name += call.callee.segments[idx];
                }

                i.stringOperand = name;

                func.instructions.push_back(i);
                return;
            }
        }
    }

    auto Lowerer::findLocalIndex(const std::string &name) const -> const std::size_t* {
        const auto it = _locals.find(name);
        if (it == _locals.end()) {
            return nullptr;
        }

        return &it->second;
    }
}
