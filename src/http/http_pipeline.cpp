#include "velo/runtime/value.h"
#include "velo/http/http_pipeline.h"

#include "velo/http/http_handler.h"
#include "velo/http/http_message.h"

namespace {
    [[nodiscard]] auto makePlainTextResponse(int status, const std::string &body) -> Velo::Runtime::HttpResponseValue {
        Velo::Runtime::HttpResponseValue response;
        response.status = status;
        response.headers["Content-Type"] = "text/plain";
        response.body = body;

        return response;
    }
}

namespace Velo::Http {
    auto handleRawHttpRequest(
        Interpreter::Interpreter &interpreter,
        const IR::Module &module,
        std::string_view rawRequest,
        std::string_view handlerName,
        const HttpRouteTable *table
        ) -> HttpPipelineResult {
        const auto parseResult = parseHttpRequest(rawRequest);
        if (!parseResult.isSuccess) {
            auto response = makePlainTextResponse(400, parseResult.error);

            return HttpPipelineResult {
                .isSuccess = false,
                .raw = serializeHttpResponse(response),
                .error = parseResult.error,
            };
        }

        std::string selectedHandler(handlerName);
        if (table != nullptr && hasHttpRoutes(*table)) {
            const auto *route = findRoute(*table, parseResult.request->method, parseResult.request->path);
            if (route == nullptr) {
                auto response = makePlainTextResponse(404, "HTTP route was not found.");

                return HttpPipelineResult {
                    .isSuccess = false,
                    .raw = serializeHttpResponse(response),
                    .error = "HTTP route was not found."
                };
            }

            selectedHandler = route->handlerName;
        }

        const auto handlerResult = executeHttpHandler(interpreter, module, parseResult.request, selectedHandler);
        if (!handlerResult.isSuccess) {
            auto response = makePlainTextResponse(500, handlerResult.error);

            return HttpPipelineResult {
                .isSuccess = false,
                .raw = serializeHttpResponse(response),
                .error = handlerResult.error,
            };
        }

        return HttpPipelineResult {
            .isSuccess = true,
            .raw = serializeHttpResponse(*handlerResult.response),
            .error = {},
        };
    }
}
