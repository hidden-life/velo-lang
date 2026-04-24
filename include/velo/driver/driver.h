#ifndef INC_VELO_DRIVER_DRIVER_H
#define INC_VELO_DRIVER_DRIVER_H

#include <string>
#include <vector>

#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/source/source_manager.h"

namespace Velo::Driver {
    struct DriverResult final {
        bool success {false};
        std::string astText {};
        std::vector<Diagnostic::Diagnostic> diagnostics {};
        std::string error {};
    };

    // The first orchestration layer for the source -> lexer -> parser -> semantic -> AST print pipeline.
    class Driver final {
    public:
        [[nodiscard]] auto parseFile(const std::string &path) -> DriverResult;
        [[nodiscard]] auto parseText(std::string path, std::string content) -> DriverResult;

    private:
        [[nodiscard]] auto runPipeline(const Source::SourceFile &source) -> DriverResult;

        Source::SourceManager _sourceManager {};
    };
}

#endif //INC_VELO_DRIVER_DRIVER_H
