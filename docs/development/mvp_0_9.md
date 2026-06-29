# Velo MVP 0.9 status
MVP 0.9 focuses on HTTP server/router foundation.

The goal is to turn the HTTP runtime values from MVP 0.8 into minimal real HTTP
serving pipeline.

## Roadmap
```text
0.9.1       HTTP parser/serializer foundation
0.9.2       handler signature validation and callable handler execution
0.9.3       CLI serve mode with conventional handle(req) function
0.9.4       basic routing helpers
0.9.5       server examples and local benchmark scripts
0.9.6       docs/examples/release checklist
```

## Current step
```text
0.9.3       CLI serve mode with conventional handle(req) function
```

## Architecture direction
MVP 0.9 starts with reusable HTTP core code.

The HTTP core should not depend on the CLI.

This allows both future entry points to use the same implementation:
```text
velo serve app.velo
http::listen(...)
```

The current milestone starts with:
```text
raw HTTP request text -> HttpRequestValuePtr
HttpResponseValue -> raw HTTP response text
```

## 0.9.1 scope
Implemented in this step:
- HTTP request parser foundation
- HTTP response serializer foundation
- HTTP reason phrase helper
- Content-Length parsing
- request headers parsing
- request body extraction
- response Content-Length generation
- default `Connection: close`
- unit tests
- documentation

Supported request parsing:
```text
METHOD PATH HTTP/1.1
Header: value

body
```

Supported response serialization:
```text
HTTP/1.1 STATUS REASON
Header: value
Content-Length: N
Connection: close

body
```

Current limitations:
- no socket server yet
- no `velo serve` yet
- no `http::listen(...)` yet
- no chunked transfer encoding
- no keep-alive
- no TLS
- no multipart parsing
- no query parser
- header names are case-sensitive

## 0.9.3-A scope
Implemented in this sub-step:
- raw HTTP request pipeline
- parser/handler/serializer composition
- `400 Bad Request` response for malformed raw requests
- `500 Internal Server Error` response for handler failures
- unit tests for pipeline behavior

The pipeline is:
```text
raw HTTP request
    -> parseHttpRequest(raw)
    -> executeHttpHandler(...)
    -> serializeHttpResponse(response)
    -> raw HTTP response
```

This sub-step does not start a socket server yet.

Current limitations:
- no socket server yet
- no `velo serve` command yet
- no routing helpers yet
- no keep-alive
- no chunked transfer encoding
- no TLS

## 0.9.3-B scope
Implemented in this sub-step:
- blocking HTTP socket server
- source file compilation during server startup
- conventional `handle(req)` validation before binding socket
- one request per connection
- request size limit
- serialized `400 Bad Request` response for incompleted socket reads
- reusable `HttpServerConfig`
- reusable `HttpServerResult`
- startup tests for invalid config/source/handler/semantic errors

The server loop is intentionally minimal:
```text
accept connection
    -> read one HTTP request
    -> handleRawHttpRequest(...)
    -> write one HTTP response
    -> close connection
```

Current defaults:
```text
host: 127.0.0.1
port: 8080
handler: handle
max request bytes: 1048576
```

Current limitations:
- no CLI command yet
- no `http::listen(...)`
- no keep-alive
- no chunked transfer encoding
- no TLS
- no async runtime
- no thread pool
- no routing table

## 0.9.3-C scope
Implemented in this sub-step:
- `velo serve <source-file.velo>` CLI command
- CLI integration with `HttpServerConfig`
- CLI error/diagnostic printing for server startup
- HTTP server example
- manual curl smoke flow
- README/docs examples updates

The command is:
```bash
./build/debug/apps/velo/velo serve ./examples/http_server_basic/main.velo
```

The default bind address is:
```text
127.0.0.1:8080
```

Manual smoke request:
```bash
curl -i http://127.0.0.1:8080/health
```

Expected response body:
```text
/health
```

Current limitations:
- no host flag yet
- no port flag yet
- no route table yet
- no `http::listen(...)` yet
- no keep-alive
- no chunked transfer encoding
- no TLS
- no thread pool

## 0.9.4 scope
Implemented in this step:
- `http::is_method(req, method): bool`
- `http::is_path(req, path): bool`
- `http::is_route(req, method, path): bool`
- route helper semantic tests
- route helper pipeline test
- route helper driver tests
- routing helper example
- documentation updates

The current routing style is explicit Velo code:
```velo
fn handle(req: http_request): http_response {
    if (http::is_route(req, "GET", "/health")) {
        return http::text_response(200, "OK");
    }

    return http::text_response(404, "not found");
}
```

This step intentionally does not add route tables or callback registration.

Current limitations:
- no `http::get(...)`
- no `http::post(...)`
- no path params
- no query parser
- no middleware
- no function references
- no `http::listen(...)`