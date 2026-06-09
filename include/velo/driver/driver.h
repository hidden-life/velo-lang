#ifndef INC_VELO_DRIVER_DRIVER_H
#define INC_VELO_DRIVER_DRIVER_H

#include <string>
#include <vector>

#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/source/source_manager.h"

namespace Velo::Driver {
    enum class DriverMode {
        Run,
        Check,
        Ast,
        Ir,
        Bytecode,
    };

    struct DriverResult final {
        bool success {false};

        std::string astText {};
        std::string irText {};

        std::vector<Diagnostic::Diagnostic> diagnostics {};
        std::string error {};

        // Process exit code produced by the interpreter
        int exitCode {0};

        std::string bytecodeText {};
    };

    // Orchestrates the source -> lexer -> parser -> semantic -> IR -> VM pipeline.
    class Driver final {
    public:
        [[nodiscard]] auto parseFile(const std::string &path, DriverMode mode = DriverMode::Run) -> DriverResult;
        [[nodiscard]] auto parseText(std::string path, std::string content, DriverMode mode = DriverMode::Run) -> DriverResult;

    private:
        [[nodiscard]] auto runPipeline(const Source::SourceFile &source, DriverMode mode) -> DriverResult;

        Source::SourceManager _sourceManager {};
    };
}

#endif //INC_VELO_DRIVER_DRIVER_H
