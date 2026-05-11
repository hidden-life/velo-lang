#ifndef INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H
#define INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H

#include <unordered_map>
#include <string>
#include <unordered_set>

#include "velo/ast/ast.h"
#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/module/module_registry.h"

namespace Velo::Semantic {
    enum class SemanticTypeKind {
        Unknown,
        Void,
        Int,
        String,
        Bool,
        Struct,
    };

    struct SemanticType final {
        SemanticTypeKind kind {SemanticTypeKind::Unknown};
        // Used only when kind == SemanticKindType::Struct
        std::string name {};
    };

    // Minimal semantic analyzer for the first executable Velo program shape.
    class SemanticAnalyzer final {
    public:
        SemanticAnalyzer(const AST::Program &program, Diagnostic::DiagnosticEngine &engine, const Module::ModuleRegistry &modules);

        [[nodiscard]] auto analyze() -> bool;

    private:
        void collectImports();
        void collectStructs();
        void collectFunctions();
        void validateEntryPoint();
        void analyzeStruct(const AST::StructDeclaration &structDecl);
        void analyzeFunction(const AST::FunctionDeclaration &func);
        void analyzeStatement(const AST::Statement &stmt);
        void analyzeExpression(const AST::Expression &expr);
        void resolveQualifiedName(const AST::QualifiedName &name, bool isCallable, std::size_t argsCount);

        [[nodiscard]] static auto visibleImportName(const AST::UseDeclaration &useDecl) -> std::string;
        [[nodiscard]] static auto isBuiltinInt(const AST::TypeName &typeName) -> bool;

        [[nodiscard]] auto analyzeExpressionType(const AST::Expression &expression) -> SemanticType;

        [[nodiscard]] auto analyzeCheckedExpressionType(const AST::Expression &expression) -> SemanticType;

        [[nodiscard]] auto typeFromTypeName(const AST::TypeName &typeName) -> SemanticType;
        [[nodiscard]] auto analyzeCallExpressionType(const AST::CallExpression &callExpr) -> SemanticType;

        [[nodiscard]] auto typeFromString(const std::string &typeName) -> SemanticType;

        [[nodiscard]] static auto isUnknownType(const SemanticType &type) -> bool;
        [[nodiscard]] static auto isVoidType(const SemanticType &type) -> bool;
        [[nodiscard]] static auto isIntType(const SemanticType &type) -> bool;
        [[nodiscard]] static auto isStringType(const SemanticType &type) -> bool;
        [[nodiscard]] static auto isBoolType(const SemanticType &type) -> bool;
        [[nodiscard]] static auto typesEqual(const SemanticType &left, const SemanticType &right) -> bool;
        [[nodiscard]] static auto semanticTypeToString(const SemanticType &type) -> std::string;

        void validateDeclaredType(
            const AST::TypeName &typeName,
            bool allowVoid,
            const std::string &subject
        );
        [[nodiscard]] static auto typeNameToString(const AST::TypeName &typeName) -> std::string;

        [[nodiscard]] static auto importedModuleName(const AST::UseDeclaration &useDecl) -> std::string;

        [[nodiscard]] static auto statementGuaranteesReturn(const AST::Statement &statement) -> bool;

        void pushScope();
        void popScope();

        [[nodiscard]] static auto builtinParameterAcceptsType(const std::string &expected, const SemanticType &actual) -> bool;

        [[nodiscard]] auto resolveUserDefinedType(const AST::TypeName &typeName) const -> const AST::StructDeclaration*;
        [[nodiscard]] static auto isBuiltinTypeName(const std::string &typeName) -> bool;

        const AST::Program &_program;
        Diagnostic::DiagnosticEngine &_engine;

        std::unordered_map<std::string, const AST::UseDeclaration*> _visibleImports {};
        std::unordered_map<std::string, const AST::StructDeclaration> _structs {};
        std::unordered_map<std::string, const AST::FunctionDeclaration*> _functions {};

        struct LocalSymbol final {
            SemanticType type {};
            bool isMutable {false};
        };
        // Scope of stack for local variables.
        std::vector<std::unordered_map<std::string, LocalSymbol>> _scopeStack {};

        const Module::ModuleRegistry &_modules;
        // Function parameters visible in the currently analyzed function.
        // Parameter name -> semantic type.
        std::unordered_map<std::string, SemanticType> _currentParameters {};
        SemanticType _currentFunctionReturnType {};

        int _loopDepth {0};

        [[nodiscard]] auto declareLocal(const std::string &name, const LocalSymbol &symbol) -> bool;
        [[nodiscard]] auto resolveLocal(const std::string &name) const -> const LocalSymbol*;
    };
}

#endif //INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H
