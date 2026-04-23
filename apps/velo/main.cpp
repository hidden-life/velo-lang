#include <iostream>

#include "velo/diagnostic/diagnostic.h"
#include "velo/driver/driver.h"

#ifndef VELO_VERSION
#define VELO_VERSION "dev"
#endif

namespace {
    auto severityToString(Velo::Diagnostic::DiagnosticSeverity severity) -> std::string_view {
        switch (severity) {
            case Velo::Diagnostic::DiagnosticSeverity::Error:
                return "error";
            case Velo::Diagnostic::DiagnosticSeverity::Warning:
                return "warning";
            case Velo::Diagnostic::DiagnosticSeverity::Note:
                return "note";
        }

        return "unknown";
    }

    void printDiagnostic(const Velo::Diagnostic::Diagnostic &diagnostic) {
        const auto &range = diagnostic.range();

        std::cerr << severityToString(diagnostic.severity())
            << '[' << diagnostic.code() << "] "
            << range.begin().line() << ":" << range.begin().column()
            << "-"
            << range.end().line() << ":" << range.end().column()
            << " "
            << diagnostic.message()
            << std::endl;
    }

    void printUsage() {
        std::cerr << "velo " << VELO_VERSION << std::endl;
        std::cerr << "Usage: velo <source-file.velo>" << std::endl;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printUsage();
        return EXIT_FAILURE;
    }

    Velo::Driver::Driver driver;
    Velo::Driver::DriverResult result = driver.parseFile(argv[1]);

    if (!result.error.empty()) {
        std::cerr << result.error << std::endl;
        return EXIT_FAILURE;
    }

    for (const auto &diag : result.diagnostics) {
        printDiagnostic(diag);
    }

    if (!result.success) {
        return EXIT_FAILURE;
    }

    std::cout << result.astText;

    return EXIT_SUCCESS;
}