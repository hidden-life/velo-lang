# Velo MVP 0.9 status
MVP 0.9 is complete.

MVP 0.9 adds the first HTTP server/router foundation for Velo.

The milestone turns the HTTP runtime values from MVP 0.8 into a minimal working
server mode.

```text
raw HTTP request
    -> parseHttpRequest
    -> execute handle(req)
    -> serializeHttpResponse
    -> raw HTTP response
```

## Roadmap
```text
0.9.1       HTTP parser/serializer foundation
0.9.2       handler signature validation and callable handler execution
0.9.3       CLI serve mode with conventional handle(req) function
0.9.4       basic routing helpers
0.9.5       server examples and local benchmark scripts
0.9.6       docs/examples/release checklist
```

## Final capabilities
MVP 0.9 supports:
- parsing raw HTTP requests into `http_request`
- serializing `http_response` into raw HTTP response text
- validating conventional HTTP handler signatures
- executing `handle(req: http_request): http_response`
- serving requests through `velo serve`
- simple routing helpers:
  - `http::is_method`
  - `http::is_route`
  - `http::is_path`
- local server examples
- local curl-based smoke benchmark scripts

Example handler:
```velo
module app;

use std::http;

fn handle(req: http_request): http_response {
    if (http::is_route(req, "GET", "/health")) {
        return http::text_response(200, "OK");
    }

    return http::text_response(404, "not found");
}

fn main(): int {
    return 0;
}
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

## 0.9.5 scope
Implemented in this step:
- HTTP JSON server example
- local HTTP smoke script
- local curl loop benchmark script
- benchmark README
- README/example documentation
- release checklist updates

The scripts intentionally use only common local tools:
```text
bash
curl
date
seq
```

Smoke:
```bash
bash benchmarks/http/run_smoke.sh
```

Curl loop:
```bash
bash benchmarks/http/run_curl_loop.sh
bash benchmarks/http/run_curl_loop.sh 100
```

## Limitations
MVP 0.9 intentionally does not include:
- `http::listen(...)`
- annotations
- route table registration
- path params
- query parser
- middleware
- cookies
- multipart
- TLS
- keep-alive
- chunked transfer encoding
- async runtime
- thread pool
- websocket
- production logging