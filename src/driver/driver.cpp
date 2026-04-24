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
            Semantic::SematicAnalyzer analyzer(*program, engine);
            [[maybe_unused]] auto ok = analyzer.analyze();
        }

        DriverResult result;
        result.diagnostics = engine.diagnostics();
        result.success = !engine.hasErrors() && program != nullptr;

        if (result.success) {
            IR::Lowerer lowerer;
            auto module = lowerer.lower(*program);

            Interpreter::Interpreter interpreter;
            [[maybe_unused]] auto ignored = interpreter.execute(module);

            AST::ASTPrinter printer;
            result.astText = printer.print(*program);
        }

        return result;
    }
}
