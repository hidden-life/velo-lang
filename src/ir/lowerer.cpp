#include "velo/ir/lowerer.h"

namespace {
    struct FieldAssignmentPath final {
        std::string rootLocalName {};
        std::vector<std::string> fields {};
    };

    void collectFieldAssignmentPath(const Velo::AST::Expression &expr, FieldAssignmentPath &path) {
        using namespace Velo::AST;

        if (expr.kind == ExpressionKind::Name) {
            const auto &nameExpr = static_cast<const NameExpression&>(expr);
            if (nameExpr.name.segments.size() == 1U) {
                path.rootLocalName = nameExpr.name.segments.front();
            }

            return;
        }

        if (expr.kind == ExpressionKind::FieldAccess) {
            const auto &fieldAccess = static_cast<const FieldAccessExpression&>(expr);
            collectFieldAssignmentPath(*fieldAccess.object, path);
            path.fields.push_back(fieldAccess.fieldName);
            return;
        }
    }

    [[nodiscard]] auto encodeFieldPath(const std::vector<std::string> &fields) -> std::string {
        std::string encoded;

        for (std::size_t idx = 0; idx < fields.size(); ++idx) {
            if (idx > 0U) {
                encoded += ".";
            }

            encoded += fields[idx];
        }

        return encoded;
    }
}

namespace Velo::IR {
    auto Lowerer::lower(const AST::Program &program) -> Module {
        collectModuleAliases(program);
        Module module;

        for (const auto &fn : program.functions) {
            module.functions.push_back(lowerFunction(fn));
        }

        _moduleAliases.clear();

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

        if (stmt.kind == StatementKind::FieldAssignment) {
            const auto &fieldAssignment = static_cast<const FieldAssignmentStatement&>(stmt);
            lowerFieldAssignmentStatement(fieldAssignment, func);
            return;
        }
    }

    void Lowerer::lowerFieldAssignmentStatement(const AST::FieldAssignmentStatement &stmt, Function &func) {
        FieldAssignmentPath path;
        collectFieldAssignmentPath(*stmt.target, path);

        if (path.rootLocalName.empty() || path.fields.empty()) {
            return;
        }

        const auto *localIdx = findLocalIndex(path.rootLocalName);
        if (localIdx == nullptr) {
            return;
        }

        // Evaluate RHS first. This preserves expected behavior for:
        //
        // user.id = user.id + 1
        //
        // RHS sees the old value, then root local is loaded and updated.
        lowerExpression(*stmt.value, func);

        func.instructions.push_back(Instruction {
            .code = OpCode::LoadLocal,
            .indexOperand = *localIdx,
        });

        func.instructions.push_back(Instruction {
            .code = OpCode::StoreFieldPath,
            .stringOperand = encodeFieldPath(path.fields),
        });

        func.instructions.push_back(Instruction {
            .code = OpCode::StoreLocal,
            .indexOperand = *localIdx,
        });
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

            case ExpressionKind::StructLiteral: {
                const auto &structLiteral = static_cast<const StructLiteralExpression&>(expr);
                lowerStructLiteralExpression(structLiteral, func);
                return;
            }

            case ExpressionKind::FieldAccess: {
                const auto &fieldAccess = static_cast<const FieldAccessExpression&>(expr);
                lowerFieldAccessExpression(fieldAccess, func);
                return;
            }

            case ExpressionKind::Binary: {
                const auto &binaryExpr = static_cast<const BinaryExpression&>(expr);
                if (binaryExpr.op == BinaryOperator::LogicalAnd) {
                    lowerLogicalAndExpression(binaryExpr, func);
                    return;
                }

                if (binaryExpr.op == BinaryOperator::LogicalOr) {
                    lowerLogicalOrExpression(binaryExpr, func);
                    return;
                }

                lowerExpression(*binaryExpr.left, func);
                lowerExpression(*binaryExpr.right, func);

                switch (binaryExpr.op) {
                    case BinaryOperator::Add:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::AddInt,
                        });
                        return;
                    case BinaryOperator::Subtract:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::SubInt,
                        });
                        return;
                    case BinaryOperator::Multiply:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::MulInt,
                        });
                        return;
                    case BinaryOperator::Divide:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::DivInt,
                        });
                        return;
                    case BinaryOperator::Modulo:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::ModInt,
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
                    case BinaryOperator::LogicalAnd:
                    case BinaryOperator::LogicalOr:
                        // Already handled above with short-circuit lowering.
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
                // Qualified calls like console::println() or str::len()
                // are treated as builtin calls.
                //
                // Single-segment calls like helper() are treated as user-defined function calls.
                i.code = call.callee.segments.size() > 1U ?
                    OpCode::CallBuiltin :
                    OpCode::CallFunction;
                i.argsCount = call.arguments.size();
                i.stringOperand = lowerQualifiedName(call.callee);

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

            case ExpressionKind::Unary: {
                const auto &unary = static_cast<const UnaryExpression&>(expr);
                lowerExpression(*unary.operand, func);

                switch (unary.op) {
                    case UnaryOperator::Not:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::LogicalNot,
                        });
                        return;
                    case UnaryOperator::Negate:
                        func.instructions.push_back(Instruction {
                            .code = OpCode::NegInt,
                        });
                        return;
                }
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

    void Lowerer::lowerLogicalAndExpression(const AST::BinaryExpression &expr, Function &func) {
        // Short-circuit AND:
        //
        // left
        // JumpIfFalse false_branch
        // right
        // Jump end
        // false_branch
        // PushBool false
        // end:

        lowerExpression(*expr.left, func);
        const std::size_t jumpIfFalseIdx = func.instructions.size();
        func.instructions.push_back(Instruction {
            .code = OpCode::JumpIfFalse,
        });

        lowerExpression(*expr.right, func);
        const std::size_t jumpToEndIdx = func.instructions.size();
        func.instructions.push_back(Instruction {
            .code = OpCode::Jump,
        });

        const std::size_t falseBranchIdx = func.instructions.size();
        func.instructions.push_back(Instruction {
            .code = OpCode::PushBool,
            .boolOperand = false,
        });

        const std::size_t endIdx = func.instructions.size();
        // If left is false, skip right operand and push false.
        func.instructions[jumpIfFalseIdx].targetOperand = falseBranchIdx;
        // If left is true, right operand has produced final bool result.
        func.instructions[jumpToEndIdx].targetOperand = endIdx;
    }

    void Lowerer::lowerLogicalOrExpression(const AST::BinaryExpression &expr, Function &func) {
        // Short-circuit OR:
        //
        // left
        // JumpIfFalse right_branch
        // PushBool true
        // Jump end
        // right_branch
        // right
        // end:
        lowerExpression(*expr.left, func);
        const std::size_t jumpToRightIdx = func.instructions.size();
        func.instructions.push_back(Instruction {
            .code = OpCode::JumpIfFalse,
        });
        func.instructions.push_back(Instruction {
            .code = OpCode::PushBool,
            .boolOperand = true,
        });

        const std::size_t jumpToEndIdx = func.instructions.size();
        func.instructions.push_back(Instruction {
            .code = OpCode::Jump,
        });

        const std::size_t rightBranchIdx = func.instructions.size();
        lowerExpression(*expr.right, func);
        const std::size_t endIdx = func.instructions.size();
        // If left is false, evaluate right operand.
        func.instructions[jumpToRightIdx].targetOperand = rightBranchIdx;
        // If left is true, PushBool true is the final result.
        func.instructions[jumpToEndIdx].targetOperand = endIdx;
    }

    void Lowerer::collectModuleAliases(const AST::Program &program) {
        _moduleAliases.clear();

        for (const auto &useDecl : program.uses) {
            if (useDecl.path.segments.empty()) {
                continue;
            }

            const std::string actualModuleName = useDecl.path.segments.back();

            std::string visibleModuleName = actualModuleName;
            if (useDecl.alias.has_value()) {
                visibleModuleName = useDecl.alias.value();
            }

            _moduleAliases.insert_or_assign(visibleModuleName, actualModuleName);
        }
    }

    auto Lowerer::lowerQualifiedName(const AST::QualifiedName &name) const -> std::string {
        if (name.segments.empty()) {
            return {};
        }

        if (name.segments.size() == 1U) {
            return name.segments.front();
        }

        std::string result;

        const auto aliasIt = _moduleAliases.find(name.segments.front());
        if (aliasIt != _moduleAliases.end()) {
            result = aliasIt->second;
        } else {
            result = name.segments.front();
        }

        for (std::size_t idx = 1U; idx < name.segments.size(); ++idx) {
            result += "::";
            result += name.segments[idx];
        }

        return result;
    }

    auto Lowerer::lowerTypeName(const AST::TypeName &typeName) const -> std::string {
        std::string result;
        for (std::size_t idx = 0U; idx < typeName.name.segments.size(); ++idx) {
            if (idx > 0U) {
                result += "::";
            }

            result += typeName.name.segments[idx];
        }

        return result;
    }

    void Lowerer::lowerStructLiteralExpression(const AST::StructLiteralExpression &expr, Function &func) {
        std::string encodedOperand = lowerTypeName(expr.type);
        encodedOperand += ":";

        for (std::size_t idx = 0U; idx < expr.fields.size(); ++idx) {
            const auto &field = expr.fields[idx];
            if (idx > 0U) {
                encodedOperand += ",";
            }

            encodedOperand += field.name;

            lowerExpression(*field.value, func);
        }

        func.instructions.push_back(Instruction {
            .code = OpCode::BuildStruct,
            .stringOperand = encodedOperand,
            .argsCount = expr.fields.size()
        });
    }

    void Lowerer::lowerFieldAccessExpression(const AST::FieldAccessExpression &expr, Function &func) {
        lowerExpression(*expr.object, func);
        func.instructions.push_back(Instruction {
            .code = OpCode::LoadField,
            .stringOperand = expr.fieldName,
        });
    }
}
