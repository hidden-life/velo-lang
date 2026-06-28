#ifndef INC_VELO_HTTP_MESSAGE_H
#define INC_VELO_HTTP_MESSAGE_H

#include "velo/runtime/value.h"

namespace Velo::Http {
    struct HttpParseResult final {
        bool isSuccess {false};
        Runtime::HttpRequestValuePtr request {};
        std::string error {};
    };

    [[nodiscard]] auto parseHttpRequest(std::string_view raw) -> HttpParseResult;

    [[nodiscard]] auto serializeHttpResponse(const Runtime::HttpResponseValue &response) -> std::string;

    [[nodiscard]] auto reasonPhraseForStatus(int status) -> std::string_view;
}

#endif //INC_VELO_HTTP_MESSAGE_H
