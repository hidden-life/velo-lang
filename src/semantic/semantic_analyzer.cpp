#include "velo/semantic/semantic_analyzer.h"

namespace {
    [[nodiscard]] auto rootNameExpression(const Velo::AST::Expression &expr) -> const Velo::AST::NameExpression* {
        using namespace Velo::AST;

        if (expr.kind == ExpressionKind::Name) {
            return &static_cast<const NameExpression&>(expr);
        }

        if (expr.kind == ExpressionKind::FieldAccess) {
            const auto &fieldAccess = static_cast<const FieldAccessExpression&>(expr);
            return rootNameExpression(*fieldAccess.object);
        }

        if (expr.kind == ExpressionKind::Index) {
            const auto &indexExpression = static_cast<const IndexExpression&>(expr);
            return rootNameExpression(*indexExpression.object);
        }

        return nullptr;
    }

    [[nodiscard]] auto isEqualityOperator(Velo::AST::BinaryOperator op) -> bool {
        return op == Velo::AST::BinaryOperator::Equal || op == Velo::AST::BinaryOperator::NotEqual;
    }

    [[nodiscard]] auto isOrderingComparisonOperator(Velo::AST::BinaryOperator op) -> bool {
        return op == Velo::AST::BinaryOperator::Less ||
            op == Velo::AST::BinaryOperator::LessEqual ||
            op == Velo::AST::BinaryOperator::Greater ||
            op == Velo::AST::BinaryOperator::GreaterEqual;
    }

    [[nodiscard]] auto isEqualityComparableType(const Velo::Semantic::SemanticType &type) -> bool {
        using Velo::Semantic::SemanticTypeKind;

        return type.kind == SemanticTypeKind::Int ||
            type.kind == SemanticTypeKind::String ||
            type.kind == SemanticTypeKind::Bool;
    }

    [[nodiscard]] auto makeArrayType(Velo::Semantic::SemanticType elementType) -> Velo::Semantic::SemanticType {
        return Velo::Semantic::SemanticType {
            .kind = Velo::Semantic::SemanticTypeKind::Array,
            .name = {},
            .elementType = std::make_shared<Velo::Semantic::SemanticType>(std::move(elementType))
        };
    }

    [[nodiscard]] auto makeMapType(
        Velo::Semantic::SemanticType keyType,
        Velo::Semantic::SemanticType valueType
    ) -> Velo::Semantic::SemanticType {
        return Velo::Semantic::SemanticType {
            .kind = Velo::Semantic::SemanticTypeKind::Map,
            .name = {},
            .elementType = {},
            .keyType = std::make_shared<Velo::Semantic::SemanticType>(std::move(keyType)),
            .valueType = std::make_shared<Velo::Semantic::SemanticType>(std::move(valueType)),
        };
    }

    [[nodiscard]] auto containsIndexExpression(const Velo::AST::Expression &expr) -> bool {
        using namespace Velo::AST;

        if (expr.kind == ExpressionKind::Index) {
            return true;
        }

        if (expr.kind == ExpressionKind::FieldAccess) {
            const auto &fieldAccess = static_cast<const FieldAccessExpression&>(expr);
            return containsIndexExpression(*fieldAccess.object);
        }

        return false;
    }
}

namespace Velo::Semantic {
    SemanticAnalyzer::SemanticAnalyzer(
        const AST::Program &program,
        Diagnostic::DiagnosticEngine &engine,
        const Module::ModuleRegistry &modules
        ) : _program(program), _engine(engine), _modules(modules) {
    }

    auto SemanticAnalyzer::analyze() -> bool {
        collectImports();
        collectStructs();
        collectFunctions();
        validateEntryPoint();

        for (const auto &strDecl : _program.structs) {
            analyzeStruct(strDecl);
        }

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

        const auto mainReturnType = typeFromTypeName(mainFunc.returnType);
        if (!isUnknownType(mainReturnType) && !isIntType(mainReturnType)) {
            _engine.error(
                "SEM005",
                "Entry point 'main' must return int.",
                mainFunc.returnType.range
            );
        }
    }

    void SemanticAnalyzer::analyzeFunction(const AST::FunctionDeclaration &func) {
        _currentParameters.clear();
        _scopeStack.clear();
        validateDeclaredType(func.returnType, true, "function '" + func.name + "' return type");
        _currentFunctionReturnType = typeFromTypeName(func.returnType);
        pushScope();

        for (const auto &param : func.parameters) {
            validateDeclaredType(param.type, false, "parameter '" + param.name + "'");
            auto paramType = typeFromTypeName(param.type);
            // If the parameter type is invalid for value position, keep it Unknown
            // to reduce follow-up type mismatch diagnostics.
            if (isVoidType(paramType)) {
                paramType = {};
            }
            const auto [it, inserted] = _currentParameters.emplace(param.name, paramType);
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
        // Otherwise, a function with console::println(...); return 0; would incorrectly get SEM017 on the first statement.
        if (!isVoidType(_currentFunctionReturnType)) {
            if (func.statements.empty() || !statementGuaranteesReturn(*func.statements.back())) {
                _engine.error(
                    "SEM017",
                    "Non-void function must end with a return statement.",
                    func.range
                );
            }
        }

        _currentParameters.clear();
        _scopeStack.clear();
        _currentFunctionReturnType = {};
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
                const auto expectedType = _currentFunctionReturnType;
                if (returnStmt.expression == nullptr) {
                    if (!isVoidType(expectedType)) {
                        _engine.error(
                            "SEM015",
                            "non-void function must return a value.",
                            returnStmt.range
                        );
                    }
                    return;
                }

                const auto actual = analyzeExpressionTypeWithExpected(*returnStmt.expression, &expectedType);

                if (isVoidType(expectedType)) {
                    _engine.error(
                        "SEM016",
                        "Void function must not return a value.",
                        returnStmt.range
                    );
                    return;
                }

                if (!isUnknownType(expectedType) && !isUnknownType(actual) && !typesEqual(actual, expectedType)) {
                    _engine.error(
                        "SEM014",
                        "Return type mismatch. Expected '" + semanticTypeToString(expectedType) + "', actual '" +
                        semanticTypeToString(actual) + "'.",
                        returnStmt.range
                    );
                }

                break;
            }
            case AST::StatementKind::VariableDeclaration: {
                const auto &varDecl = static_cast<const AST::VariableDeclarationStatement&>(stmt);
                validateDeclaredType(varDecl.type, false, "local variable '" + varDecl.name + "'");
                auto declType = typeFromTypeName(varDecl.type);
                if (isVoidType(declType)) {
                    declType = {};
                }

                const auto initType = analyzeExpressionTypeWithExpected(*varDecl.initializer, &declType);

                if (!isUnknownType(declType) && !isUnknownType(initType) && !typesEqual(initType, declType)) {
                    _engine.error(
                        "SEM019",
                        "Local variable initializer type mismatch. Expected '"
                        + semanticTypeToString(declType) + "', actual '" +
                        semanticTypeToString(initType) + "'.",
                        varDecl.range
                    );
                }

                if (!declareLocal(
                    varDecl.name,
                    LocalSymbol {
                        .type = declType,
                        .isMutable = varDecl.isMutable
                    }
                )) {
                    _engine.error(
                        "SEM018",
                        "Duplicate local variable '" + varDecl.name + "'.",
                        varDecl.range
                    );
                }

                break;
            }
            case AST::StatementKind::Assignment: {
                const auto &assignment = static_cast<const AST::AssignmentStatement&>(stmt);
                const auto *local = resolveLocal(assignment.name);
                if (local == nullptr) {
                    _engine.error(
                        "SEM020",
                        "Unknown local variable '" + assignment.name + "'.",
                        assignment.range
                    );
                    return;
                }

                if (!local->isMutable) {
                    _engine.error(
                        "SEM021",
                        "Cannot assign to immutable local variable '" + assignment.name + "'.",
                        assignment.range
                    );
                }

                const auto valueType = analyzeExpressionTypeWithExpected(*assignment.value, &local->type);

                if (!isUnknownType(valueType) && !isUnknownType(local->type) && !typesEqual(valueType, local->type)) {
                    _engine.error(
                        "SEM022",
                        "Assignment type mismatch. Expected '" + semanticTypeToString(local->type) + "', actual '" +
                        semanticTypeToString(valueType) + "'.",
                        assignment.range
                    );
                }

                break;
            }

            case AST::StatementKind::If: {
                const auto &ifStmt = static_cast<const AST::IfStatement&>(stmt);
                const auto conditionType = analyzeCheckedExpressionType(*ifStmt.condition);
                if (!isBoolType(conditionType) && !isUnknownType(conditionType)) {
                    _engine.error(
                        "SEM023",
                        "If condition must be bool.",
                        ifStmt.condition->range
                    );
                }

                pushScope();
                for (const auto &nested : ifStmt.thenBranch) {
                    analyzeStatement(*nested);
                }
                popScope();

                pushScope();
                for (const auto &nested : ifStmt.elseBranch) {
                    analyzeStatement(*nested);
                }
                popScope();

                break;
            }

            case AST::StatementKind::While: {
                const auto &whileStmt = static_cast<const AST::WhileStatement&>(stmt);
                const auto conditionType = analyzeCheckedExpressionType(*whileStmt.condition);
                if (!isBoolType(conditionType) && !isUnknownType(conditionType)) {
                    _engine.error(
                        "SEM025",
                        "While condition must be bool.",
                        whileStmt.condition->range
                    );
                }

                _loopDepth++;
                pushScope();
                for (const auto &nested : whileStmt.body) {
                    analyzeStatement(*nested);
                }
                popScope();
                _loopDepth--;

                break;
            }

            case AST::StatementKind::Break: {
                if (_loopDepth == 0) {
                    _engine.error(
                        "SEM026",
                        "break used outside of loop.",
                        stmt.range
                    );
                }

                break;
            }

            case AST::StatementKind::Continue: {
                if (_loopDepth == 0) {
                    _engine.error(
                        "SEM027",
                        "continue used outside of loop.",
                        stmt.range
                    );
                }

                break;
            }

            case AST::StatementKind::FieldAssignment: {
                const auto &fieldAssignment = static_cast<const AST::FieldAssignmentStatement&>(stmt);
                analyzeFieldAssignmentStatement(fieldAssignment);
                break;
            }

            case AST::StatementKind::IndexAssignment: {
                const auto &indexAssignment = static_cast<const AST::IndexAssignmentStatement&>(stmt);
                analyzeIndexAssignmentStatement(indexAssignment);
                break;
            }
        }
    }

    void SemanticAnalyzer::analyzeFieldAssignmentStatement(const AST::FieldAssignmentStatement &stmt) {
        if (containsIndexExpression(*stmt.target)) {
            _engine.error(
                "SEM045",
                "Field assignment through array index is not supported yet.",
                stmt.target->range
            );

            static_cast<void>(analyzeCheckedExpressionType(*stmt.value));
            return;
        }

        const auto *rootName = rootNameExpression(*stmt.target);
        if (rootName == nullptr || rootName->name.segments.size() != 1U) {
            _engine.error(
                "SEM045",
                "Field assignment target must start from a local variable.",
                stmt.target->range
            );

            static_cast<void>(analyzeCheckedExpressionType(*stmt.value));
            return;
        }

        const std::string &rootLocalName = rootName->name.segments.front();
        const auto *local = resolveLocal(rootLocalName);
        if (local == nullptr) {
            _engine.error(
                "SEM045",
                "Field assignment target must start from a local variable.",
                rootName->range
            );

            static_cast<void>(analyzeCheckedExpressionType(*stmt.value));
            return;
        }

        if (!local->isMutable) {
            _engine.error(
                "SEM046",
                "Cannot assign to field through immutable local variable '" + rootLocalName + "'.",
                rootName->range
            );
        }

        const auto targetType = analyzeFieldAccessExpressionType(*stmt.target);
        const auto valueType = analyzeCheckedExpressionType(*stmt.value);

        if (!isUnknownType(targetType) && !isUnknownType(valueType) && !typesEqual(targetType, valueType)) {
            _engine.error(
                "SEM047",
                "Field assignment type mismatch. Expected '" + semanticTypeToString(targetType) + "', actual" +
                semanticTypeToString(valueType) + "'.",
                stmt.range
            );
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

            case AST::ExpressionKind::StructLiteral: {
                const auto &structLiteral = static_cast<const AST::StructLiteralExpression&>(expr);
                static_cast<void>(analyzeStructLiteralExpressionType(structLiteral));

                return;
            }

            case AST::ExpressionKind::ArrayLiteral: {
                const auto &arrayLiteral = static_cast<const AST::ArrayLiteralExpression&>(expr);
                static_cast<void>(analyzeArrayLiteralExpressionType(arrayLiteral));
                return;
            }

            case AST::ExpressionKind::MapLiteral: {
                const auto &mapLiteral = static_cast<const AST::MapLiteralExpression&>(expr);
                static_cast<void>(analyzeMapLiteralExpressionType(mapLiteral));
                return;
            }

            case AST::ExpressionKind::FieldAccess: {
                const auto &fieldAccess = static_cast<const AST::FieldAccessExpression&>(expr);
                static_cast<void>(analyzeFieldAccessExpressionType(fieldAccess));

                return;
            }

            case AST::ExpressionKind::Index: {
                const auto &indexExpr = static_cast<const AST::IndexExpression&>(expr);
                static_cast<void>(analyzeIndexExpressionType(indexExpr));
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

            case AST::ExpressionKind::Unary: {
                const auto &unaryExpr = static_cast<const AST::UnaryExpression&>(expr);
                analyzeExpression(*unaryExpr.operand);
                // Unary expressions must be type-checked in all expression contexts.
                static_cast<void>(analyzeExpressionType(unaryExpr));

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

            if (!isCallable && _currentParameters.find(firstSegment) != _currentParameters.end()) {
                return;
            }

            if (!isCallable && resolveLocal(firstSegment) != nullptr) {
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

    auto SemanticAnalyzer::analyzeExpressionType(const AST::Expression &expression) -> SemanticType {
        using namespace AST;

        switch (expression.kind) {
            case ExpressionKind::IntegerLiteral:
                return SemanticType {
                    .kind = SemanticTypeKind::Int
                };

            case ExpressionKind::StringLiteral:
                return SemanticType {
                    .kind = SemanticTypeKind::String
                };

            case ExpressionKind::Name: {
                const auto &nameExpr = static_cast<const NameExpression&>(expression);
                if (nameExpr.name.segments.size() == 1U) {
                    const std::string &name = nameExpr.name.segments.front();
                    const auto paramIt = _currentParameters.find(name);
                    if (paramIt != _currentParameters.end()) {
                        return paramIt->second;
                    }

                    const auto *local = resolveLocal(name);
                    if (local != nullptr) {
                        return local->type;
                    }
                }

                return {};
            }

            case ExpressionKind::StructLiteral: {
                const auto &structLiteral = static_cast<const StructLiteralExpression&>(expression);
                return analyzeStructLiteralExpressionType(structLiteral);
            }

            case ExpressionKind::ArrayLiteral: {
                const auto &arrayLiteral = static_cast<const ArrayLiteralExpression&>(expression);
                return analyzeArrayLiteralExpressionType(arrayLiteral);
            }

            case ExpressionKind::MapLiteral: {
                const auto &mapLiteral = static_cast<const MapLiteralExpression&>(expression);

                return analyzeMapLiteralExpressionType(mapLiteral);
            }

            case ExpressionKind::FieldAccess: {
                const auto &fieldAccess = static_cast<const FieldAccessExpression&>(expression);
                return analyzeFieldAccessExpressionType(fieldAccess);
            }

            case ExpressionKind::Index: {
                const auto &indexExpression = static_cast<const IndexExpression&>(expression);
                return analyzeIndexExpressionType(indexExpression);
            }

            case ExpressionKind::Binary: {
                const auto &binaryExpr = static_cast<const BinaryExpression&>(expression);
                const auto left = analyzeExpressionType(*binaryExpr.left);
                const auto right = analyzeExpressionType(*binaryExpr.right);

                if (binaryExpr.op == BinaryOperator::Add ||
                    binaryExpr.op == BinaryOperator::Subtract ||
                    binaryExpr.op == BinaryOperator::Multiply ||
                    binaryExpr.op == BinaryOperator::Divide ||
                    binaryExpr.op == BinaryOperator::Modulo
                ) {
                    if (isIntType(left) && isIntType(right)) {
                        return typeFromString("int");
                    }

                    _engine.error(
                    "SEM013",
                    "Arithmetic operators require integer operands.",
                        binaryExpr.range
                    );

                    return {};
                }

                if (binaryExpr.op == BinaryOperator::LogicalAnd || binaryExpr.op == BinaryOperator::LogicalOr) {
                    if (isBoolType(left) && isBoolType(right)) {
                        return typeFromString("bool");
                    }

                    _engine.error(
                    "SEM029",
                    "Logical operators require bool operands.",
                        binaryExpr.range
                    );

                    return {};
                }

                if (isEqualityOperator(binaryExpr.op)) {
                    if (isUnknownType(left) || isUnknownType(right)) {
                        return {};
                    }

                    if (typesEqual(left, right) && isEqualityComparableType(left)) {
                        return typeFromString("bool");
                    }

                    _engine.error(
                        "SEM048",
                        "Equality operators require operands of the same comparable type. Left '" +
                        semanticTypeToString(left) +
                        "', right '" +
                        semanticTypeToString(right) +
                        "'.",
                        binaryExpr.range
                    );

                    return {};
                }

                if (isOrderingComparisonOperator(binaryExpr.op)) {
                    if (isIntType(left) && isIntType(right)) {
                        return typeFromString("bool");
                    }

                    _engine.error(
                        "SEM024",
                        "Comparison operators require integer operands.",
                        binaryExpr.range
                    );

                    return {};
                }

                return {};
            }

            case ExpressionKind::Call: {
                const auto &callExpr = static_cast<const CallExpression&>(expression);
                // Call type must be derived from a user function declaration or builtin module metadata.
                return analyzeCallExpressionType(callExpr);
            }

            case ExpressionKind::BooleanLiteral: {
                return SemanticType {
                    .kind = SemanticTypeKind::Bool
                };
            }

            case ExpressionKind::Unary: {
                const auto &unaryExpr = static_cast<const UnaryExpression&>(expression);
                const auto operandType = analyzeExpressionType(*unaryExpr.operand);
                switch (unaryExpr.op) {
                    case UnaryOperator::Not: {
                        if (isBoolType(operandType)) {
                            return SemanticType {
                                .kind = SemanticTypeKind::Bool
                            };
                        }

                        if (!isUnknownType(operandType)) {
                            _engine.error(
                            "SEM028",
                            "Operator '!' requires bool operand.",
                                unaryExpr.range
                            );
                        }

                        return {};
                    }

                    case UnaryOperator::Negate: {
                        if (isIntType(operandType)) {
                            return SemanticType {
                                .kind = SemanticTypeKind::Int
                            };
                        }

                        if (!isUnknownType(operandType)) {
                            _engine.error(
                            "SEM032",
                            "Unary '-' requires int operand.",
                                unaryExpr.range
                            );
                        }

                        return {};
                    }
                }
            }
        }

        return {};
    }

    auto SemanticAnalyzer::typeFromTypeName(const AST::TypeName &typeName) -> SemanticType {
        SemanticType baseType {};
        if (typeName.kind == AST::TypeNameKind::Map) {
            if (typeName.mapKeyType == nullptr || typeName.mapValueType == nullptr) {
                return {};
            }

            auto keyType = typeFromTypeName(*typeName.mapKeyType);
            auto valueType = typeFromTypeName(*typeName.mapValueType);

            if (isUnknownType(keyType) || isUnknownType(valueType)) {
                return {};
            }

            if (!isStringType(keyType)) {
                return {};
            }

            if (isVoidType(valueType)) {
                return {};
            }

            baseType = makeMapType(std::move(keyType), std::move(valueType));
        } else {
            if (typeName.name.segments.size() != 1U) {
                return {};
            }

            const std::string &name = typeName.name.segments.front();
            baseType = typeFromString(name);

            if (isUnknownType(baseType) && resolveUserDefinedType(typeName) != nullptr) {
                baseType = SemanticType {
                    .kind = SemanticTypeKind::Struct,
                    .name = name,
                };
            }

            if (isUnknownType(baseType)) {
                return {};
            }
        }

        if (typeName.arrayDepth == 0U) {
            return baseType;
        }

        if (isVoidType(baseType)) {
            return {};
        }

        SemanticType result = std::move(baseType);
        for (std::size_t idx = 0; idx < typeName.arrayDepth; ++idx) {
            result = makeArrayType(std::move(result));
        }

        return result;
    }

    auto SemanticAnalyzer::analyzeCallExpressionType(const AST::CallExpression &callExpr) -> SemanticType {
        if (callExpr.callee.segments.empty()) {
            return {};
        }

        // Single-segment call is a user-defined function call: helper().
        if (callExpr.callee.segments.size() == 1U) {
            const std::string &funcName = callExpr.callee.segments.front();
            const auto it = _functions.find(funcName);
            if (it == _functions.end()) {
                return {};
            }

            const auto &targetFunc = *it->second;
            for (std::size_t idx = 0; idx < callExpr.arguments.size(); ++idx) {
                if (idx >= targetFunc.parameters.size()) {
                    break;
                }

                const auto expectedType = typeFromTypeName(targetFunc.parameters[idx].type);
                const auto actualType = analyzeExpressionTypeWithExpected(*callExpr.arguments[idx], &expectedType);

                if (!isUnknownType(expectedType) && !isUnknownType(actualType) && !typesEqual(actualType, expectedType)) {
                    _engine.error(
                        "SEM037",
                        "Function argument type mismatch. Expected '" +
                        semanticTypeToString(expectedType) +
                        "', actual '" + semanticTypeToString(actualType) + "'.",
                        callExpr.arguments[idx]->range
                    );
                }
            }

            return typeFromTypeName(targetFunc.returnType);
        }

        // At this stage builtins only support console::println.
        // It does not return a useful value, so we treat it as void.
        const std::string &moduleName = callExpr.callee.segments[0];
        const std::string &funcName = callExpr.callee.segments[1];

        const auto importIt = _visibleImports.find(moduleName);
        if (importIt == _visibleImports.end()) {
            return {};
        }

        const std::string actual = importedModuleName(*importIt->second);
        const auto *module = _modules.find(actual);
        if (module == nullptr) {
            return {};
        }

        const auto *func = module->findFunction(funcName);
        if (func == nullptr) {
            return {};
        }

        for (std::size_t idx = 0; idx < callExpr.arguments.size(); ++idx) {
            if (idx >= func->parameterTypes.size()) {
                break;
            }

            const std::string &expectedTypeName = func->parameterTypes[idx];
            if (expectedTypeName == "any") {
                continue;
            }

            const auto actualType = analyzeCheckedExpressionType(*callExpr.arguments[idx]);
            if (!builtinParameterAcceptsType(expectedTypeName, actualType)) {
                _engine.error(
                    "SEM033",
                    "Builtin argument type mismatch. Expected '" + expectedTypeName + "', actual '" +
                    semanticTypeToString(actualType) + "'.",
                    callExpr.arguments[idx]->range
                );
            }
        }

        // Builtin function types is now read from ModuleRegistry metadata.
        return typeFromString(func->returnType);
    }

    auto SemanticAnalyzer::typeFromString(const std::string &typeName) -> SemanticType {
        if (typeName == "int") {
            return SemanticType {
                .kind = SemanticTypeKind::Int,
            };
        }

        if (typeName == "string") {
            return SemanticType {
                .kind = SemanticTypeKind::String,
            };
        }

        if (typeName == "void") {
            return SemanticType {
                .kind = SemanticTypeKind::Void,
            };
        }

        if (typeName == "bool") {
            return SemanticType {
                .kind = SemanticTypeKind::Bool,
            };
        }

        return {};
    }

    auto SemanticAnalyzer::analyzeFieldAccessExpressionType(const AST::FieldAccessExpression &expr) -> SemanticType {
        const auto objectType = analyzeCheckedExpressionType(*expr.object);
        if (isUnknownType(objectType)) {
            return {};
        }

        const auto *structDecl = resolveStructType(objectType);
        if (structDecl == nullptr) {
            _engine.error(
                "SEM043",
                "Field access target must be struct, actual '" + semanticTypeToString(objectType) + "'.",
                expr.object->range
            );

            return {};
        }

        for (const auto &field : structDecl->fields) {
            if (field.name == expr.fieldName) {
                return typeFromTypeName(field.type);
            }
        }

        _engine.error(
            "SEM044",
            "Unknown field '" + expr.fieldName + "' in struct '" + structDecl->name + "'.",
            expr.fieldRange
        );

        return {};
    }

    auto SemanticAnalyzer::resolveStructType(const SemanticType &type) const -> const AST::StructDeclaration* {
        if (type.kind != SemanticTypeKind::Struct) {
            return nullptr;
        }

        const auto it = _structs.find(type.name);
        if (it == _structs.end()) {
            return nullptr;
        }

        return it->second;
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

    void SemanticAnalyzer::pushScope() {
        _scopeStack.emplace_back();
    }

    void SemanticAnalyzer::popScope() {
        if (!_scopeStack.empty()) {
            _scopeStack.pop_back();
        }
    }

    auto SemanticAnalyzer::declareLocal(const std::string &name, const LocalSymbol &symbol) -> bool {
        if (_scopeStack.empty()) {
            pushScope();
        }

        auto &current = _scopeStack.back();
        // Duplicate declaration is forbidden only inside the current scope.
        // Shadowing a variable from an outer scope is allowed.
        if (current.contains(name)) {
            return false;
        }

        current.emplace(name, symbol);
        return true;
    }

    auto SemanticAnalyzer::resolveLocal(const std::string &name) const -> const LocalSymbol* {
        for (auto it = _scopeStack.rbegin(); it != _scopeStack.rend(); ++it) {
            const auto found = it->find(name);
            if (found != it->end()) {
                return &found->second;
            }
        }

        return nullptr;
    }

    void SemanticAnalyzer::validateDeclaredType(
        const AST::TypeName &typeName,
        bool allowVoid,
        const std::string &subject
    ) {
        if (typeName.arrayDepth > 0U &&
            typeName.kind == AST::TypeNameKind::Named &&
            typeName.name.segments.size() == 1U &&
            typeName.name.segments.front() == "void"
        ) {
            _engine.error(
                "SEM031",
                "Type 'void' cannot be used as an array element type in " + subject + ".",
                typeName.range
            );
            return;
        }


        if (typeName.kind == AST::TypeNameKind::Map) {
            if (typeName.mapKeyType == nullptr || typeName.mapValueType == nullptr) {
                _engine.error(
                    "SEM057",
                    "Map type must have key and value types in " + subject + ".",
                    typeName.range
                );

                return;
            }

            const auto keyType = typeFromTypeName(*typeName.mapKeyType);
            if (isUnknownType(keyType)) {
                _engine.error(
                    "SEM030",
                    "Unknown type '" + typeNameToString(*typeName.mapKeyType) + "'.",
                    typeName.mapKeyType->range
                );

                return;
            }

            if (!isStringType(keyType)) {
                _engine.error(
                    "SEM056",
                    "Map key type must be string, actual '" + semanticTypeToString(keyType) + "'.",
                    typeName.mapKeyType->range
                );

                return;
            }

            const auto valueType = typeFromTypeName(*typeName.mapValueType);
            if (isUnknownType(valueType)) {
                _engine.error(
                    "SEM030",
                    "Unknown type '" + typeNameToString(*typeName.mapValueType) + "'.",
                    typeName.mapValueType->range
                );

                return;
            }

            if (isVoidType(valueType)) {
                _engine.error(
                    "SEM031",
                    "Type 'void' cannot be used as map value type in " + subject + ".",
                    typeName.mapValueType->range
                );

                return;
            }
        }

        const auto type = typeFromTypeName(typeName);
        if (isUnknownType(type)) {
            _engine.error(
                "SEM030",
                "Unknown type '" + typeNameToString(typeName) + "'.",
                typeName.range
            );
            return;
        }

        if (!allowVoid && isVoidType(type)) {
            _engine.error(
                "SEM031",
                "Type 'void' cannot be used in " + subject + ".",
                typeName.range
            );
        }
    }

    auto SemanticAnalyzer::typeNameToString(const AST::TypeName &typeName) -> std::string {
        std::string result;
        for (std::size_t idx = 0; idx < typeName.arrayDepth; ++idx) {
            result += "[]";
        }

        if (typeName.kind == AST::TypeNameKind::Map) {
            result += "map<";

            if (typeName.mapKeyType != nullptr) {
                result += typeNameToString(*typeName.mapKeyType);
            } else {
                result += "unknown";
            }

            result += ", ";

            if (typeName.mapValueType != nullptr) {
                result += typeNameToString(*typeName.mapValueType);
            } else {
                result += "unknown";
            }

            result += ">";

            return result;
        }

        for (std::size_t idx = 0; idx < typeName.name.segments.size(); ++idx) {
            if (idx > 0U) {
                result += "::";
            }
            result += typeName.name.segments[idx];
        }

        return result;
    }

    auto SemanticAnalyzer::analyzeCheckedExpressionType(const AST::Expression &expression) -> SemanticType {
        const auto errorsBefore = _engine.size();
        analyzeExpression(expression);

        // If name/function/module validation already reported an error,
        // do not run type analysis to avoid duplicate/cascading diagnostics.
        if (_engine.size() != errorsBefore) {
            return {};
        }

        return analyzeExpressionType(expression);
    }

    auto SemanticAnalyzer::builtinParameterAcceptsType(const std::string &expected, const SemanticType &actual) -> bool {
        if (expected == "any") {
            return true;
        }

        if (isUnknownType(actual)) {
            return true;
        }

        if (expected == "int") {
            return isIntType(actual);
        }

        if (expected == "string") {
            return isStringType(actual);
        }

        if (expected == "bool") {
            return isBoolType(actual);
        }

        if (expected == "array") {
            return actual.kind == SemanticTypeKind::Array;
        }

        return false;
    }

    void SemanticAnalyzer::collectStructs() {
        for (const auto &structDecl : _program.structs) {
            if (isBuiltinTypeName(structDecl.name)) {
                _engine.error(
                    "SEM036",
                    "Struct name '" + structDecl.name + "' conflicts with a built-in type.",
                    structDecl.range
                );
                continue;
            }

            const auto [it, inserted] = _structs.emplace(structDecl.name, &structDecl);
            static_cast<void>(it);

            if (!inserted) {
                _engine.error(
                    "SEM034",
                    "Duplicate struct declaration '" + structDecl.name + "'.",
                    structDecl.range
                );
            }
        }
    }

    void SemanticAnalyzer::analyzeStruct(const AST::StructDeclaration &structDecl) {
        std::unordered_set<std::string> fieldNames;
        for (const auto &field : structDecl.fields) {
            const auto [it, inserted] = fieldNames.insert(field.name);
            static_cast<void>(it);

            if (!inserted) {
                _engine.error(
                    "SEM035",
                    "Duplicate field '" + field.name + "' in struct '" + structDecl.name + "'.",
                    field.range
                );
            }

            validateDeclaredType(
                field.type,
                false,
                "field '" + structDecl.name + "::" + field.name + "'"
            );
        }
    }

    auto SemanticAnalyzer::resolveUserDefinedType(const AST::TypeName &typeName) const -> const AST::StructDeclaration* {
        if (typeName.kind != AST::TypeNameKind::Named) {
            return nullptr;
        }

        if (typeName.name.segments.size() != 1U) {
            return nullptr;
        }

        const std::string &name = typeName.name.segments.front();
        const auto it = _structs.find(name);
        if (it == _structs.end()) {
            return nullptr;
        }

        return it->second;
    }

    auto SemanticAnalyzer::isBuiltinTypeName(const std::string &typeName) -> bool {
        return typeName == "int" ||
            typeName == "string" ||
            typeName == "bool" ||
            typeName == "void";
    }

    auto SemanticAnalyzer::isUnknownType(const SemanticType &type) -> bool {
        return type.kind == SemanticTypeKind::Unknown;
    }

    auto SemanticAnalyzer::isVoidType(const SemanticType &type) -> bool {
        return type.kind == SemanticTypeKind::Void;
    }

    auto SemanticAnalyzer::isIntType(const SemanticType &type) -> bool {
        return type.kind == SemanticTypeKind::Int;
    }

    auto SemanticAnalyzer::isStringType(const SemanticType &type) -> bool {
        return type.kind == SemanticTypeKind::String;
    }

    auto SemanticAnalyzer::isBoolType(const SemanticType &type) -> bool {
        return type.kind == SemanticTypeKind::Bool;
    }

    auto SemanticAnalyzer::typesEqual(const SemanticType &left, const SemanticType &right) -> bool {
        if (left.kind != right.kind) {
            return false;
        }

        if (left.kind == SemanticTypeKind::Struct) {
            return left.name == right.name;
        }

        if (left.kind == SemanticTypeKind::Array) {
            if (left.elementType == nullptr || right.elementType == nullptr) {
                return false;
            }

            return typesEqual(*left.elementType, *right.elementType);
        }

        if (left.kind == SemanticTypeKind::Map) {
            if (left.keyType == nullptr || right.keyType == nullptr || left.valueType == nullptr || right.valueType == nullptr) {
                return false;
            }

            return typesEqual(*left.keyType, *right.keyType) && typesEqual(*left.valueType, *right.valueType);
        }

        return true;
    }

    auto SemanticAnalyzer::semanticTypeToString(const SemanticType &type) -> std::string {
        switch (type.kind) {
            case SemanticTypeKind::Unknown:
                return "unknown";
            case SemanticTypeKind::Void:
                return "void";
            case SemanticTypeKind::Int:
                return "int";
            case SemanticTypeKind::String:
                return "string";
            case SemanticTypeKind::Bool:
                return "bool";
            case SemanticTypeKind::Struct:
                return type.name;
            case SemanticTypeKind::Array:
                if (type.elementType == nullptr) {
                    return "[]unknown";
                }

                return "[]" + semanticTypeToString(*type.elementType);
            case SemanticTypeKind::Map:
                if (type.keyType == nullptr || type.valueType == nullptr) {
                    return "map<unknown, unknown>";
                }

                return "map<" + semanticTypeToString(*type.keyType) + ", " + semanticTypeToString(*type.valueType) + ">";
        }

        return "unknown";
    }

    auto SemanticAnalyzer::analyzeStructLiteralExpressionType(
        const AST::StructLiteralExpression &expr
    ) -> SemanticType {
        const auto *structDecl = resolveUserDefinedType(expr.type);
        if (structDecl == nullptr) {
            _engine.error(
                "SEM038",
                "Unknown struct type '" + typeNameToString(expr.type) + "'.",
                expr.type.range
            );

            return {};
        }

        std::unordered_map<std::string, const AST::StructField*> declaredFields;
        for (const auto &field : structDecl->fields) {
            declaredFields.emplace(field.name, &field);
        }

        std::unordered_set<std::string> providedFields;
        for (const auto &literalField : expr.fields) {
            const auto [it, inserted] = providedFields.emplace(literalField.name);
            static_cast<void>(it);

            if (!inserted) {
                _engine.error(
                    "SEM039",
                    "Duplicate field '" + literalField.name + "' in struct literal '" + structDecl->name + "'.",
                    literalField.range
                );

                continue;
            }

            const auto declaredIt = declaredFields.find(literalField.name);
            if (declaredIt == declaredFields.end()) {
                _engine.error(
                    "SEM040",
                    "Unknown field '" + literalField.name + "' in struct literal '" + structDecl->name + "'.",
                    literalField.range
                );

                continue;
            }

            const auto expectedType = typeFromTypeName(declaredIt->second->type);
            const auto actualType = analyzeExpressionTypeWithExpected(*literalField.value, &expectedType);

            if (!isUnknownType(expectedType) && !isUnknownType(actualType) && !typesEqual(actualType, expectedType)) {
                _engine.error(
                    "SEM041",
                    "Struct field initializer type mismatch for field '" + literalField.name +
                    "'. Expected '" + semanticTypeToString(expectedType) +
                    "', actual '" + semanticTypeToString(actualType) + "'.",
                    literalField.range
                );
            }
        }

        for (const auto &field : structDecl->fields) {
            if (!providedFields.contains(field.name)) {
                _engine.error(
                    "SEM042",
                    "Missing field '" + field.name + "' in struct literal '" + structDecl->name + "'.",
                    expr.range
                );
            }
        }

        return SemanticType {
            .kind = SemanticTypeKind::Struct,
            .name = structDecl->name,
        };
    }

    auto SemanticAnalyzer::analyzeArrayLiteralExpressionType(
        const AST::ArrayLiteralExpression &expr,
        const SemanticType *expectedType) -> SemanticType {
        const SemanticType *expected = nullptr;
        if (expectedType != nullptr && expectedType->kind == SemanticTypeKind::Array && expectedType->elementType != nullptr) {
            expected = expectedType->elementType.get();
        }

        if (expr.elements.empty()) {
            if (expectedType != nullptr && expectedType->kind == SemanticTypeKind::Array && expectedType->elementType != nullptr) {
                return *expectedType;
            }

            _engine.error(
                "SEM049",
                "Cannot infer type of empty array literal.",
                expr.range
            );

            return {};
        }

        SemanticType elementType = analyzeExpressionTypeWithExpected(*expr.elements.front(), expected);
        if (isUnknownType(elementType)) {
            return {};
        }

        for (std::size_t idx = 1U; idx < expr.elements.size(); ++idx) {
            const auto currentType = analyzeExpressionTypeWithExpected(*expr.elements[idx], expected);

            if (!isUnknownType(currentType) && !typesEqual(currentType, elementType)) {
                _engine.error(
                    "SEM050",
                    "Array literal element type mismatch. Expected '" +
                    semanticTypeToString(elementType) +
                    "', actual '" +
                    semanticTypeToString(currentType) +
                    "'.",
                    expr.elements[idx]->range
                );
            }
        }

        SemanticType result {
            .kind = SemanticTypeKind::Array,
            .name = {},
            .elementType = std::make_shared<SemanticType>(std::move(elementType)),
        };

        if (expectedType != nullptr && expectedType->kind == SemanticTypeKind::Array && !typesEqual(result, *expectedType)) {
            _engine.error(
                "SEM050",
                "Array literal type mismatch. Expected '" +
                semanticTypeToString(*expectedType) +
                "', actual '" +
                semanticTypeToString(result) +
                "'.",
                expr.range
            );
        }

        return result;
    }

    auto SemanticAnalyzer::analyzeIndexExpressionType(const AST::IndexExpression &expr) -> SemanticType {
        const auto objectType = analyzeCheckedExpressionType(*expr.object);
        const auto indexType = analyzeCheckedExpressionType(*expr.index);

        if (isUnknownType(objectType)) {
            return {};
        }

        if (objectType.kind == SemanticTypeKind::Array) {
            if (!isUnknownType(indexType) && !isIntType(indexType)) {
                _engine.error(
                    "SEM052",
                    "Array index must be int, actual '" + semanticTypeToString(indexType) + "'.",
                    expr.index->range
                );

                return {};
            }

            if (objectType.elementType == nullptr) {
                return {};
            }

            return *objectType.elementType;
        }

        if (objectType.kind == SemanticTypeKind::Map) {
            if (!isUnknownType(indexType) && !isStringType(indexType)) {
                _engine.error(
                    "SEM061",
                    "Map index must be string, actual '" + semanticTypeToString(indexType) + "'.",
                    expr.index->range
                );

                return {};
            }

            if (objectType.valueType == nullptr) {
                return {};
            }

            return *objectType.valueType;
        }

        _engine.error(
            "SEM051",
            "Index target must be array or map, actual '" + semanticTypeToString(objectType) + "'.",
            expr.object->range
        );

        return {};
    }

    void SemanticAnalyzer::analyzeIndexAssignmentStatement(const AST::IndexAssignmentStatement &stmt) {
        const auto *rootName = rootNameExpression(*stmt.target);
        if (rootName == nullptr || rootName->name.segments.size() != 1U) {
            _engine.error(
                "SEM053",
                "Array element assignment target must start from a local variable.",
                stmt.target->range
            );

            static_cast<void>(analyzeCheckedExpressionType(*stmt.value));
            return;
        }

        const std::string &rootLocalName = rootName->name.segments.front();
        const auto *local = resolveLocal(rootLocalName);
        if (local == nullptr) {
            _engine.error(
                "SEM053",
                "Array element assignment target must start from a local variable.",
                rootName->range
            );

            static_cast<void>(analyzeCheckedExpressionType(*stmt.value));
            return;
        }

        if (!local->isMutable) {
            _engine.error(
                "SEM054",
                "Cannot assign array element through immutable local variable '" + rootLocalName + "'.",
                rootName->range
            );
        }

        if (containsMapIndexInAssignmentTarget(*stmt.target)) {
            _engine.error(
                "SEM062",
                "Map element assignment is not implemented yet.",
                stmt.target->range
            );

            static_cast<void>(analyzeCheckedExpressionType(*stmt.value));
            return;
        }

        const auto targetType = analyzeIndexExpressionType(*stmt.target);
        const auto valueType = analyzeExpressionTypeWithExpected(*stmt.value, &targetType);

        if (!isUnknownType(targetType) && !isUnknownType(valueType) && !typesEqual(targetType, valueType)) {
            _engine.error(
                "SEM055",
                "Array element assignment type mismatch. Expected '" +
                semanticTypeToString(targetType) +
                "', actual '" +
                semanticTypeToString(valueType) +
                "'.",
                stmt.range
            );
        }
    }

    auto SemanticAnalyzer::analyzeMapLiteralExpressionType(const AST::MapLiteralExpression &expr,
        const SemanticType *expected) -> SemanticType {
        const SemanticType *expectedValueType = nullptr;

        if (expected != nullptr && expected->kind == SemanticTypeKind::Map && expected->valueType != nullptr) {
            expectedValueType = expected->valueType.get();
        }

        if (expr.entries.empty()) {
            if (expected != nullptr && expected->kind == SemanticTypeKind::Map && expected->keyType != nullptr && expected->valueType != nullptr) {
                return *expected;
            }

            _engine.error(
                "SEM058",
                "Cannot infer type of empty map literal.",
                expr.range
            );

            return {};
        }

        std::unordered_set<std::string> keys;
        for (const auto &entry : expr.entries) {
            const auto [it, inserted] = keys.emplace(entry.key);
            static_cast<void>(it);

            if (!inserted) {
                _engine.error(
                    "SEM060",
                    "Duplicate key '" + entry.key + "' in map literal.",
                    entry.keyRange
                );
            }
        }

        SemanticType valueType = analyzeExpressionTypeWithExpected(*expr.entries.front().value, expectedValueType);
        if (isUnknownType(valueType)) {
            return {};
        }

        for (std::size_t idx = 1U; idx < expr.entries.size(); ++idx) {
            const auto currentType = analyzeExpressionTypeWithExpected(*expr.entries[idx].value, expectedValueType);

            if (!isUnknownType(currentType) && !typesEqual(currentType, valueType)) {
                _engine.error(
                    "SEM059",
                    "Map literal value type mismatch. Expected '" +
                    semanticTypeToString(valueType) +
                    "', actual '" +
                    semanticTypeToString(currentType) +
                    "'.",
                    expr.entries[idx].range
                );
            }
        }

        SemanticType result {
            .kind = SemanticTypeKind::Map,
            .name = {},
            .elementType = {},
            .keyType = std::make_shared<SemanticType>(SemanticType {
                .kind = SemanticTypeKind::String,
            }),
            .valueType = std::make_shared<SemanticType>(std::move(valueType)),
        };

        if (expected != nullptr && expected->kind == SemanticTypeKind::Map && !typesEqual(result, *expected)) {
            _engine.error(
                "SEM059",
                "Map literal type mismatch. Expected '" +
                semanticTypeToString(*expected) +
                "', actual '" +
                semanticTypeToString(result) +
                "'.",
                expr.range
            );
        }

        return result;
    }

    auto SemanticAnalyzer::analyzeExpressionTypeWithExpected(const AST::Expression &expr,
        const SemanticType *expected) -> SemanticType {
        if (expr.kind == AST::ExpressionKind::ArrayLiteral) {
            const auto &arrayLiteral = static_cast<const AST::ArrayLiteralExpression&>(expr);

            return analyzeArrayLiteralExpressionType(arrayLiteral, expected);
        }

        if (expr.kind == AST::ExpressionKind::MapLiteral) {
            const auto &mapLiteral = static_cast<const AST::MapLiteralExpression&>(expr);

            return analyzeMapLiteralExpressionType(mapLiteral, expected);
        }

        return analyzeCheckedExpressionType(expr);
    }

    auto SemanticAnalyzer::containsMapIndexInAssignmentTarget(const AST::IndexExpression &expr) -> bool {
        const auto objectType = analyzeCheckedExpressionType(*expr.object);
        if (!isUnknownType(objectType) && objectType.kind == SemanticTypeKind::Map) {
            return true;
        }

        if (expr.object->kind == AST::ExpressionKind::Index) {
            const auto &nestedIndex = static_cast<const AST::IndexExpression&>(*expr.object);
            return containsMapIndexInAssignmentTarget(nestedIndex);
        }

        return false;
    }
}
