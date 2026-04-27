#include "velo/semantic/semantic_analyzer.h"

namespace Velo::Semantic {
    SemanticAnalyzer::SemanticAnalyzer(
        const AST::Program &program,
        Diagnostic::DiagnosticEngine &engine,
        const Module::ModuleRegistry &modules
        ) : _program(program), _engine(engine), _modules(modules) {
    }

    auto SemanticAnalyzer::analyze() -> bool {
        collectImports();
        collectFunctions();
        validateEntryPoint();

        for (const auto &func : _program.functions) {
            analyzeFunction(func);
        }

        return !_engine.hasErrors();
    }

    void SemanticAnalyzer::collectImports() {
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

    void SemanticAnalyzer::collectFunctions() {
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

    void SemanticAnalyzer::validateEntryPoint() {
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

    void SemanticAnalyzer::analyzeFunction(const AST::FunctionDeclaration &func) {
        _currentParameters.clear();

        for (const auto &param : func.parameters) {
            const auto [it, inserted] = _currentParameters.insert(param.name);
            if (!inserted) {
                _engine.error(
                    "SEM012",
                    "Duplicate parameter '" + param.name + "'.",
                    param.range
                );
            }
        }

        for (const auto &stmt : func.statements) {
            analyzeStatement(*stmt);
        }

        _currentParameters.clear();
    }

    void SemanticAnalyzer::analyzeStatement(const AST::Statement &stmt) {
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

    void SemanticAnalyzer::analyzeExpression(const AST::Expression &expr) {
        switch (expr.kind) {
            case AST::ExpressionKind::IntegerLiteral:
            case AST::ExpressionKind::StringLiteral:
                return;
            case AST::ExpressionKind::Name: {
                const auto &nameExpr = static_cast<const AST::NameExpression&>(expr);
                resolveQualifiedName(nameExpr.name, false, 0U);
                return;
            }

            case AST::ExpressionKind::Call: {
                const auto &callExpr = static_cast<const AST::CallExpression&>(expr);
                resolveQualifiedName(callExpr.callee, true, callExpr.arguments.size());
                for (const auto &arg : callExpr.arguments) {
                    analyzeExpression(*arg);
                }

                return;
            }

            case AST::ExpressionKind::Binary: {
                const auto &binaryExpr = static_cast<const AST::BinaryExpression&>(expr);

                analyzeExpression(*binaryExpr.left);
                analyzeExpression(*binaryExpr.right);

                return;
            }
        }
    }

    void SemanticAnalyzer::resolveQualifiedName(const AST::QualifiedName &name, bool isCallable, std::size_t argsCount) {
        if (name.segments.empty()) {
            return;
        }

        const std::string &firstSegment = name.segments.front();
        if (name.segments.size() == 1U) {
            if (const auto funcIterator = _functions.find(firstSegment); funcIterator != _functions.end()) {
                if (const auto *f = funcIterator->second; isCallable && f->parameters.size() != argsCount) {
                    _engine.error(
                        "SEM011",
                        "Function '" + firstSegment + "' expects " +
                        std::to_string(f->parameters.size()) +
                        " argument(s), but "
                        + std::to_string(argsCount)
                        + " provded.",
                        name.range
                    );
                }
                return;
            }

            if (!isCallable && _currentParameters.contains(firstSegment)) {
                return;
            }

            if (_visibleImports.contains(firstSegment)) {
                _engine.error(
                    "SEM006",
                    isCallable ?
                        "Imported module name '" + firstSegment + "' cannot be called directly. Use '" + firstSegment + "::...'." :
                        "Imported module name '" + firstSegment + "' cannot be used as a value directly. Use '" + firstSegment +"::...'.",
                        name.range
                );
                return;
            }

            _engine.error(
                "SEM007",
                "Unknown symbol '" + firstSegment + "'.",
                name.range
            );
            return;
        }

        const auto it = _visibleImports.find(firstSegment);
        if (it == _visibleImports.end()) {
            _engine.error(
                "SEM008",
                "Unknown module qualifier '" + firstSegment + "'. Add a matching 'use' declaration.",
                name.range
            );
            return;
        }

        const auto *module = _modules.find(firstSegment);
        if (module == nullptr) {
            _engine.error(
                "SEM008",
                "Unknown module qualifier '" + firstSegment + "'. Add a matching 'use' declaration.",
                name.range
            );

            return;
        }

        const std::string &funcName = name.segments[1];
        const auto *func = module->findFunction(funcName);
        if (func == nullptr) {
            _engine.error(
                "SEM009",
                "Unknown function '" + funcName + "' in module '" + firstSegment + "'.",
                name.range
            );
            return;
        }

        if (isCallable && func->arity != argsCount) {
            _engine.error(
                "SEM010",
                "Function '" + firstSegment + "::" + funcName + "' expects " +
                std::to_string(func->arity) +
                " argument(s), but " +
                std::to_string(argsCount) +
                " provided.",
                name.range
            );
        }
    }

    auto SemanticAnalyzer::visibleImportName(const AST::UseDeclaration &useDecl) -> std::string {
        if (useDecl.alias.has_value()) {
            return useDecl.alias.value();
        }

        if (!useDecl.path.segments.empty()) {
            return useDecl.path.segments.back();
        }

        return {};
    }

    auto SemanticAnalyzer::isBuiltinInt(const AST::TypeName &typeName) -> bool {
        return typeName.name.segments.size() == 1U && typeName.name.segments[0] == "int";
    }
}
