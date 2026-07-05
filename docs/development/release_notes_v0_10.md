# Velo MVP 0.10 release notes
MVP 0.10 introduces function annotations and annotation-based HTTP route dispatch.

This milestone builds on MVP 0.9 HTTP server mode.

## Highlights
- function annotation syntax
- generic semantic validation for annotations
- annotation metadata propagation into IR
- HTTP route annotation validation
- `velo serve` dispatch through route annotations
- backward compatibility with conventional `handle(req)`
- permanent examples for annotation metadata and annotation routes

## Annotation syntax
Velo now supports function annotations:
```velo
@auth(true)
fn main(): int {
    return 0;
}
```

Qualified annotations are supported:
```velo
use std::http;

@http::get("/health")
fn health(req: http_request): http_response {
    return http::text_response(200, "OK");
}
```

Import aliases are normalized during lowering:
```velo
use std::http as web;

@web::get("/health")
fn health(req: http_request): http_response {
    return web::text_response(200, "OK");
}
```

The IR metadata stores the canonical annotation name:
```text
http::get
```

## HTTP route annotations
Supported route annotations:
```text
@http::get("/path")
@http::post("/path")
```

Handler signature:
```text
fn name(req: http_request): http_response
```

Example:
```velo
module app;

use std::http;

@http::get("/health")
fn health(req: http_request): http_response {
    return http::text_response(200, "OK");
}

@http::post("/echo")
fn echo(req: http_request): http_response {
    return http::text_response(201, http::request_body(req));
}

fn main(): int {
    return 0;
}
```

Run:
```bash
./build/debug/apps/velo/velo serve examples/http_annotation_routes/main.velo
```

Smoke:
```bash
curl -i http://127.0.0.1:8080/health
curl -i -X POST http://127.0.0.1:8080/echo -d 'hello'
curl -i http://127.0.0.1:8080/missing
```

Expected behavior:
```text
GET /health     ->  200 OK
POST /echo      ->  201 Created
GET /missing    ->  404 Not Found
```

## Backward compatibility
MVP 0.10 keeps MVP 0.9 server mode working.

Programs without HTTP route annotations still use the conventional handler:
```velo
fn handle(req: http_request): http_response {
    return http::text_response(200, "OK");
}
```

## Validation
The compiler validates HTTP route annotations before server runtime:
- route annotation must have exactly one argument
- route argument must be a string literal
- route path must start with `/`
- route handler must have exactly one parameter
- route handler parameter must be `http_request`
- route handler return type must be `http_response`
- duplicate method/path routes are rejected
- multiple HTTP route annotations on the same function are rejected

## Examples
```text
examples/annotation_metadata/main.velo
examples/http_annotation_routes/main.velo
```

## Current limitations
Not included in MVP 0.10:
- path parameters
- query parser
- middleware
- route groups
- route priority
- HEAD fallback to GET
- PUT/PATCH/DELETE annotations
- keep-alive
- chunked transfer encoding
- TLS
- async runtime
- thread pool
- bytecode annotation metadata
- custom annotation declarations