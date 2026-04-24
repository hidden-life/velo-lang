#include "velo/driver/driver.h"

#include "velo/ast/ast_printer.h"
#include "velo/lexer/lexer.h"
#include "velo/parser/parser.h"
#include "velo/semantic/semantic_analyzer.h"
#include "velo/ir/lowerer.h"
#include "velo/interpreter/interpreter.h"

namespace Velo::Driver {
    auto Driver::parseFile(const std::string &path) -> DriverResult {
        const auto *file = _sourceManager.loadFromDisk(path);
        if (file == nullptr) {
            DriverResult result;
            result.error = "Failed to load source file: " + path;
            return result;
        }

        return runPipeline(*file);
    }

    auto Driver::parseText(std::string path, std::string content) -> DriverResult {
        const auto &file = _sourceManager.addVirtualFile(std::move(path), std::move(content));

        return runPipeline(file);
    }

    auto Driver::runPipeline(const Source::SourceFile &source) -> DriverResult {
        Diagnostic::DiagnosticEngine engine;
        Lexer::Lexer lexer(source, engine);
        Parser::Parser parser(lexer.lexAll(), engine);

        auto program = parser.parse();
        if (program != nullptr && !engine.hasErrors()) {
            Runtime::Runtime runtime;
            Semantic::SemanticAnalyzer analyzer(*program, engine, runtime.modules());
            [[maybe_unused]] auto ok = analyzer.analyze();
        }

        DriverResult result;
        result.diagnostics = engine.diagnostics();
        result.success = !engine.hasErrors() && program != nullptr;

        if (!result.success) {
            return result;
        }

        IR::Lowerer lowerer;
        const auto module = lowerer.lower(*program);
        Runtime::Runtime runtime;
        Interpreter::Interpreter interpreter(runtime);
        const auto execResult = interpreter.execute(module);
        if (!execResult.success) {
            result.success = false;
            result.error = execResult.error;

            return result;
        }

        AST::ASTPrinter printer;
        result.astText = printer.print(*program);

        return result;
    }
}
