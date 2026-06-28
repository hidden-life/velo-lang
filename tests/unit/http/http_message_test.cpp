#include "velo/http/http_message.h"

#include <gtest/gtest.h>

TEST(HttpMessageTest, ParsesGetRequestWithoutBody) {
    const auto result = Velo::Http::parseHttpRequest(
        "GET /health HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    ASSERT_TRUE(result.isSuccess) << result.error;
    ASSERT_NE(result.request, nullptr);

    EXPECT_EQ(result.request->method, "GET");
    EXPECT_EQ(result.request->path, "/health");
    EXPECT_TRUE(result.request->body.empty());

    ASSERT_EQ(result.request->headers.size(), 1U);
    EXPECT_EQ(result.request->headers.at("Host"), "localhost");
}

TEST(HttpMessageTest, ParsesPostRequestWithContentLength) {
    const auto result = Velo::Http::parseHttpRequest(
        "POST /users HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 15\r\n"
        "\r\n"
        "{\"name\":\"Alex\"}"
    );

    ASSERT_TRUE(result.isSuccess) << result.error;
    ASSERT_NE(result.request, nullptr);

    EXPECT_EQ(result.request->method, "POST");
    EXPECT_EQ(result.request->path, "/users");
    EXPECT_EQ(result.request->body, "{\"name\":\"Alex\"}");

    EXPECT_EQ(result.request->headers.at("Host"), "localhost");
    EXPECT_EQ(result.request->headers.at("Content-Type"), "application/json");
    EXPECT_EQ(result.request->headers.at("Content-Length"), "15");
}

TEST(HttpMessageTest, ReportsMalformedRequestLine) {
    const auto result = Velo::Http::parseHttpRequest(
        "BROKEN\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    EXPECT_FALSE(result.isSuccess);
    EXPECT_EQ(result.request, nullptr);
    EXPECT_NE(result.error.find("malformed"), std::string::npos);
}

TEST(HttpMessageTest, ReportsInvalidContentLength) {
    const auto result = Velo::Http::parseHttpRequest(
        "POST /users HTTP/1.1\r\n"
        "Content-Length: nope\r\n"
        "\r\n"
        "{\"name\":\"Alex\"}"
    );

    EXPECT_FALSE(result.isSuccess);
    EXPECT_EQ(result.request, nullptr);
    EXPECT_NE(result.error.find("Content-Length"), std::string::npos);
}

TEST(HttpMessageTest, ReportsShortBodyForContentLength) {
    const auto result = Velo::Http::parseHttpRequest(
        "POST /users HTTP/1.1\r\n"
        "Content-Length: 100\r\n"
        "\r\n"
        "{\"name\":\"Alex\"}"
    );

    EXPECT_FALSE(result.isSuccess);
    EXPECT_EQ(result.request, nullptr);
    EXPECT_NE(result.error.find("shorter"), std::string::npos);
}

TEST(HttpMessageTest, SerializesResponseWithContentLengthAndConnection) {
    Velo::Runtime::HttpResponseValue response;
    response.status = 201;
    response.headers["Content-Type"] = "application/json";
    response.body = "{\"ok\":true}";

    const auto serialized = Velo::Http::serializeHttpResponse(response);

    EXPECT_NE(serialized.find("HTTP/1.1 201 Created\r\n"), std::string::npos);
    EXPECT_NE(serialized.find("Content-Type: application/json\r\n"), std::string::npos);
    EXPECT_NE(serialized.find("Content-Length: 11\r\n"), std::string::npos);
    EXPECT_NE(serialized.find("Connection: close\r\n"), std::string::npos);
    EXPECT_NE(serialized.find("\r\n\r\n{\"ok\":true}"), std::string::npos);
}

TEST(HttpMessageTest, SerializesKnownReasonPhrases) {
    EXPECT_EQ(Velo::Http::reasonPhraseForStatus(200), "OK");
    EXPECT_EQ(Velo::Http::reasonPhraseForStatus(201), "Created");
    EXPECT_EQ(Velo::Http::reasonPhraseForStatus(400), "Bad Request");
    EXPECT_EQ(Velo::Http::reasonPhraseForStatus(404), "Not Found");
    EXPECT_EQ(Velo::Http::reasonPhraseForStatus(500), "Internal Server Error");
}