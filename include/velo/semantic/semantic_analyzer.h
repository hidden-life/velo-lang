#ifndef INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H
#define INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H

#include <unordered_map>

#include "velo/ast/ast.h"
#include "velo/diagnostic/diagnostic_engine.h"

namespace Velo::Semantic {
    // Minimal semantic analyzer for the first executable Velo program shape.
    class SematicAnalyzer final {
    public:
        SematicAnalyzer(const AST::Program &program, Diagnostic::DiagnosticEngine &engine);

        [[nodiscard]] auto analyze() -> bool;

    private:
        void collectImports();
        void collectFunctions();
        void validateEntryPoint();
        void analyzeFunction(const AST::FunctionDeclaration &func);
        void analyzeStatement(const AST::Statement &stmt);
        void analyzeExpression(const AST::Expression &expr);
        void resolveQualifiedName(const AST::QualifiedName &name, bool isCallable);

        [[nodiscard]] static auto visibleImportName(const AST::UseDeclaration &useDecl) -> std::string;
        [[nodiscard]] static auto isBuiltinInt(const AST::TypeName &typeName) -> bool;

        const AST::Program &_program;
        Diagnostic::DiagnosticEngine &_engine;

        std::unordered_map<std::string, const AST::UseDeclaration*> _visibleImports {};
        std::unordered_map<std::string, const AST::FunctionDeclaration*> _functions {};
    };
}

#endif //INC_VELO_SEMANTIC_SEMATIC_ANALYZER_H
