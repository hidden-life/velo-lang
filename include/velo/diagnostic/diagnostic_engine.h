#ifndef INC_VELO_DIAGNOSTIC_DIAGNOSTIC_ENGINE_H
#define INC_VELO_DIAGNOSTIC_DIAGNOSTIC_ENGINE_H

#include <vector>

#include "diagnostic.h"

namespace Velo::Diagnostic {
    // Central collection point for compiler diagnostics
    class DiagnosticEngine final {
    public:
        DiagnosticEngine() = default;

        void report(
            DiagnosticSeverity severity,
            std::string code,
            std::string message,
            Source::SourceRange range
        );

        void error(std::string code, std::string message, Source::SourceRange range);
        void warning(std::string code, std::string message, Source::SourceRange range);
        void note(std::string code, std::string message, Source::SourceRange range);

        [[nodiscard]] auto hasErrors() const -> bool;
        [[nodiscard]] auto diagnostics() const -> const std::vector<Diagnostic>&;
        [[nodiscard]] auto size() const -> size_t;

    private:
        std::vector<Diagnostic> _diagnostics {};
    };
}

#endif //INC_VELO_DIAGNOSTIC_DIAGNOSTIC_ENGINE_H
