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
        _currentLocals.clear();
        _currentParameters.clear();

        if (!func.returnType.name.segments.empty()) {
            _currentFunctionReturnType = func.returnType.name.segments.front();
        } else {
            _currentFunctionReturnType.clear();
        }

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

        // Guaranteed return check must run only after all statements are analyzed.
        // Otherwise a function with console::println(...); return 0; would incorrectly get SEM017 on the first statement.
        if (_currentFunctionReturnType != "void") {
            if (func.statements.empty()) {
                _engine.error(
                    "SEM017",
                    "Non-void function must end with a return statement.",
                    func.range
                );
            } else {
                const auto &lastStmt = func.statements.back();
                if (!statementGuaranteesReturn(*lastStmt)) {
                    _engine.error(
                        "SEM017",
                        "Non-void function must end with a return statement.",
                        func.range
                    );
                }
            }
        }

        _currentParameters.clear();
        _currentLocals.clear();
        _currentFunctionReturnType.clear();
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
                const auto expectedType = typeFromString(_currentFunctionReturnType);
                if (returnStmt.expression == nullptr) {
                    if (expectedType != ExpressionType::Void) {
                        _engine.error(
                            "SEM015",
                            "non-void function must return a value.",
                            returnStmt.range
                        );
                    }
                    return;
                }

                const auto actual = analyzeExpressionType(*returnStmt.expression);
                if (expectedType == ExpressionType::Void) {
                    _engine.error(
                        "SEM016",
                        "Void function must return a value.",
                        returnStmt.range
                    );
                    return;
                }

                if (expectedType != ExpressionType::Unknown && actual != expectedType) {
                    _engine.error(
                        "SEM014",
                        "Return type mismatch.",
                        returnStmt.range
                    );
                }

                break;
            }
            case AST::StatementKind::VariableDeclaration: {
                const auto &varDecl = static_cast<const AST::VariableDeclarationStatement&>(stmt);
                if (_currentLocals.contains(varDecl.name)) {
                    _engine.error(
                        "SEM018",
                        "Duplicate local variable '" + varDecl.name + "'.",
                        varDecl.range
                    );

                    return;
                }

                const auto declType = typeFromTypeName(varDecl.type);
                const auto initType = analyzeExpressionType(*varDecl.initializer);

                if (declType != ExpressionType::Unknown && initType != declType) {
                    _engine.error(
                        "SEM019",
                        "Local variable initializer type mismatch.",
                        varDecl.range
                    );
                }

                _currentLocals.emplace(varDecl.name, LocalSymbol {
                    .type = declType,
                    .isMutable = varDecl.isMutable,
                });

                break;
            }
            case AST::StatementKind::Assignment: {
                const auto &assignment = static_cast<const AST::AssignmentStatement&>(stmt);
                const auto localIt = _currentLocals.find(assignment.name);
                if (localIt == _currentLocals.end()) {
                    _engine.error(
                        "SEM020",
                        "Unknown local variable '" + assignment.name + "'.",
                        assignment.range
                    );
                    return;
                }

                if (!localIt->second.isMutable) {
                    _engine.error(
                        "SEM021",
                        "Cannot assign to immutable local variable '" + assignment.name + "'.",
                        assignment.range
                    );
                }

                const auto valueType = analyzeExpressionType(*assignment.value);

                if (valueType != localIt->second.type) {
                    _engine.error(
                        "SEM022",
                        "Assignment type mismatch.",
                        assignment.range
                    );
                }

                break;
            }

            case AST::StatementKind::If: {
                const auto &ifStmt = static_cast<const AST::IfStatement&>(stmt);
                const auto conditionType = analyzeExpressionType(*ifStmt.condition);
                if (conditionType != ExpressionType::Bool) {
                    _engine.error(
                        "SEM023",
                        "If condition must be bool.",
                        ifStmt.condition->range
                    );
                }

                for (const auto &nested : ifStmt.thenBranch) {
                    analyzeStatement(*nested);
                }

                for (const auto &nested : ifStmt.elseBranch) {
                    analyzeStatement(*nested);
                }

                break;
            }
        }
    }

    void SemanticAnalyzer::analyzeExpression(const AST::Expression &expr) {
        switch (expr.kind) {
            case AST::ExpressionKind::IntegerLiteral:
            case AST::ExpressionKind::StringLiteral:
            case AST::ExpressionKind::BooleanLiteral:
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

                // Binary expressions must be checked not only in return statements,
                // but also inside call arguments, expression statements, etc.
                static_cast<void>(analyzeExpressionType(binaryExpr));

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

            if (!isCallable && _currentLocals.contains(firstSegment)) {
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

        const auto *imported = it->second;
        const std::string actual = importedModuleName(*imported);
        const auto *module = _modules.find(actual);
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

    auto SemanticAnalyzer::analyzeExpressionType(const AST::Expression &expression) -> ExpressionType {
        using namespace AST;

        switch (expression.kind) {
            case ExpressionKind::IntegerLiteral:
                return ExpressionType::Int;

            case ExpressionKind::StringLiteral:
                return ExpressionType::String;

            case ExpressionKind::Name: {
                const auto &nameExpr = static_cast<const NameExpression&>(expression);
                if (nameExpr.name.segments.size() == 1U) {
                    const std::string &name = nameExpr.name.segments.front();
                    if (_currentParameters.contains(name)) {
                        return ExpressionType::Int; // currently INT
                    }

                    const auto localIt = _currentLocals.find(name);
                    if (localIt != _currentLocals.end()) {
                        return localIt->second.type;
                    }
                }

                return ExpressionType::Unknown;
            }

            case ExpressionKind::Binary: {
                const auto &binaryExpr = static_cast<const BinaryExpression&>(expression);
                const auto left = analyzeExpressionType(*binaryExpr.left);
                const auto right = analyzeExpressionType(*binaryExpr.right);

                if (binaryExpr.op == BinaryOperator::Add) {
                    if (left == ExpressionType::Int && right == ExpressionType::Int) {
                        return ExpressionType::Int;
                    }

                    _engine.error(
                        "SEM013",
                        "Operator '+' requires integer operands.",
                        binaryExpr.range
                    );

                    return ExpressionType::Unknown;
                }

                return ExpressionType::Unknown;
            }

            case ExpressionKind::Call: {
                const auto &callExpr = static_cast<const CallExpression&>(expression);
                // Call type must be derived from a user function declaration or builtin module metadata.
                return analyzeCallExpressionType(callExpr);
            }

            case ExpressionKind::BooleanLiteral: {
                return ExpressionType::Bool;
            }
        }

        return ExpressionType::Unknown;
    }

    auto SemanticAnalyzer::typeFromTypeName(const AST::TypeName &typeName) -> ExpressionType {
        if (typeName.name.segments.size() != 1U) {
            return ExpressionType::Unknown;
        }

        return typeFromString(typeName.name.segments.front());
    }

    auto SemanticAnalyzer::analyzeCallExpressionType(const AST::CallExpression &callExpr) -> ExpressionType {
        if (callExpr.callee.segments.empty()) {
            return ExpressionType::Unknown;
        }

        // Single-segment call is a user-defined function call: helper().
        if (callExpr.callee.segments.size() == 1U) {
            const std::string &funcName = callExpr.callee.segments.front();
            const auto it = _functions.find(funcName);
            if (it == _functions.end()) {
                return ExpressionType::Unknown;
            }

            return typeFromTypeName(it->second->returnType);
        }

        // At this stage builtins only support console::println.
        // It does not return a useful value, so we treat it as void.
        const std::string &moduleName = callExpr.callee.segments[0];
        const std::string &funcName = callExpr.callee.segments[1];

        const auto importIt = _visibleImports.find(moduleName);
        if (importIt == _visibleImports.end()) {
            return ExpressionType::Unknown;
        }

        const std::string actual = importedModuleName(*importIt->second);
        const auto *module = _modules.find(actual);
        if (module == nullptr) {
            return ExpressionType::Unknown;
        }

        const auto *func = module->findFunction(funcName);
        if (func == nullptr) {
            return ExpressionType::Unknown;
        }

        // Builtin function types is now read from ModuleRegistry metadata.
        return typeFromString(func->returnType);
    }

    auto SemanticAnalyzer::typeFromString(const std::string &typeName) -> ExpressionType {
        if (typeName == "int") {
            return ExpressionType::Int;
        }

        if (typeName == "string") {
            return ExpressionType::String;
        }

        if (typeName == "void") {
            return ExpressionType::Void;
        }

        if (typeName == "bool") {
            return ExpressionType::Bool;
        }

        return ExpressionType::Unknown;
    }

    auto SemanticAnalyzer::importedModuleName(const AST::UseDeclaration &useDecl) -> std::string {
        if (useDecl.path.segments.empty()) {
            return {};
        }

        // use std::console as out; visible name = out, actual module = console
        return useDecl.path.segments.back();
    }

    auto SemanticAnalyzer::statementGuaranteesReturn(const AST::Statement &statement) -> bool {
        if (statement.kind == AST::StatementKind::Return) {
            return true;
        }

        if (statement.kind != AST::StatementKind::If) {
            return false;
        }

        const auto &ifStmt = static_cast<const AST::IfStatement&>(statement);
        if (ifStmt.thenBranch.empty() || ifStmt.elseBranch.empty()) {
            return false;
        }

        return statementGuaranteesReturn(*ifStmt.thenBranch.back()) && statementGuaranteesReturn(*ifStmt.elseBranch.back());
    }
}
