#ifndef INC_VELO_DIAGNOSTIC_DIAGNOSTIC_H
#define INC_VELO_DIAGNOSTIC_DIAGNOSTIC_H

#include <string>

#include "velo/source/source_range.h"

namespace Velo::Diagnostic {
    enum class DiagnosticSeverity {
        Error,
        Warning,
        Note,
    };

    // A single compiler diagnostic message
    class Diagnostic final {
    public:
        Diagnostic(DiagnosticSeverity severity, std::string code, std::string message, Source::SourceRange range);

        [[nodiscard]] auto severity() const -> DiagnosticSeverity;
        [[nodiscard]] auto code() const -> std::string_view;
        [[nodiscard]] auto message() const -> std::string_view;
        [[nodiscard]] auto range() const -> const Source::SourceRange&;

    private:
        DiagnosticSeverity _severity {DiagnosticSeverity::Error};
        std::string _code;
        std::string _message;
        Source::SourceRange _range {};
    };
}

#endif //INC_VELO_DIAGNOSTIC_DIAGNOSTIC_H
