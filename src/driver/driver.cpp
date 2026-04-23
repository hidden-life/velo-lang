#include "velo/driver/driver.h"

#include "velo/ast/ast_printer.h"
#include "velo/lexer/lexer.h"
#include "velo/parser/parser.h"

namespace Velo::Driver {
    auto Driver::parseFile(const std::string &path) -> DriverResult {
        const auto *file = _sourceManager.loadFromDisk(path);
        if (file == nullptr) {
            DriverResult result;
            result.error = "Failed to load source file: " + path;
            return result;
        }

        return parseText(file->path(), file->content());
    }

    auto Driver::parseText(std::string path, std::string content) -> DriverResult {
        const auto &file = _sourceManager.addVirtualFile(std::move(path), std::move(content));

        Diagnostic::DiagnosticEngine engine;
        Lexer::Lexer lexer(file, engine);
        Parser::Parser parser(lexer.lexAll(), engine);

        auto program = parser.parse();
        DriverResult result;
        result.diagnostics = engine.diagnostics();
        result.success = !engine.hasErrors() && program != nullptr;

        if (result.success) {
            AST::ASTPrinter printer;
            result.astText = printer.print(*program);
        }

        return result;
    }
}
