#include "velo/semantic/semantic_analyzer.h"

namespace Velo::Semantic {
    SematicAnalyzer::SematicAnalyzer(const AST::Program &program, Diagnostic::DiagnosticEngine &engine) : _program(program), _engine(engine) {
    }

    auto SematicAnalyzer::analyze() -> bool {
        collectImports();
        collectFunctions();
        validateEntryPoint();

        for (const auto &func : _program.functions) {
            analyzeFunction(func);
        }

        return !_engine.hasErrors();
    }

    void SematicAnalyzer::collectImports() {
        for (const auto &useDecl : _program.uses) {
            const std::string visibleName = visibleImportName(useDecl);
            const auto [it, inserted] = _visibleImports.emplace(visibleName, &useDecl);
            if (!inserted) {
                _engine.error(
                    "SEM001",
                    "Duplicate visible import name '" + visibleName + "'.",
                    useDecl.range
                );
            }
        }
    }

    void SematicAnalyzer::collectFunctions() {
        for (const auto &func : _program.functions) {
            const auto [it, inserted] = _functions.emplace(func.name, &func);
            if (!inserted) {
                _engine.error(
                    "SEM002",
                    "Duplicate function declaration '" + func.name + "'.",
                    func.range
                );
            }
        }
    }

    void SematicAnalyzer::validateEntryPoint() {
        const auto it = _functions.find("main");
        if (it == _functions.end()) {
            _engine.error(
                "SEM003",
                "Missing entry point 'main'.",
                _program.range
            );
            return;
        }

        const auto &mainFunc = *it->second;
        if (!mainFunc.parameters.empty()) {
            _engine.error(
                "SEM004",
                "Entry point 'main' must not have parameters.",
                mainFunc.range
            );
        }

        if (!isBuiltinInt(mainFunc.returnType)) {
            _engine.error(
                "SEM005",
                "Entry point 'main' must return int.",
                mainFunc.returnType.range
            );
        }
    }

    void SematicAnalyzer::analyzeFunction(const AST::FunctionDeclaration &func) {
        for (const auto &stmt : func.statements) {
            analyzeStatement(*stmt);
        }
    }

    void SematicAnalyzer::analyzeStatement(const AST::Statement &stmt) {
        switch (stmt.kind) {
            case AST::StatementKind::Expression: {
                const auto &exprStmt = static_cast<const AST::ExpressionStatement&>(stmt);
                analyzeExpression(*exprStmt.expression);
                break;
            }
            case AST::StatementKind::Return: {
                const auto &returnStmt = static_cast<const AST::ReturnStatement&>(stmt);
                analyzeExpression(*returnStmt.expression);
                break;
            }
        }
    }

    void SematicAnalyzer::analyzeExpression(const AST::Expression &expr) {
        switch (expr.kind) {
            case AST::ExpressionKind::IntegerLiteral:
            case AST::ExpressionKind::StringLiteral:
                return;
            case AST::ExpressionKind::Name: {
                const auto &nameExpr = static_cast<const AST::NameExpression&>(expr);
                resolveQualifiedName(nameExpr.name, false);
                return;
            }

            case AST::ExpressionKind::Call: {
                const auto &callExpr = static_cast<const AST::CallExpression&>(expr);
                resolveQualifiedName(callExpr.callee, true);
                for (const auto &arg : callExpr.arguments) {
                    analyzeExpression(*arg);
                }

                return;
            }
        }
    }

    void SematicAnalyzer::resolveQualifiedName(const AST::QualifiedName &name, bool isCallable) {
        if (name.segments.empty()) {
            return;
        }

        const std::string &firstSegment = name.segments.front();
        if (name.segments.size() == 1U) {
            if (_functions.contains(firstSegment)) {
                return;
            }

            if (_visibleImports.contains(firstSegment)) {
                if (isCallable) {
                    _engine.error(
                        "SEM006",
                        "Imported module name '" + firstSegment + "' cannot be called directly. Use '" + firstSegment + "::...'.",
                        name.range
                    );
                } else {
                    _engine.error(
                        "SEM006",
                        "Imported module name '" + firstSegment + "' cannot be used as a value directly. Use '" + firstSegment + "::...'.",
                        name.range
                    );
                }
                return;
            }

            _engine.error(
                "SEM007",
                "Unknown symbol '" + firstSegment + "'.",
                name.range
            );
            return;
        }

        if (_visibleImports.contains(firstSegment)) {
            return;
        }

        _engine.error(
            "SEM008",
            "Unknown module qualifier '" + firstSegment + "'. Add a matching 'use' declaration.",
            name.range
        );
    }

    auto SematicAnalyzer::visibleImportName(const AST::UseDeclaration &useDecl) -> std::string {
        if (useDecl.alias.has_value()) {
            return useDecl.alias.value();
        }

        if (!useDecl.path.segments.empty()) {
            return useDecl.path.segments.back();
        }

        return {};
    }

    auto SematicAnalyzer::isBuiltinInt(const AST::TypeName &typeName) -> bool {
        return typeName.name.segments.size() == 1U && typeName.name.segments[0] == "int";
    }
}
