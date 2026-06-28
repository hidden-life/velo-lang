#include "velo/http/http_handler.h"

namespace {
    [[nodiscard]] auto isNamedType(const Velo::AST::TypeName &typeName, std::string_view expected) -> bool {
        return typeName.kind == Velo::AST::TypeNameKind::Named &&
            typeName.arrayDepth == 0U &&
            typeName.name.segments.size() == 1U &&
            typeName.name.segments.front() == expected;
    }
}

namespace Velo::Http {
    auto validateHttpHandlerSignature(const AST::Program &program,
        std::string_view handlerName) -> HttpHandlerValidationResult {
        const auto it = std::ranges::find_if(
            program.functions,
            [handlerName](const AST::FunctionDeclaration &f) {
                return f.name == handlerName;
            }
        );

        if (it == program.functions.end()) {
            return HttpHandlerValidationResult {
                .isSuccess = false,
                .error = "HTTP handler '" + std::string(handlerName) + "' was not found."
            };
        }

        if (it->parameters.size() != 1U) {
            return HttpHandlerValidationResult {
                .isSuccess = false,
                .error = "HTTP handler '" + std::string(handlerName) + "' must accept exactly one parameter."
            };
        }

        if (!isNamedType(it->parameters.front().type, "http_request")) {
            return HttpHandlerValidationResult {
                .isSuccess = false,
                .error = "HTTP handler '" + std::string(handlerName) + "' parameter must be http_request."
            };
        }

        if (!isNamedType(it->returnType, "http_response")) {
            return HttpHandlerValidationResult {
                .isSuccess = false,
                .error = "HTTP handler '" + std::string(handlerName) + "' must return http_response."
            };
        }

        return HttpHandlerValidationResult {
            .isSuccess = true,
            .error = {},
        };
    }

    auto executeHttpHandler(Interpreter::Interpreter &interpreter, const IR::Module &module,
        Runtime::HttpRequestValuePtr request, std::string_view handlerName) -> HttpHandlerExecutionResult {
        if (request == nullptr) {
            return HttpHandlerExecutionResult {
                .isSuccess = false,
                .response = {},
                .error = "HTTP handler request must not be null."
            };
        }

        const auto result = interpreter.executeFunction(module, std::string(handlerName), {request});
        if (!result.success) {
            return HttpHandlerExecutionResult {
                .isSuccess = false,
                .response = {},
                .error = result.error
            };
        }

        if (!result.returnValue.has_value()) {
            return HttpHandlerExecutionResult {
                .isSuccess = false,
                .response = {},
                .error = "HTTP handler '" + std::string(handlerName) + "' did not return a value."
            };
        }

        if (!std::holds_alternative<Runtime::HttpResponseValuePtr>(*result.returnValue)) {
            return HttpHandlerExecutionResult {
                .isSuccess = false,
                .response = {},
                .error = "HTTP handler '" + std::string(handlerName) + "' must return http_response."
            };
        }

        const auto response = std::get<Runtime::HttpResponseValuePtr>(*result.returnValue);
        if (response == nullptr) {
            return HttpHandlerExecutionResult {
                .isSuccess = false,
                .response = {},
                .error = "HTTP handler '" + std::string(handlerName) + "' return a null http_response."
            };
        }

        return HttpHandlerExecutionResult {
            .isSuccess = true,
            .response = response,
            .error = {},
        };
    }
}
