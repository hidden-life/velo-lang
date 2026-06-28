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