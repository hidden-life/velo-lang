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
        std::cerr << "Usage:" << std::endl;
        std::cerr << "    velo <source-file.velo>" << std::endl;
        std::cerr << "    velo run <source-file.velo>" << std::endl;
        std::cerr << "    velo check <source-file.velo>" << std::endl;
        std::cerr << "    velo ast <source-file.velo>" << std::endl;
        std::cerr << "    velo --version" << std::endl;
        std::cerr << "    velo --help" << std::endl;
    }

    auto parseMode(std::string_view command) -> Velo::Driver::DriverMode {
        if (command == "check") {
            return Velo::Driver::DriverMode::Check;
        }

        if (command == "ast") {
            return Velo::Driver::DriverMode::Ast;
        }

        return Velo::Driver::DriverMode::Run;
    }

    auto printResult(const Velo::Driver::DriverResult &result, bool printAst) -> int {
        if (!result.error.empty()) {
            std::cerr << result.error << std::endl;
        }

        for (const auto &diag : result.diagnostics) {
            printDiagnostic(diag);
        }

        if (!result.success) {
            return result.exitCode == 0 ? EXIT_FAILURE : result.exitCode;
        }

        if (printAst && !result.astText.empty()) {
            std::cout << result.astText;
        }

        return result.exitCode;
    }
}

int main(int argc, char **argv) {
    if (argc == 2) {
        const std::string_view argument = argv[1];

        if (argument == "--help" || argument == "-h") {
            printUsage();
            return EXIT_SUCCESS;
        }

        if (argument == "--version" || argument == "-v") {
            std::cout << VELO_VERSION << std::endl;
            return EXIT_SUCCESS;
        }

        // Backward-compatible mode:
        //      velo file.velo
        // behaves like:
        //      velo run file.velo
        Velo::Driver::Driver driver;
        const auto result = driver.parseFile(std::string(argument), Velo::Driver::DriverMode::Run);

        return printResult(result, false);
    }

    if (argc == 3) {
        const std::string_view command = argv[1];
        const std::string sourcePath = argv[2];

        if (command != "run" && command != "check" && command != "ast") {
            printUsage();
            return EXIT_FAILURE;
        }

        const auto mode = parseMode(command);

        Velo::Driver::Driver driver;
        const auto result = driver.parseFile(sourcePath, mode);
        const bool shouldPrintAst = (mode == Velo::Driver::DriverMode::Ast);

        return printResult(result, shouldPrintAst);
    }

    printUsage();

    return EXIT_FAILURE;
}