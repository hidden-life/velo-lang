# Velo MVP 0.8 status
MVP 0.8 focuses on HTTP runtime foundation.

The goal is to add HTTP request/response runtime values and standard HTTP helpers before introducing
a real HTTP server in a later milestone.

## Roadmap
```text
0.8.1   runtime http request/response values
0.8.2   std::http response builders
0.8.3   std::http response access helpers
0.8.4   std::http request builders and access helpers
0.8.5   http/json integration examples and micro-benchmark scaffold
0.8.6   docs/examples/release checklist
```

## Current step
```text
0.8.2   std::http response builders
```

## 0.8.1 scope
Implemented in this step:
- runtime HTTP request value
- runtime HTTP response value
- deep-copy support for HTTP request values
- deep-copy support for HTTP response values
- debug string output for HTTP values
- `http_request` semantic type
- `http_response` semantic type
- semantic tests
- runtime tests
- documentation

HTTP request runtime model:
```text
method: string
path: string
headers: map<string, string>
body: string
```

HTTP response runtime model:
```text
status: int
headers: map<string, string>
body: string
```

Supported semantic type names:
```velo
http_request
http_response
```

Example function declarations:
```velo
fn handle(req: http_request): http_response {
    return fallback(req);
}
```

Not implemented in 0.8.1:
- `std::http` builders
- `std::http` access helpers
- real HTTP server
- routing
- sockets
- benchmarks

## 0.8.2 scope
Implemented in this step:
- `std::http` module
- `http::response`
- `http::text_response`
- `http::json_response`
- response builder runtime implementation
- response builder semantic validation
- runtime module sync tests
- semantic tests
- driver tests
- example
- documentation

Response builder signatures:
```text
http::response          int, string -> http_response
http::text_response     int, string -> http_response
http::json_response     int, string -> http_response
```

Examples:
```velo
let raw: http_response = http::response(200, "OK");
let text: http_response = http::text_response(200, "Hello");
let jsonRes: http_response = http::json_response(201, json::parse("{\"ok\":true}"));
```

Current limitations:
- response access helpers are not implemented yet
- request builders are not implemented yet
- real HTTP server/runtime is not implemented yet