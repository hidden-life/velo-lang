#ifndef INC_VELO_HTTP_ROUTER_H
#define INC_VELO_HTTP_ROUTER_H

#include <string>
#include <vector>

#include "velo/ir/module.h"

namespace Velo::Http {
    struct HttpRoute final {
        std::string method {};
        std::string path {};
        std::string handlerName {};
    };

    struct HttpRouteTable final {
        std::vector<HttpRoute> routes {};
    };

    struct HttpRouteTableBuildResult final {
        bool isSuccess {false};
        HttpRouteTable routeTable {};
        std::string error {};
    };

    [[nodiscard]] auto buildTable(const IR::Module &module) -> HttpRouteTableBuildResult;

    [[nodiscard]] auto findRoute(const HttpRouteTable &table, std::string_view method, std::string_view path) -> const HttpRoute*;

    [[nodiscard]] auto hasHttpRoutes(const HttpRouteTable &table) -> bool;
}

#endif //INC_VELO_HTTP_ROUTER_H
