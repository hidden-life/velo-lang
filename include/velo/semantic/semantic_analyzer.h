#ifndef INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H
#define INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H

#include <unordered_map>
#include <unordered_set>

#include "velo/ast/ast.h"
#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/module/module_registry.h"

namespace Velo::Semantic {
    // Minimal semantic analyzer for the first executable Velo program shape.
    class SemanticAnalyzer final {
    public:
        SemanticAnalyzer(const AST::Program &program, Diagnostic::DiagnosticEngine &engine, const Module::ModuleRegistry &modules);

        [[nodiscard]] auto analyze() -> bool;

    private:
        void collectImports();
        void collectFunctions();
        void validateEntryPoint();
        void analyzeFunction(const AST::FunctionDeclaration &func);
        void analyzeStatement(const AST::Statement &stmt);
        void analyzeExpression(const AST::Expression &expr);
        void resolveQualifiedName(const AST::QualifiedName &name, bool isCallable, std::size_t argsCount);

        [[nodiscard]] static auto visibleImportName(const AST::UseDeclaration &useDecl) -> std::string;
        [[nodiscard]] static auto isBuiltinInt(const AST::TypeName &typeName) -> bool;

        const AST::Program &_program;
        Diagnostic::DiagnosticEngine &_engine;

        std::unordered_map<std::string, const AST::UseDeclaration*> _visibleImports {};
        std::unordered_map<std::string, const AST::FunctionDeclaration*> _functions {};

        const Module::ModuleRegistry &_modules;
        std::unordered_set<std::string> _currentParameters {};
    };
}

#endif //INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H
