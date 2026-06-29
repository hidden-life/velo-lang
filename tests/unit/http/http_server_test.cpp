#include "velo/http/http_server.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {
    [[nodiscard]] auto writeTempVeloFile(
        const std::string &fileName,
        const std::string &content
    ) -> std::filesystem::path {
        const auto path = std::filesystem::temp_directory_path() / fileName;

        std::ofstream file(path);
        file << content;
        file.close();

        return path;
    }
}

TEST(HttpServerTest, ReportsInvalidPortBeforeLoadingSource) {
    Velo::Http::HttpServerConfig config;
    config.sourcePath = "missing.velo";
    config.port = 0;

    const auto result = Velo::Http::run(config);

    EXPECT_FALSE(result.isSuccess);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_NE(result.error.find("Invalid HTTP server port"), std::string::npos);
}

TEST(HttpServerTest, ReportsMissingSourceFile) {
    Velo::Http::HttpServerConfig config;
    config.sourcePath = "this_file_should_not_exist.velo";
    config.port = 18080;

    const auto result = Velo::Http::run(config);

    EXPECT_FALSE(result.isSuccess);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_NE(result.error.find("Failed to load source file"), std::string::npos);
}

TEST(HttpServerTest, ReportsMissingConventionalHandlerBeforeBindingSocket) {
    const auto path = writeTempVeloFile(
        "velo_http_server_missing_handler.velo",
        R"(module app;

fn main(): int {
    return 0;
}
)"
    );

    Velo::Http::HttpServerConfig config;
    config.sourcePath = path.string();
    config.port = 18081;

    const auto result = Velo::Http::run(config);

    EXPECT_FALSE(result.isSuccess);
    EXPECT_EQ(result.exitCode, 1);
    EXPECT_NE(result.error.find("HTTP handler 'handle' was not found"), std::string::npos);

    std::filesystem::remove(path);
}

TEST(HttpServerTest, ReportsSemanticDiagnosticsBeforeBindingSocket) {
    const auto path = writeTempVeloFile(
        "velo_http_server_semantic_error.velo",
        R"(module app;
use std::http;

fn handle(req: http_request): http_response {
    return http::text_response(200, unknown_name);
}

fn main(): int {
    return 0;
}
)"
    );

    Velo::Http::HttpServerConfig config;
    config.sourcePath = path.string();
    config.port = 18082;

    const auto result = Velo::Http::run(config);

    EXPECT_FALSE(result.isSuccess);
    EXPECT_EQ(result.exitCode, 1);
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().code(), "SEM007");

    std::filesystem::remove(path);
}