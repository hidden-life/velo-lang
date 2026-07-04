#include <iostream>

#include "velo/diagnostic/diagnostic.h"
#include "velo/driver/driver.h"
#include "velo/http/http_server.h"

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
        std::cerr << "    velo ir <source-file.velo>" << std::endl;
        std::cerr << "    velo bytecode <source-file.velo>" << std::endl;
        std::cerr << "    velo bc <source-file.velo>" << std::endl;
        std::cerr << "    velo serve <source-file.velo>" << std::endl;
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

        if (command == "ir") {
            return Velo::Driver::DriverMode::Ir;
        }

        if (command == "bytecode" || command == "bc") {
            return Velo::Driver::DriverMode::Bytecode;
        }

        return Velo::Driver::DriverMode::Run;
    }

    auto isKnownCommand(std::string_view command) -> bool {
        return command == "run" ||
            command == "check" ||
            command == "ast" ||
            command == "ir" ||
            command == "bytecode" ||
            command == "bc" ||
            command == "serve"
        ;
    }

    auto printResult(
        const Velo::Driver::DriverResult &result,
        bool printAst,
        bool printIr,
        bool printBytecode
    ) -> int {
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

        if (printIr && !result.irText.empty()) {
            std::cout << result.irText;
        }

        if (printBytecode && !result.bytecodeText.empty()) {
            std::cout << result.bytecodeText;
        }

        return result.exitCode;
    }

    auto printServerResult(
        const Velo::Http::HttpServerResult &result
    ) -> int {
        if (!result.error.empty()) {
            std::cerr << result.error << std::endl;
        }

        for (const auto &diag : result.diagnostics) {
            printDiagnostic(diag);
        }

        if (!result.isSuccess) {
            if (result.error.empty() && result.diagnostics.empty()) {
                std::cerr << "HTT server failed without error details." << std::endl;
            }
            
            return result.exitCode == 0 ? EXIT_FAILURE : result.exitCode;
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

        return printResult(result, false, false, false);
    }

    if (argc == 3) {
        const std::string_view command = argv[1];
        const std::string sourcePath = argv[2];

        if (!isKnownCommand(command)) {
            printUsage();
            return EXIT_FAILURE;
        }

        if (command == "serve") {
            Velo::Http::HttpServerConfig config;
            config.sourcePath = sourcePath;

            std::cerr << "Serving " << sourcePath << " on " << config.host << ":" << config.port << std::endl;
            const auto result = Velo::Http::run(config);

            return printServerResult(result);
        }

        const auto mode = parseMode(command);

        Velo::Driver::Driver driver;
        const auto result = driver.parseFile(sourcePath, mode);
        const bool shouldPrintAst = (mode == Velo::Driver::DriverMode::Ast);
        const bool shouldPrintIr = (mode == Velo::Driver::DriverMode::Ir);
        const bool shouldPrintBytecode = (mode == Velo::Driver::DriverMode::Bytecode);

        return printResult(result, shouldPrintAst, shouldPrintIr, shouldPrintBytecode);
    }

    printUsage();

    return EXIT_FAILURE;
}