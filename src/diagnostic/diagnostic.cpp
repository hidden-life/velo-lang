#include <velo/diagnostic/diagnostic.h>

namespace Velo::Diagnostic {
    Diagnostic::Diagnostic(
        DiagnosticSeverity severity,
        std::string code,
        std::string message,
        Source::SourceRange range) :
    _severity(severity),
    _code(std::move(code)),
    _message(std::move(message)),
    _range(range) {
    }

    auto Diagnostic::severity() const -> DiagnosticSeverity {
        return _severity;
    }

    auto Diagnostic::code() const -> std::string_view {
        return _code;
    }

    auto Diagnostic::message() const -> std::string_view {
        return _message;
    }

    auto Diagnostic::range() const -> const Source::SourceRange& {
        return _range;
    }
}
