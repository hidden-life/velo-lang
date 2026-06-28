#include "velo/http/http_handler.h"

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
        Velo::Source::SourceFile file("handler.velo", source);
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

    [[nodiscard]] auto makeRequest(
        std::string method,
        std::string path,
        std::string body = {}
    ) -> Velo::Runtime::HttpRequestValuePtr {
        auto request = std::make_shared<Velo::Runtime::HttpRequestValue>();
        request->method = std::move(method);
        request->path = std::move(path);
        request->body = std::move(body);

        return request;
    }
}

TEST(HttpHandlerTest, ValidatesConventionalHandlerSignature) {
    const auto compiled = compileProgram(R"(module app;
use std::http;

fn handle(req: http_request): http_response {
    return http::text_response(200, http::path(req));
}

fn main(): int {
    return 0;
}
)");

    ASSERT_TRUE(compiled.success);
    ASSERT_NE(compiled.program, nullptr);

    const auto validation = Velo::Http::validateHttpHandlerSignature(*compiled.program);

    EXPECT_TRUE(validation.isSuccess) << validation.error;
    EXPECT_TRUE(validation.error.empty());
}

TEST(HttpHandlerTest, ExecutesConventionalHandler) {
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
    ASSERT_NE(compiled.program, nullptr);

    const auto validation = Velo::Http::validateHttpHandlerSignature(*compiled.program);
    ASSERT_TRUE(validation.isSuccess) << validation.error;

    Velo::Runtime::Runtime runtime;
    Velo::Interpreter::Interpreter interpreter(runtime);

    const auto result = Velo::Http::executeHttpHandler(
        interpreter,
        compiled.module,
        makeRequest("GET", "/health")
    );

    ASSERT_TRUE(result.isSuccess) << result.error;
    ASSERT_NE(result.response, nullptr);
    EXPECT_EQ(result.response->status, 200);
    EXPECT_EQ(result.response->body, "/health");
    EXPECT_EQ(result.response->headers.at("Content-Type"), "text/plain");
}

TEST(HttpHandlerTest, ReportsMissingHandler) {
    const auto compiled = compileProgram(R"(module app;

fn main(): int {
    return 0;
}
)");

    ASSERT_TRUE(compiled.success);
    ASSERT_NE(compiled.program, nullptr);

    const auto validation = Velo::Http::validateHttpHandlerSignature(*compiled.program);

    EXPECT_FALSE(validation.isSuccess);
    EXPECT_NE(validation.error.find("was not found"), std::string::npos);
}

TEST(HttpHandlerTest, ReportsInvalidHandlerParameterCount) {
    const auto compiled = compileProgram(R"(module app;
use std::http;

fn handle(): http_response {
    return http::text_response(200, "OK");
}

fn main(): int {
    return 0;
}
)");

    ASSERT_TRUE(compiled.success);
    ASSERT_NE(compiled.program, nullptr);

    const auto validation = Velo::Http::validateHttpHandlerSignature(*compiled.program);

    EXPECT_FALSE(validation.isSuccess);
    EXPECT_NE(validation.error.find("exactly one parameter"), std::string::npos);
}

TEST(HttpHandlerTest, ReportsInvalidHandlerParameterType) {
    const auto compiled = compileProgram(R"(module app;
use std::http;

fn handle(req: string): http_response {
    return http::text_response(200, req);
}

fn main(): int {
    return 0;
}
)");

    ASSERT_TRUE(compiled.success);
    ASSERT_NE(compiled.program, nullptr);

    const auto validation = Velo::Http::validateHttpHandlerSignature(*compiled.program);

    EXPECT_FALSE(validation.isSuccess);
    EXPECT_NE(validation.error.find("http_request"), std::string::npos);
}

TEST(HttpHandlerTest, ReportsInvalidHandlerReturnType) {
    const auto compiled = compileProgram(R"(module app;

fn handle(req: http_request): int {
    return 0;
}

fn main(): int {
    return 0;
}
)");

    ASSERT_TRUE(compiled.success);
    ASSERT_NE(compiled.program, nullptr);

    const auto validation = Velo::Http::validateHttpHandlerSignature(*compiled.program);

    EXPECT_FALSE(validation.isSuccess);
    EXPECT_NE(validation.error.find("http_response"), std::string::npos);
}

TEST(HttpHandlerTest, ReportsRuntimeReturnTypeMismatch) {
    Velo::IR::Module module;

    Velo::IR::Function handler;
    handler.name = "handle";
    handler.parameters = {"req"};
    handler.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::PushInt,
        .intOperand = 42
    });
    handler.instructions.push_back(Velo::IR::Instruction {
        .code = Velo::IR::OpCode::Return
    });

    module.functions.push_back(std::move(handler));

    Velo::Runtime::Runtime runtime;
    Velo::Interpreter::Interpreter interpreter(runtime);

    const auto result = Velo::Http::executeHttpHandler(
        interpreter,
        module,
        makeRequest("GET", "/health")
    );

    EXPECT_FALSE(result.isSuccess);
    EXPECT_EQ(result.response, nullptr);
    EXPECT_NE(result.error.find("http_response"), std::string::npos);
}