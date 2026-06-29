#ifndef INC_VELO_HTTP_SERVER_H
#define INC_VELO_HTTP_SERVER_H

#include <string>
#include <vector>

#include "velo/diagnostic/diagnostic.h"

namespace Velo::Http {
    struct HttpServerConfig final {
        std::string sourcePath {};
        std::string host {"127.0.0.1"};
        int port {8080};
        std::string handlerName {"handle"};
        std::size_t maxRequestBytes {1024U * 1024U};
        std::size_t maxConnections {0U};
    };

    struct HttpServerResult final {
        bool isSuccess {false};
        int exitCode {1};
        std::string error {};
        std::vector<Diagnostic::Diagnostic> diagnostics {};
    };

    [[nodiscard]] auto run(const HttpServerConfig &config) -> HttpServerResult;
}

#endif //INC_VELO_HTTP_SERVER_H
