#include "velo/http/http_message.h"

#include <charconv>
#include <string_view>
#include <sstream>

namespace {
    [[nodiscard]] auto trimHttpWhitespace(std::string_view value) -> std::string_view {
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
            value.remove_prefix(1U);
        }

        while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r')) {
            value.remove_suffix(1U);
        }

        return value;
    }

    [[nodiscard]] auto stripTrailingCarriageReturn(std::string_view value) -> std::string_view {
        if (!value.empty() && value.back() == '\r') {
            value.remove_suffix(1U);
        }

        return value;
    }

    [[nodiscard]] auto parseContentLength(std::string_view value, std::size_t &outLength) -> bool {
        value = trimHttpWhitespace(value);
        if (value.empty()) {
            return false;
        }

        std::size_t parsed = 0U;
        const auto *begin = value.data();
        const auto *end = value.data() + value.size();
        const auto result = std::from_chars(begin, end, parsed);

        if (result.ec != std::errc {} || result.ptr != end) {
            return false;
        }

        outLength = parsed;
        return true;
    }

    [[nodiscard]] auto findHeaderBodySeparator(std::string_view raw) -> std::pair<std::size_t, std::size_t> {
        const auto crlfPos = raw.find("\r\n\r\n");
        if (crlfPos != std::string_view::npos) {
            return {crlfPos, 4U};
        }

        const auto lfPos = raw.find("\n\n");
        if (lfPos != std::string_view::npos) {
            return {lfPos, 2U};
        }

        return {std::string_view::npos, 0U};
    }

    [[nodiscard]] auto nextLine(std::string_view text, std::size_t &offset) -> std::string_view {
        if (offset >= text.size()) {
            return {};
        }

        const auto lineEnd = text.find('\n', offset);
        if (lineEnd == std::string_view::npos) {
            const auto line = text.substr(offset);
            offset = text.size();

            return stripTrailingCarriageReturn(line);
        }

        const auto line = text.substr(offset, lineEnd - offset);
        offset = lineEnd + 1U;

        return stripTrailingCarriageReturn(line);
    }
}

namespace Velo::Http {
    auto reasonPhraseForStatus(int status) -> std::string_view {
        switch (status) {
            case 200:
                return "OK";
            case 201:
                return "Created";
            case 204:
                return "No Content";
            case 400:
                return "Bad Request";
            case 404:
                return "Not Found";
            case 405:
                return "Method Not Allowed";
            case 500:
                return "Internal Server Error";
            default:
                return "OK";
        }
    }

    auto parseHttpRequest(std::string_view raw) -> HttpParseResult {
        const auto [headerEnd, separatorSize] = findHeaderBodySeparator(raw);
        if (headerEnd == std::string_view::npos) {
            return HttpParseResult {
                .isSuccess = false,
                .request = {},
                .error = "HTTP request is missing"
            };
        }

        const auto headerText = raw.substr(0U, headerEnd);
        const auto bodyText = raw.substr(headerEnd + separatorSize);
        std::size_t offset = 0U;
        const auto requestLine = nextLine(headerText, offset);

        if (requestLine.empty()) {
            return HttpParseResult {
                .isSuccess = false,
                .request = {},
                .error = "HTTP request line is empty."
            };
        }

        const auto methodEnd = requestLine.find(' ');
        if (methodEnd == std::string_view::npos) {
            return HttpParseResult {
                .isSuccess = false,
                .request = {},
                .error = "HTTP request line is malformed."
            };
        }

        const auto pathStart = methodEnd + 1U;
        const auto pathEnd = requestLine.find(' ', pathStart);
        if (pathEnd == std::string_view::npos) {
            return HttpParseResult {
                .isSuccess = false,
                .request = {},
                .error = "HTTP request line is missing HTTP version."
            };
        }

        const auto method = requestLine.substr(0U, methodEnd);
        const auto path = requestLine.substr(pathStart, pathEnd - pathStart);
        const auto version = requestLine.substr(pathEnd + 1U);

        if (method.empty() || path.empty()) {
            return HttpParseResult {
                .isSuccess = false,
                .request = {},
                .error = "HTTP request method or path is empty."
            };
        }

        if (version != "HTTP/1.1" && version != "HTTP/1.0") {
            return HttpParseResult {
                .isSuccess = false,
                .request = {},
                .error = "Unsupported HTTP version."
            };
        }

        auto request = std::make_shared<Runtime::HttpRequestValue>();
        request->method = std::string(method);
        request->path = std::string(path);

        bool hasContentLength = false;
        std::size_t contentLength = 0U;
        while (offset < headerText.size()) {
            auto line = nextLine(headerText, offset);
            if (line.empty()) {
                continue;
            }

            const auto colon = line.find(':');
            if (colon == std::string_view::npos) {
                return HttpParseResult {
                    .isSuccess = false,
                    .request = {},
                    .error = "HTTP header line is malformed."
                };
            }

            const auto key = trimHttpWhitespace(line.substr(0U, colon));
            const auto value = trimHttpWhitespace(line.substr(colon + 1U));
            if (key.empty()) {
                return HttpParseResult {
                    .isSuccess = false,
                    .request = {},
                    .error = "HTTP header name is empty."
                };
            }

            request->headers[std::string(key)] = std::string(value);
            if (key == "Content-Length") {
                hasContentLength = true;
                if (!parseContentLength(value, contentLength)) {
                    return HttpParseResult {
                        .isSuccess = false,
                        .request = {},
                        .error = "Invalid Content-Length header."
                    };
                }
            }
        }

        if (hasContentLength) {
            if (bodyText.size() < contentLength) {
                return HttpParseResult {
                    .isSuccess = false,
                    .request = {},
                    .error = "HTTP body is shorter than Content-Length."
                };
            }

            request->body = std::string(bodyText.substr(0U, contentLength));
        } else {
            request->body = std::string(bodyText);
        }

        return HttpParseResult {
            .isSuccess = true,
            .request = request,
            .error = {}
        };
    }

    auto serializeHttpResponse(const Runtime::HttpResponseValue &response) -> std::string {
        auto headers = response.headers;

        headers["Content-Length"] = std::to_string(response.body.size());
        if (headers.find("Connection") == headers.end()) {
            headers["Connection"] = "close";
        }

        std::ostringstream stream;

        stream << "HTTP/1.1 "
            << response.status
            << " "
            << reasonPhraseForStatus(response.status)
            << "\r\n"
        ;

        for (const auto &[key, value] : headers) {
            stream << key << ": " << value << "\r\n";
        }

        stream << "\r\n";
        stream << response.body;

        return stream.str();
    }
}
