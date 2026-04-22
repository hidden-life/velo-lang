#include <velo/diagnostic/diagnostic_engine.h>

namespace Velo::Diagnostic {
    void DiagnosticEngine::report(DiagnosticSeverity severity, std::string code, std::string message,
        Source::SourceRange range) {
        _diagnostics.emplace_back(severity, std::move(code), std::move(message), range);
    }

    void DiagnosticEngine::error(std::string code, std::string message, Source::SourceRange range) {
        report(DiagnosticSeverity::Error, std::move(code), std::move(message), range);
    }

    void DiagnosticEngine::warning(std::string code, std::string message, Source::SourceRange range) {
        report(DiagnosticSeverity::Warning, std::move(code), std::move(message), range);
    }

    void DiagnosticEngine::note(std::string code, std::string message, Source::SourceRange range) {
        report(DiagnosticSeverity::Note, std::move(code), std::move(message), range);
    }

    auto DiagnosticEngine::hasErrors() const -> bool {
        for (const auto &diag : _diagnostics) {
            if (diag.severity() == DiagnosticSeverity::Error) {
                return true;
            }
        }

        return false;
    }

    auto DiagnosticEngine::diagnostics() const -> const std::vector<Diagnostic>& {
        return _diagnostics;
    }

    auto DiagnosticEngine::size() const -> size_t {
        return _diagnostics.size();
    }
}
