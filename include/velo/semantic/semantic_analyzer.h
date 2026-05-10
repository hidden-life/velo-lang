#ifndef INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H
#define INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H

#include <unordered_map>
#include <string>
#include <unordered_set>

#include "velo/ast/ast.h"
#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/module/module_registry.h"

namespace Velo::Semantic {
    enum class ExpressionType {
        Unknown,
        Void,
        Int,
        String,
        Bool,
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

        [[nodiscard]] auto analyzeExpressionType(const AST::Expression &expression) -> ExpressionType;

        [[nodiscard]] auto analyzeCheckedExpressionType(const AST::Expression &expression) -> ExpressionType;

        [[nodiscard]] auto typeFromTypeName(const AST::TypeName &typeName) -> ExpressionType;
        [[nodiscard]] auto analyzeCallExpressionType(const AST::CallExpression &callExpr) -> ExpressionType;

        [[nodiscard]] auto typeFromString(const std::string &typeName) -> ExpressionType;

        void validateDeclaredType(
            const AST::TypeName &typeName,
            bool allowVoid,
            const std::string &subject,
            bool allowUserDefinedTypes = false
        );
        [[nodiscard]] static auto typeNameToString(const AST::TypeName &typeName) -> std::string;

        [[nodiscard]] static auto importedModuleName(const AST::UseDeclaration &useDecl) -> std::string;

        [[nodiscard]] static auto statementGuaranteesReturn(const AST::Statement &statement) -> bool;

        void pushScope();
        void popScope();

        [[nodiscard]] static auto builtinParameterAcceptsType(const std::string &expected, ExpressionType actual) -> bool;

        [[nodiscard]] auto resolveUserDefinedType(const AST::TypeName &typeName) const -> const AST::StructDeclaration*;
        [[nodiscard]] static auto isBuiltinTypeName(const std::string &typeName) -> bool;

        const AST::Program &_program;
        Diagnostic::DiagnosticEngine &_engine;

        std::unordered_map<std::string, const AST::UseDeclaration*> _visibleImports {};
        std::unordered_map<std::string, const AST::StructDeclaration> _structs {};
        std::unordered_map<std::string, const AST::FunctionDeclaration*> _functions {};

        struct LocalSymbol final {
            ExpressionType type {ExpressionType::Unknown};
            bool isMutable {false};
        };
        // Scope of stack for local variables.
        std::vector<std::unordered_map<std::string, LocalSymbol>> _scopeStack {};

        const Module::ModuleRegistry &_modules;
        // Function parameters visible in the currently analyzed function.
        // Parameter name -> semantic type.
        std::unordered_map<std::string, ExpressionType> _currentParameters {};
        std::string _currentFunctionReturnType {};

        int _loopDepth {0};

        [[nodiscard]] auto declareLocal(const std::string &name, const LocalSymbol &symbol) -> bool;
        [[nodiscard]] auto resolveLocal(const std::string &name) const -> const LocalSymbol*;
    };
}

#endif //INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H
