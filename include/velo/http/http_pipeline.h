#ifndef INC_VELO_HTTP_PIPELINE_H
#define INC_VELO_HTTP_PIPELINE_H

#include <string>

#include "velo/interpreter/interpreter.h"
#include "velo/http/http_router.h"

namespace Velo::Http {
    struct HttpPipelineResult final {
        bool isSuccess {false};
        std::string raw {};
        std::string error {};
    };

    [[nodiscard]] auto handleRawHttpRequest(
        Interpreter::Interpreter &interpreter,
        const IR::Module &module,
        std::string_view rawRequest,
        std::string_view handlerName = "handle",
        const HttpRouteTable *table = nullptr
    ) -> HttpPipelineResult;
}

#endif //INC_VELO_HTTP_PIPELINE_H
