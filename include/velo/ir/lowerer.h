#ifndef INC_VELO_IR_LOWERER_H
#define INC_VELO_IR_LOWERER_H

#include <unordered_map>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>

#include "module.h"
#include "velo/ast/ast.h"

namespace Velo::IR {
    // Lowers AST into executable IR.
    class Lowerer final {
    public:
        [[nodiscard]] auto lower(const AST::Program &program) -> Module;

    private:
        Function lowerFunction(const AST::FunctionDeclaration &func);
        void lowerStatement(const AST::Statement &stmt, Function &func);
        void lowerExpression(const AST::Expression &expr, Function &func);

        [[nodiscard]] auto findLocalIndex(const std::string &name) const -> const std::size_t*;

        void collectModuleAliases(const AST::Program &program);
        [[nodiscard]] auto lowerQualifiedName(const AST::QualifiedName &name) const -> std::string;

        [[nodiscard]] auto lowerAnnotations(const std::vector<AST::Annotation> &annotations) const -> std::vector<Annotation>;
        [[nodiscard]] auto lowerAnnotation(const AST::Annotation &annotation) const -> Annotation;
        [[nodiscard]] static auto lowerAnnotationArgument(const AST::AnnotationArgument &arg) -> AnnotationArgument;

        [[nodiscard]] auto lowerTypeName(const AST::TypeName &typeName) const -> std::string;
        void lowerStructLiteralExpression(const AST::StructLiteralExpression &expr, Function &func);
        void lowerArrayLiteralExpression(const AST::ArrayLiteralExpression &expr, Function &func);
        void lowerMapLiteralExpression(const AST::MapLiteralExpression &expr, Function &func);
        void lowerFieldAccessExpression(const AST::FieldAccessExpression &expr, Function &func);
        void lowerIndexExpression(const AST::IndexExpression &expr, Function &func);

        void lowerFieldAssignmentStatement(const AST::FieldAssignmentStatement &stmt, Function &func);
        void lowerIndexAssignmentStatement(const AST::IndexAssignmentStatement &stmt, Function &func);

        void lowerLogicalAndExpression(const AST::BinaryExpression &expr, Function &func);
        void lowerLogicalOrExpression(const AST::BinaryExpression &expr, Function &func);

        void pushScope();
        void popScope();

        [[nodiscard]] auto declareLocal(const std::string &name) -> std::size_t;
        void lowerStatementBlock(const std::vector<std::unique_ptr<AST::Statement>> &statements, Function &func);

        std::vector<std::unordered_map<std::string, std::size_t>> _scopeStack {};
        std::size_t _nextLocalIndex {0U};

        // Visible module name -> runtime module name
        //
        // Examples:
        //      use std::string;        string -> string
        //      use std::string as str; str -> string
        std::unordered_map<std::string, std::string> _moduleAliases {};

        struct LoopContext {
            size_t conditionIndex {0};
            std::vector<size_t> breakJumps;
        };

        std::vector<LoopContext> _loopStack;
    };
}

#endif //INC_VELO_IR_LOWERER_H
