#include "velo/http/http_pipeline.h"

#include "velo/diagnostic/diagnostic_engine.h"
#include "velo/interpreter/interpreter.h"
#include "velo/ir/lowerer.h"
#include "velo/lexer/lexer.h"
#include "velo/parser/parser.h"
#include "velo/runtime/runtime.h"
#include "velo/semantic/semantic_analyzer.h"
#include "velo/source/source_file.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
    struct CompiledProgram final {
        std::unique_ptr<Velo::AST::Program> program {};
        Velo::IR::Module module {};
        std::vector<Velo::Diagnostic::Diagnostic> diagnostics {};
        bool success {false};
    };

    [[nodiscard]] auto compileProgram(const std::string &source) -> CompiledProgram {
        Velo::Source::SourceFile file("pipeline.velo", source);
        Velo::Diagnostic::DiagnosticEngine engine;
        Velo::Lexer::Lexer lexer(file, engine);
        Velo::Parser::Parser parser(lexer.lexAll(), engine);

        auto program = parser.parse();
        if (program == nullptr || engine.hasErrors()) {
            return CompiledProgram {
                .program = std::move(program),
                .module = {},
                .diagnostics = engine.diagnostics(),
                .success = false
            };
        }

        Velo::Runtime::Runtime runtime;
        Velo::Semantic::SemanticAnalyzer analyzer(*program, engine, runtime.modules());
        const bool semanticOk = analyzer.analyze();

        if (!semanticOk || engine.hasErrors()) {
            return CompiledProgram {
                .program = std::move(program),
                .module = {},
                .diagnostics = engine.diagnostics(),
                .success = false
            };
        }

        Velo::IR::Lowerer lowerer;
        auto module = lowerer.lower(*program);

        return CompiledProgram {
            .program = std::move(program),
            .module = std::move(module),
            .diagnostics = engine.diagnostics(),
            .success = true
        };
    }
}

TEST(HttpPipelineTest, HandlesRawRequestWithConventionalHandler) {
    auto compiled = compileProgram(R"(module app;
use std::http;

fn handle(req: http_request): http_response {
    return http::text_response(200, http::path(req));
}

fn main(): int {
    return 0;
}
)");

    ASSERT_TRUE(compiled.success);

    Velo::Runtime::Runtime runtime;
    Velo::Interpreter::Interpreter interpreter(runtime);

    const auto result = Velo::Http::handleRawHttpRequest(
        interpreter,
        compiled.module,
        "GET /health HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    ASSERT_TRUE(result.isSuccess) << result.error;
    EXPECT_NE(result.raw.find("HTTP/1.1 200 OK\r\n"), std::string::npos);
    EXPECT_NE(result.raw.find("Content-Type: text/plain\r\n"), std::string::npos);
    EXPECT_NE(result.raw.find("\r\n\r\n/health"), std::string::npos);
}

TEST(HttpPipelineTest, PassesRequestBodyToHandler) {
    auto compiled = compileProgram(R"(module app;
use std::http;

fn handle(req: http_request): http_response {
    return http::text_response(201, http::request_body(req));
}

fn main(): int {
    return 0;
}
)");

    ASSERT_TRUE(compiled.success);

    Velo::Runtime::Runtime runtime;
    Velo::Interpreter::Interpreter interpreter(runtime);

    const auto result = Velo::Http::handleRawHttpRequest(
        interpreter,
        compiled.module,
        "POST /echo HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello"
    );

    ASSERT_TRUE(result.isSuccess) << result.error;
    EXPECT_NE(result.raw.find("HTTP/1.1 201 Created\r\n"), std::string::npos);
    EXPECT_NE(result.raw.find("Content-Length: 5\r\n"), std::string::npos);
    EXPECT_NE(result.raw.find("\r\n\r\nhello"), std::string::npos);
}

TEST(HttpPipelineTest, ReturnsBadRequestForMalformedRawRequest) {
    auto compiled = compileProgram(R"(module app;
use std::http;

fn handle(req: http_request): http_response {
    return http::text_response(200, "OK");
}

fn main(): int {
    return 0;
}
)");

    ASSERT_TRUE(compiled.success);

    Velo::Runtime::Runtime runtime;
    Velo::Interpreter::Interpreter interpreter(runtime);

    const auto result = Velo::Http::handleRawHttpRequest(
        interpreter,
        compiled.module,
        "BROKEN\r\n"
        "\r\n"
    );

    EXPECT_FALSE(result.isSuccess);
    EXPECT_NE(result.error.find("malformed"), std::string::npos);
    EXPECT_NE(result.raw.find("HTTP/1.1 400 Bad Request\r\n"), std::string::npos);
}

TEST(HttpPipelineTest, ReturnsInternalServerErrorForHandlerFailure) {
    Velo::IR::Module module;

    Velo::IR::Function mainFunction;
    mainFunction.name = "main";
    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::PushInt,
        .intOperand = 0
    });
    mainFunction.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::Return
    });

    module.functions.push_back(std::move(mainFunction));

    Velo::Runtime::Runtime runtime;
    Velo::Interpreter::Interpreter interpreter(runtime);

    const auto result = Velo::Http::handleRawHttpRequest(
        interpreter,
        module,
        "GET /health HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    EXPECT_FALSE(result.isSuccess);
    EXPECT_NE(result.error.find("handle"), std::string::npos);
    EXPECT_NE(result.raw.find("HTTP/1.1 500 Internal Server Error\r\n"), std::string::npos);
}