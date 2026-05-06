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

        if (stmt.kind == StatementKind::While) {
            const auto &whileStmt = static_cast<const WhileStatement&>(stmt);
            LoopContext loopCtx;
            loopCtx.conditionIndex = func.instructions.size();
            _loopStack.push_back(loopCtx);
            lowerExpression(*whileStmt.condition, func);

            const size_t jumpIfFalseIdx = func.instructions.size();
            func.instructions.push_back(Instruction {
                .code = OpCode::JumpIfFalse,
            });

            for (const auto &nested : whileStmt.body) {
                lowerStatement(*nested, func);
            }

            func.instructions.push_back(Instruction {
                .code = OpCode::Jump,
                .targetOperand = loopCtx.conditionIndex,
            });

            const size_t endIdx = func.instructions.size();
            func.instructions[jumpIfFalseIdx].targetOperand = endIdx;

            // patch break jumps
            for (auto idx : _loopStack.back().breakJumps) {
                func.instructions[idx].targetOperand = endIdx;
            }

            _loopStack.pop_back();

            return;
        }

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

        if (stmt.kind == StatementKind::VariableDeclaration) {
            const auto &varDecl = static_cast<const VariableDeclarationStatement&>(stmt);
            lowerExpression(*varDecl.initializer, func);

            const std::size_t localIdx = _locals.size();
            _locals.emplace(varDecl.name, localIdx);

            func.instructions.push_back(Instruction {
                .code = OpCode::StoreLocal,
                .indexOperand = localIdx,
            });
        }

        if (stmt.kind == StatementKind::Assignment) {
            const auto &assign = static_cast<const AssignmentStatement&>(stmt);
            lowerExpression(*assign.value, func);
            const auto *localIdx = findLocalIndex(assign.name);
            if (localIdx != nullptr) {
                func.instructions.push_back(Instruction {
                    .code = OpCode::StoreLocal,
                    .indexOperand = *localIdx,
                });
            }

            return;
        }

        if (stmt.kind == StatementKind::If) {
            const auto &ifStmt = static_cast<const IfStatement&>(stmt);
            lowerExpression(*ifStmt.condition, func);

            const std::size_t jumpIfFalseIdx = func.instructions.size();
            func.instructions.push_back(Instruction {
                .code = OpCode::JumpIfFalse
            });

            for (const auto &nested : ifStmt.thenBranch) {
                lowerStatement(*nested, func);
            }

            const std::size_t jumpOverElseIdx = func.instructions.size();
            func.instructions.push_back(Instruction {
                .code = OpCode::Jump
            });

            const std::size_t elseStartIdx = func.instructions.size();
            for (const auto &nested : ifStmt.elseBranch) {
                lowerStatement(*nested, func);
            }

            const std::size_t endIdx = func.instructions.size();
            // If condition is false, jump to the else branch.
            func.instructions[jumpIfFalseIdx].targetOperand = elseStartIdx;
            // After then branch, skip the else branch.
            func.instructions[jumpOverElseIdx].targetOperand = endIdx;

            return;
        }

        if (stmt.kind == StatementKind::Break) {
            if (_loopStack.empty()) return;
            const size_t jumpIdx = func.instructions.size();
            func.instructions.push_back(Instruction {
                .code = OpCode::Jump,
            });

            _loopStack.back().breakJumps.push_back(jumpIdx);

            return;
        }

        if (stmt.kind == StatementKind::Continue) {
            if (_loopStack.empty()) return;

            func.instructions.push_back(Instruction {
                .code = OpCode::Jump,
                .targetOperand = _loopStack.back().conditionIndex,
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
                const auto &binaryExpr = static_cast<const BinaryExpression&>(expr);

                lowerExpression(*binaryExpr.left, func);
                lowerExpression(*binaryExpr.right, func);

                switch (binaryExpr.op) {
                    case BinaryOperator::Add:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::AddInt,
                        });
                        return;
                    case BinaryOperator::Equal:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::CompareEqualInt,
                        });
                        return;
                    case BinaryOperator::NotEqual:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::CompareNotEqualInt,
                        });
                        return;
                    case BinaryOperator::Less:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::CompareLessInt,
                        });
                        return;
                    case BinaryOperator::Greater:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::CompareGreaterInt,
                        });
                        return;
                    case BinaryOperator::LessEqual:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::CompareLessEqualInt,
                        });
                        return;
                    case BinaryOperator::GreaterEqual:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::CompareGreaterEqualInt,
                        });
                        return;
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

            case ExpressionKind::BooleanLiteral: {
                const auto &literal = static_cast<const BooleanLiteralExpression&>(expr);
                func.instructions.push_back(Instruction {
                    .code = OpCode::PushBool,
                    .boolOperand = literal.value
                });
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
