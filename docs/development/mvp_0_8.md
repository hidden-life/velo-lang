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
0.8.6   docs/examples/release checklist
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

## 0.8.3 scope
Implemented in this step:
- `http::status`
- `http::body`
- `http::has_header`
- `http::header`
- response access helper runtime implementation
- response access helper semantic validation
- runtime module sync tests
- semantic tests
- driver tests
- example
- documentation

Response access helper signatures:
```text
http::status        http_response -> int
http::body          http_response -> string
http::has_header    http_response, string -> bool
http::header        http_response, string -> string
```

Example:
```velo
let res: http_response = http::text_response(200, "Hello");

let status: int = http::status(res);
let body: string = http::body(res);
let contentType: string = http::header(res, "Content-Type");
```

## 0.8.4 scope
Implemented in this step:
- `http::request`
- `http::method`
- `http::path`
- `http::request_body`
- `http::json_body`
- request builder runtime implementation
- request access helper runtime implementation
- JSON request body parsing
- runtime module sync tests
- semantic tests
- driver tests
- example
- documentation

Request helper signatures:
```text
http::request      string, string, string -> http_request
http::method       http_request -> string
http::path         http_request -> string
http::request_body http_request -> string
http::json_body    http_request -> json
```

Example:
```velo
let req: http_request = http::request("POST", "/users", "{\"name\":\"Alex\"}");
let body: json = http::json_body(req);
let name: string = json::get_string(body, "name");
```

## 0.8.5 scope
Implemented in this step:
- handler-like HTTP/JSON flow example
- semantic tests for handler-like flow
- driver tests for handler-like flow
- driver checks for response status/body/content-type
- IR smoke test for HTTP/JSON builtin calls
- bytecode smoke test for HTTP/JSON builtin calls
- runtime benchmark scaffold
- benchmark plan documentation
- documentation updates

Example flow:
```velo
fn create_user(req: http_request): http_response {
    let body: json = http::json_body(req);
    let name: string = json::get_string(body, "name");

    if (string::len(name) == 0) {
        return http::json_response(400, json::parse("{\"error\":\"empty name\"}"));
    }

    return http::json_response(201, json::parse("{\"ok\":true}"));
}
```

Benchmark scaffold:
```text
benchmarks/runtime/run_smoke.sh
benchmarks/runtime/README.md
docs/development/benchmark_plan.md
```

Benchmark scaffold notes:
- no third-party benchmark dependency is required
- script uses standard shell tool
- no performance thresholds are enforced
- results are manual smoke measurements, no CI gates

## 0.8.6 scope
Implemented in this step:
- MVP 0.8 documentation review
- examples documentation review
- release checklist update
- release notes draft
- README current status update
- architecture docs synchronization
- language docs synchronization
- benchmark plan review
- smoke-test command list
- tag instructions

## MVP 0.8 completed feature set
MVP 0.8 includes:
- runtime HTTP request values
- runtime HTTP response values
- `http_request` semantic type
- `http_response` semantic type
- `std::http` module
- `http::response`
- `http::text_response`
- `http::json_response`
- `http::status`
- `http::body`
- `http::has_header`
- `http::header`
- `http::request`
- `http::method`
- `http::path`
- `http::request_body`
- `http::json_body`
- HTTP/JSON handler-like flow examples
- runtime benchmark smoke scaffold
- benchmark plan documentation

## MVP 0.8 examples
```text
examples/http_response/main.velo
examples/http_response_access/main.velo
examples/http_request/main.velo
examples/http_json_flow/main.velo
```

## MVP 0.8 benchmark files
```text
benchmarks/runtime/README.md
benchmarks/runtime/run_smoke.sh
docs/development/benchmark_plan.md
```

## MVP 0.8 limitations
Not implemented in MVP 0.8:
- real HTTP server
- socket runtime
- routing
- route params
- query parameter parser
- request header builders/access helpers
- response mutation helpers
- cookies
- multipart body parsing
- streaming body
- middleware
- async/event loop
- TLS
- server benchmarks
- external benchmark tool integration
- performance thresholds in tests

## Dependencies
MVP 0.8 does not add third-party runtime libraries.

The benchmark smoke script uses standard shell tools available on typical Linux systems:
```text
bash
date
seq
chmod
```

No package installation is required for MVP 0.8.

## Next milestone
Suggested next milestone:
```text
MVP 0.9     HTTP server/router foundation
```

Reason:
- MVP 0.8 gives HTTP value foundation
- MVP 0.8 gives HTTP/JSON handler-like flows
- the next logical step is mapping real HTTP requests to handler functions