#include "velo/driver/driver.h"

#include "velo/ast/ast_printer.h"
#include "velo/lexer/lexer.h"
#include "velo/parser/parser.h"
#include "velo/semantic/semantic_analyzer.h"
#include "velo/ir/lowerer.h"
#include "velo/interpreter/interpreter.h"

namespace Velo::Driver {
    auto Driver::parseFile(const std::string &path, DriverMode mode) -> DriverResult {
        const auto *file = _sourceManager.loadFromDisk(path);
        if (file == nullptr) {
            DriverResult result;
            result.error = "Failed to load source file: " + path;
            result.success = false;
            result.exitCode = 1;

            return result;
        }

        return runPipeline(*file, mode);
    }

    auto Driver::parseText(std::string path, std::string content, DriverMode mode) -> DriverResult {
        const auto &file = _sourceManager.addVirtualFile(std::move(path), std::move(content));

        return runPipeline(file, mode);
    }

    auto Driver::runPipeline(const Source::SourceFile &source, DriverMode mode) -> DriverResult {
        Diagnostic::DiagnosticEngine engine;
        Lexer::Lexer lexer(source, engine);
        Parser::Parser parser(lexer.lexAll(), engine);

        auto program = parser.parse();
        DriverResult result;
        if (program == nullptr || engine.hasErrors()) {
            result.success = false;
            result.exitCode = 1;
            result.diagnostics = engine.diagnostics();

            return result;
        }

        Runtime::Runtime runtime;
        Semantic::SemanticAnalyzer analyzer(*program, engine, runtime.modules());
        const bool semanticOk = analyzer.analyze();
        result.diagnostics = engine.diagnostics();
        if (!semanticOk || engine.hasErrors()) {
            result.success = false;
            result.exitCode = 1;

            return result;
        }

        if (mode == DriverMode::Check) {
            result.success = true;
            result.exitCode = 0;

            return result;
        }

        if (mode == DriverMode::Ast) {
            AST::ASTPrinter printer;
            result.astText = printer.print(*program);
            result.success = true;
            result.exitCode = 0;

            return result;
        }

        IR::Lowerer lowerer;
        const auto module = lowerer.lower(*program);

        Interpreter::Interpreter interpreter(runtime);
        const auto execResult = interpreter.execute(module);

        result.exitCode = execResult.exitCode;

        if (!execResult.success) {
            result.success = false;
            // Runtime errors should produce non-zero process status.
            if (result.exitCode == 0) {
                result.exitCode = 1;
            }
            result.error = execResult.error;

            return result;
        }

        AST::ASTPrinter printer;
        result.astText = printer.print(*program);
        result.success = true;

        return result;
    }
}
