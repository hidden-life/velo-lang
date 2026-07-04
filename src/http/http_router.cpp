#include "velo/http/http_router.h"

#include <unordered_set>

namespace {
    [[nodiscard]] auto httpMethodFromAnnotationName(std::string_view annotationName) -> std::string {
        if (annotationName == "http::get") {
            return "GET";
        }

        if (annotationName == "http::post") {
            return "POST";
        }

        return {};
    }

    [[nodiscard]] auto isHttpRouteAnnotationName(std::string_view annotationName) -> bool {
        return annotationName == "http::get" || annotationName == "http::post";
    }

    [[nodiscard]] auto makeRouteKey(std::string_view method, std::string_view path) -> std::string {
        std::string key;
        key.reserve(method.size() + 1U + path.size());
        key.append(method);
        key.push_back(' ');
        key.append(path);

        return key;
    }
}

namespace Velo::Http {
    auto buildTable(const IR::Module &module) -> HttpRouteTableBuildResult {
        HttpRouteTable table;
        std::unordered_set<std::string> seenRoutes;

        for (const auto &func : module.functions) {
            for (const auto &annotation : func.annotations) {
                if (!isHttpRouteAnnotationName(annotation.name)) {
                    continue;
                }

                if (annotation.arguments.size() != 1U) {
                    return HttpRouteTableBuildResult {
                        .isSuccess = false,
                        .routeTable = {},
                        .error = "HTTP route annotation '" + annotation.name + "' must have exactly one path argument."
                    };
                }

                const auto &pathArg = annotation.arguments[0];
                if (pathArg.kind != IR::AnnotationArgumentKind::StringLiteral) {
                    return HttpRouteTableBuildResult {
                        .isSuccess = false,
                        .routeTable = {},
                        .error = "HTTP route annotation '" + annotation.name + "' path argument must be string."
                    };
                }

                if (pathArg.value.empty() || pathArg.value.front() != '/') {
                    return HttpRouteTableBuildResult {
                        .isSuccess = false,
                        .routeTable = {},
                        .error = "HTTP route annotation '" + annotation.name + "' path must start with '/'."
                    };
                }

                const std::string method = httpMethodFromAnnotationName(annotation.name);
                const std::string routeKey = makeRouteKey(method, pathArg.value);

                if (!seenRoutes.insert(routeKey).second) {
                    return HttpRouteTableBuildResult {
                        .isSuccess = false,
                        .routeTable = {},
                        .error = "Duplicate HTTP route '" + routeKey + "'."
                    };
                }

                table.routes.push_back(HttpRoute {
                    .method = method,
                    .path = pathArg.value,
                    .handlerName = func.name,
                });
            }
        }

        return HttpRouteTableBuildResult {
            .isSuccess = true,
            .routeTable = std::move(table),
            .error = {},
        };
    }

    auto findRoute(const HttpRouteTable &table, std::string_view method, std::string_view path) -> const HttpRoute* {
        for (const auto &route : table.routes) {
            if (route.method == method && route.path == path) {
                return &route;
            }
        }

        return nullptr;
    }

    auto hasHttpRoutes(const HttpRouteTable &table) -> bool {
        return !table.routes.empty();
    }
}
