#include <gtest/gtest.h>

#include  "velo/http/http_router.h"

namespace {
    [[nodiscard]] auto makeRouteFunction(
        std::string name,
        std::string annotationName,
        std::string path
    ) -> Velo::IR::Function {
        Velo::IR::Function function;
        function.name = std::move(name);
        function.annotations.push_back(Velo::IR::Annotation {
            .name = std::move(annotationName),
            .arguments = {
                Velo::IR::AnnotationArgument {
                    .kind = Velo::IR::AnnotationArgumentKind::StringLiteral,
                    .value = std::move(path),
                }
            }
        });

        return function;
    }
}

TEST(HttpRouterTest, BuildsRouteTableFromIrAnnotations) {
    Velo::IR::Module module;
    module.functions.push_back(makeRouteFunction("health", "http::get", "/health"));
    module.functions.push_back(makeRouteFunction("echo", "http::post", "/echo"));

    const auto result = Velo::Http::buildTable(module);

    ASSERT_TRUE(result.isSuccess) << result.error;
    ASSERT_EQ(result.routeTable.routes.size(), 2U);

    EXPECT_EQ(result.routeTable.routes[0].method, "GET");
    EXPECT_EQ(result.routeTable.routes[0].path, "/health");
    EXPECT_EQ(result.routeTable.routes[0].handlerName, "health");

    EXPECT_EQ(result.routeTable.routes[1].method, "POST");
    EXPECT_EQ(result.routeTable.routes[1].path, "/echo");
    EXPECT_EQ(result.routeTable.routes[1].handlerName, "echo");
}

TEST(HttpRouterTest, FindsRouteByMethodAndPath) {
    Velo::IR::Module module;
    module.functions.push_back(makeRouteFunction("health", "http::get", "/health"));

    const auto result = Velo::Http::buildTable(module);
    ASSERT_TRUE(result.isSuccess) << result.error;

    const auto *route = Velo::Http::findRoute(result.routeTable, "GET", "/health");

    ASSERT_NE(route, nullptr);
    EXPECT_EQ(route->handlerName, "health");

    EXPECT_EQ(Velo::Http::findRoute(result.routeTable, "POST", "/health"), nullptr);
    EXPECT_EQ(Velo::Http::findRoute(result.routeTable, "GET", "/missing"), nullptr);
}

TEST(HttpRouterTest, IgnoresNonHttpRouteAnnotations) {
    Velo::IR::Module module;

    Velo::IR::Function mainFunction;
    mainFunction.name = "main";
    mainFunction.annotations.push_back(Velo::IR::Annotation {
        .name = "auth",
        .arguments = {}
    });

    module.functions.push_back(std::move(mainFunction));

    const auto result = Velo::Http::buildTable(module);

    ASSERT_TRUE(result.isSuccess) << result.error;
    EXPECT_TRUE(result.routeTable.routes.empty());
    EXPECT_FALSE(Velo::Http::hasHttpRoutes(result.routeTable));
}

TEST(HttpRouterTest, ReportsDuplicateRoutesDefensively) {
    Velo::IR::Module module;
    module.functions.push_back(makeRouteFunction("first", "http::get", "/health"));
    module.functions.push_back(makeRouteFunction("second", "http::get", "/health"));

    const auto result = Velo::Http::buildTable(module);

    EXPECT_FALSE(result.isSuccess);
    EXPECT_NE(result.error.find("Duplicate HTTP route"), std::string::npos);
}