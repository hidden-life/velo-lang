#include <charconv>
#include <cstring>
#include <sys/socket.h>

#include "velo/http/http_server.h"

#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/http/http_handler.h"
#include "velo/http/http_message.h"
#include "velo/http/http_pipeline.h"
#include "velo/ir/lowerer.h"
#include "velo/ir/module.h"
#include "velo/lexer/lexer.h"
#include "velo/parser/parser.h"
#include "velo/runtime/runtime.h"
#include "velo/semantic/semantic_analyzer.h"
#include "velo/source/source_manager.h"

namespace {
    struct CompiledServerProgram final {
        bool isSuccess {false};
        Velo::IR::Module module {};
        std::string error {};
        std::vector<Velo::Diagnostic::Diagnostic> diagnostics {};
    };

    struct SocketReadResult final {
        bool isSuccess {false};
        std::string request {};
        std::string error {};
    };

    [[nodiscard]] auto findHeaderBodySeparator(std::string_view raw) -> std::pair<std::size_t, std::size_t> {
        const auto crlfPos = raw.find("\r\n\r\n");
        if (crlfPos != std::string_view::npos) {
            return {crlfPos, 4U};
        }

        const auto lfPos = raw.find("\n\n");
        if (lfPos != std::string_view::npos) {
            return {lfPos, 2U};
        }

        return {std::string_view::npos, 0U};
    }

    [[nodiscard]] auto trimHttpWhitespace(std::string_view value) -> std::string_view {
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
            value.remove_prefix(1U);
        }

        while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r')) {
            value.remove_suffix(1U);
        }

        return value;
    }

    [[nodiscard]] auto contentLengthFromHeaders(std::string_view text) -> std::size_t {
        std::size_t offset = 0U;

        while (offset < text.size()) {
            const auto lineEnd = text.find('\n', offset);
            auto line = lineEnd == std::string_view::npos ? text.substr(offset) : text.substr(offset, lineEnd - offset);
            offset = lineEnd == std::string_view::npos ? text.size() : lineEnd + 1U;
            if (!line.empty() && line.back() == '\r') {
                line.remove_suffix(1U);
            }

            const auto colon = line.find(':');
            if (colon == std::string_view::npos) {
                continue;
            }

            const auto name = trimHttpWhitespace(line.substr(0U, colon));
            if (name != "Content-Length") {
                continue;
            }

            const auto value = trimHttpWhitespace(line.substr(colon + 1U));
            std::size_t parsed = 0U;
            const auto *begin = value.data();
            const auto *end = value.data() + value.size();
            const auto result = std::from_chars(begin, end, parsed);

            if (result.ec != std::errc {} || result.ptr != end) {
                return 0U;
            }

            return parsed;
        }

        return 0U;
    }

    [[nodiscard]] auto hasCompleteRequest(std::string_view raw) -> bool {
        const auto [headerEnd, separatorSize] = findHeaderBodySeparator(raw);
        if (headerEnd == std::string_view::npos) {
            return false;
        }

        const auto headerText = raw.substr(0U, headerEnd);
        const auto contentLength = contentLengthFromHeaders(headerText);

        return raw.size() >= headerEnd + separatorSize + contentLength;
    }

    [[nodiscard]] auto compileServerProgram(
        const Velo::Http::HttpServerConfig &config,
        Velo::Source::SourceManager &sourceManager,
        Velo::Runtime::Runtime &runtime
    ) -> CompiledServerProgram {
        if (config.sourcePath.empty()) {
            return CompiledServerProgram {
                .isSuccess = false,
                .module = {},
                .error = "HTTP server source path is empty.",
                .diagnostics = {},
            };
        }

        const auto *source = sourceManager.loadFromDisk(config.sourcePath);
        if (source == nullptr) {
            return CompiledServerProgram {
                .isSuccess = false,
                .module = {},
                .error = "Failed to load source file: " + config.sourcePath,
                .diagnostics = {},
            };
        }

        Velo::Diagnostic::DiagnosticEngine engine;
        Velo::Lexer::Lexer lexer(*source, engine);
        Velo::Parser::Parser parser(lexer.lexAll(), engine);

        auto program = parser.parse();
        if (program == nullptr || engine.hasErrors()) {
            return CompiledServerProgram {
                .isSuccess = false,
                .module = {},
                .error = {},
                .diagnostics = engine.diagnostics(),
            };
        }

        Velo::Semantic::SemanticAnalyzer analyzer(*program, engine, runtime.modules());
        const bool semanticOk = analyzer.analyze();

        if (!semanticOk || engine.hasErrors()) {
            return CompiledServerProgram {
                .isSuccess = false,
                .module = {},
                .error = {},
                .diagnostics = engine.diagnostics(),
            };
        }

        const auto validation = Velo::Http::validateHttpHandlerSignature(*program, config.handlerName);
        if (!validation.isSuccess) {
            return CompiledServerProgram {
                .isSuccess = false,
                .module = {},
                .error = validation.error,
                .diagnostics = {},
            };
        }

        Velo::IR::Lowerer lowerer;
        auto module = lowerer.lower(*program);

        return CompiledServerProgram {
            .isSuccess = true,
            .module = std::move(module),
            .error = {},
            .diagnostics = {},
        };
    }

    [[nodiscard]] auto readHttpRequestFromSocket(int id, std::size_t maxBytes) -> SocketReadResult {
        if (maxBytes == 0U) {
            return SocketReadResult {
                .isSuccess = false,
                .request = {},
                .error = "HTTP request byte limit must be greater than zero.",
            };
        }

        std::string request;
        char buffer[4096] {};

        while (request.size() < maxBytes) {
            const auto bytesRead = ::recv(id, buffer, sizeof(buffer), 0);

            if (bytesRead < 0) {
                return SocketReadResult {
                    .isSuccess = false,
                    .request = {},
                    .error = "Failed to read HTTP request from socket: " + std::string(std::strerror(errno)),
                };
            }

            if (bytesRead == 0) {
                if (hasCompleteRequest(request)) {
                    return SocketReadResult {
                        .isSuccess = true,
                        .request = std::move(request),
                        .error = {},
                    };
                }

                return SocketReadResult {
                    .isSuccess = false,
                    .request = {},
                    .error = "HTTP request was closed before a complete request was received."
                };
            }

            request.append(buffer, static_cast<std::size_t>(bytesRead));
            if (hasCompleteRequest(request)) {
                return SocketReadResult {
                    .isSuccess = true,
                    .request = std::move(request),
                    .error = {},
                };
            }
        }

        return SocketReadResult {
            .isSuccess = false,
            .request = {},
            .error = "HTTP request was closed before a complete request was received."
        };
    }

    [[nodiscard]] auto sendAll(int id, std::string_view data) -> bool {
        std::size_t sent = 0U;

        while (sent < data.size()) {
            const auto bytesSent = ::send(id, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
            if (bytesSent <= 0) {
                return false;
            }

            sent += static_cast<std::size_t>(bytesSent);
        }

        return true;
    }

    [[nodiscard]] auto makeErrorResponseText(int status, const std::string &msg) -> std::string {
        Velo::Runtime::HttpResponseValue response;
        response.status = status;
        response.headers["Content-Type"] = "text/plain";
        response.body = msg;

        return Velo::Http::serializeHttpResponse(response);
    }
}

namespace Velo::Http {
    auto run(const HttpServerConfig &config) -> HttpServerResult {
        if (config.port <= 0 || config.port > 65535) {
            return HttpServerResult {
                .isSuccess = false,
                .exitCode = 1,
                .error = "Invalid HTTP server port: " + std::to_string(config.port),
                .diagnostics = {},
            };
        }

        if (config.maxRequestBytes == 0U) {
            return HttpServerResult {
                .isSuccess = false,
                .exitCode = 1,
                .error = "Invalid HTTP server max request bytes: 0",
                .diagnostics = {},
            };
        }

        Source::SourceManager sourceManager;
        Runtime::Runtime runtime;

        auto compiled = compileServerProgram(config, sourceManager, runtime);
        if (!compiled.isSuccess) {
            return HttpServerResult {
                .isSuccess = false,
                .exitCode = 1,
                .error = compiled.error,
                .diagnostics = compiled.diagnostics,
            };
        }

        const int serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd < 0) {
            return HttpServerResult {
                .isSuccess = false,
                .exitCode = 1,
                .error = "Failed to create HTTP server socket: " + std::string(std::strerror(errno)),
                .diagnostics = {},
            };
        }

        int reuseAddress = 1;
        static_cast<void>(::setsockopt(
            serverFd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuseAddress,
            sizeof(reuseAddress)
        ));

        sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<uint16_t>(config.port));

        if (::inet_pton(AF_INET, config.host.c_str(), &address.sin_addr) != 1) {
            ::close(serverFd);

            return HttpServerResult {
                .isSuccess = false,
                .exitCode = 1,
                .error = "Invalid HTTP server host: " + config.host,
                .diagnostics = {},
            };
        }

        if (::bind(serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
            const std::string err = std::strerror(errno);
            ::close(serverFd);

            return HttpServerResult {
                .isSuccess = false,
                .exitCode = 1,
                .error = "Failed to bind HTTP server socket: " + err,
                .diagnostics = {},
            };
        }

        if (::listen(serverFd, 16) < 0) {
            const std::string err = std::strerror(errno);
            ::close(serverFd);

            return HttpServerResult {
                .isSuccess = false,
                .exitCode = 1,
                .error = "Failed to listen on HTTP server socket: " + err,
                .diagnostics = {},
            };
        }

        Interpreter::Interpreter interpreter(runtime);
        std::size_t handledConnections = 0U;

        while (config.maxConnections == 0U || handledConnections < config.maxConnections) {
            const int clientFd = ::accept(serverFd, nullptr, nullptr);
            if (clientFd < 0) {
                const std::string err = std::strerror(errno);
                ::close(serverFd);

                return HttpServerResult {
                    .isSuccess = false,
                    .exitCode = 1,
                    .error = "Failed to accept HTTP client connection: " + err,
                    .diagnostics = {},
                };
            }

            const auto readResult = readHttpRequestFromSocket(clientFd, config.maxRequestBytes);
            if (!readResult.isSuccess) {
                const auto response = makeErrorResponseText(400, readResult.error);
                static_cast<void>(sendAll(clientFd, response));
                ::close(clientFd);

                ++handledConnections;
                continue;
            }

            const auto pipelineResult = handleRawHttpRequest(interpreter, compiled.module, readResult.request, config.handlerName);
            static_cast<void>(sendAll(clientFd, pipelineResult.raw));
            ::close(clientFd);

            ++handledConnections;
        }

        ::close(serverFd);

        return HttpServerResult {
            .isSuccess = true,
            .exitCode = 0,
            .error = {},
            .diagnostics = {},
        };
    }
}
