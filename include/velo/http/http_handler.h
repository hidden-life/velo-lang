#ifndef INC_VELO_HTTP_HANDLER_H
#define INC_VELO_HTTP_HANDLER_H

#include <string>

#include "velo/ast/ast.h"
#include "velo/interpreter/interpreter.h"
#include "velo/runtime/value.h"

namespace Velo::Http {
    struct HttpHandlerValidationResult final {
        bool isSuccess {false};
        std::string error {};
    };

    struct HttpHandlerExecutionResult final {
        bool isSuccess {false};
        Runtime::HttpResponseValuePtr response {};
        std::string error {};
    };

    [[nodiscard]] auto validateHttpHandlerSignature(
        const AST::Program &program,
        std::string_view handlerName = "handle") -> HttpHandlerValidationResult;

    [[nodiscard]] auto executeHttpHandler(
        Interpreter::Interpreter &interpreter,
        const IR::Module &module,
        Runtime::HttpRequestValuePtr request,
        std::string_view handlerName = "handle"
    ) -> HttpHandlerExecutionResult;
}

#endif //INC_VELO_HTTP_HANDLER_H
