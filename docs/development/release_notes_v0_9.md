# Velo MVP 0.9 release notes
Velo MVP 0.9 introduces the first HTTP server/router foundation.

The milestone focuses on making backend-style Velo programs executable through
a minimal local HTTP server.

## Highlights
- HTTP request parser
- HTTP response serializer
- conventional handler validation
- callable handler execution
- `velo serve`
- blocking local HTTP server
- HTTP routing helper builtins
- server examples
- curl-based local smoke benchmark scripts

## HTTP parser/serializer
MVP 0.9 adds reusable HTTP message helpers:
```text
parseHttpRequest(raw)
serializeHttpResponse(response)
reasonPhraseForStatus(status)
```

Supported request shape:
```text
METHOD PATH HTTP/1.1
Header: value

body
```

Supported response shape:
```text
HTTP/1.1 STATUS REASON
Header: value
Content-Length: N
Connection: close

body
```

## Conventional handler execution
A Velo server program defines:
```velo
fn handle(req: http_request): http_response {
    return http::text_response(200, "OK");
}
```

The runtime validates:
```text
handler exists
handler accepts exactly one http_request
handler returns http_response
```

## CLI serve mode
Run:
```bash
./build/debug/apps/velo/velo serve ./examples/http_routing_helpers/main.velo
```

Smoke:

```bash
curl -i http://127.0.0.1:8080/health
```

## Routing helpers

MVP 0.9 adds:

```velo
http::is_method(req, "GET")
http::is_path(req, "/health")
http::is_route(req, "GET", "/health")
```

Example:

```velo
fn handle(req: http_request): http_response {
    if (http::is_route(req, "GET", "/health")) {
        return http::text_response(200, "OK");
    }

    return http::text_response(404, "not found");
}
```

## Examples

Server examples:

```text
examples/http_server_basic/main.velo
examples/http_routing_helpers/main.velo
examples/http_server_json/main.velo
```

## Local smoke benchmark scripts

```bash
bash benchmarks/http/run_smoke.sh
bash benchmarks/http/run_curl_loop.sh
bash benchmarks/http/run_curl_loop.sh 100
```

These scripts are development smoke checks, not production benchmark targets.

## Known limitations

MVP 0.9 does not include:

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

## Suggested next milestones

```text
MVP 0.10  annotation foundation
MVP 0.11  annotation-based HTTP routing
```