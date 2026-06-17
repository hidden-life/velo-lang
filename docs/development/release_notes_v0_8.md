# Velo v0.8.0 release notes draft
Velo MVP 0.8 focuses on HTTP runtime foundation.

It adds HTTP request/response runtime values, a `std::http` module, response builders,
response access helpers, request helpers, HTTP/JSON integration examples, and a benchmark smoke
scaffold.

## Highlights
- runtime `http_request` value
- runtime `http_response` value
- `http_request` semantic type
- `http_response` semantic type
- `std::http` module
- `http::response`
- `http::json_response`
- `http::status`
- `http::body`
- `http::header`
- `http::has_header`
- `http::request`
- `http::method`
- `http::path`
- `http::request_body`
- `http::json_body`

## HTTP runtime values
MVP 0.8 adds runtime HTTP value representations.

HTTP request value:
```text
method: string
path: string
headers: map<string, string>
body: string
```

Velo types:
```velo
http_request
http_response
```

## Response builders
Response builders are available through `std::http`.
```velo
use std::http;
use std::json;

let raw: http_response = http::response(200, "OK");
let text: http_response = http::text_response(200, "Hello");
let jsonRes: http_response = http::json_response(201, json::parse("{\"ok\":true}"));
```

Signatures:
```text
http::response          int, string -> http_response
http::json_response     int, json   -> http_response
http::text_response     int, string -> http_response
```

Runtime behavior:
- `http::response` creates a raw response with status and body
- `http::text_response` sets `Content-Type` to `text/plain`
- `http::json_response` serializes a `json` value and sets `Content-Type` to `application/json`

## Response access helpers
```velo
let status: int = http::status(res);
let body: string = http::body(res);
let exists: bool = http::has_header(res, "Content-Type");
let contentType: string = http::header(res, "Content-Type");
```

Signatures:
```text
http::status        http_response -> int
http::body          http_response -> string
http::has_header    http_response, string -> bool
http::header        http_response, string -> string
```

Missing response headers are runtime errors for `http::header`.

Header lookup is case-sensitive in MVP 0.8.

## Request helpers
```velo
let req: http_request = http::request("POST", "/users", "{\"name\":\"Alex\"}");

let method: string = http::method(req);
let path: string = http::path(req);
let bodyText: string = http::request_body(req);
let body: json = http::json_body(req);
```

Signatures:
```text
http::request       string, string, string -> http_request
http::method        http_request -> string
http::path          http_request -> string
http::request_body  http_request -> string
http::json_body     http_request -> json
```

`http::json_body` parses request body using the runtime JSON parser.

Invalid JSON request body is a runtime error.

## HTTP/JSON flow
MVP 0.8 can model backend-like handler logic without a real server.

```velo
module app;

use std::http;
use std::json;
use std::string;

fn create_user(req: http_request): http_response {
    let body: json = http::json_body(req);
    let name: string = json::get_string(body, "name");

    if (string::len(name) == 0) {
        return http::json_response(400, json::parse("{\"error\":\"empty name\"}"));
    }

    return http::json_response(201, json::parse("{\"ok\":true}"));
}
```

## Examples
New examples:
```text
examples/http_response/main.velo
examples/http_response_access/main.velo
examples/http_request/main.velo
examples/http_json_flow/main.velo
```

## Benchmarks
MVP 0.8 adds a smoke benchmark scaffold:
```text
benchmarks/runtime/README.md
benchmarks/runtime/run_smoke.sh
docs/development/benchmark_plan.md
```

Run:
```bash
VELO_BIN=./build/debug/apps/velo/velo ITERATIONS=100 ./benchmarks/runtime/run_smoke.sh
```

Benchmark notes:
- no third-party benchmark tool is required
- no package installation is required
- no performance thresholds are enforced
- script measures end-to-end CLI execution time
- results include process startup overhead

## Tests
- MVP 0.8 adds tests for:
- HTTP runtime value cloning
- HTTP semantic types
- `std::http` module sync
- response builders
- response access helpers
- request helpers
- HTTP/JSON integration flows
- runtime errors for missing headers
- runtime errors for invalid JSON request bodies
- IR smoke tests
- bytecode smoke tests

## Known limitations
MVP 0.8 does not include:
- real HTTP server
- socket runtime
- routing
- route params
- query parameter parser
- request header helpers
- response mutation helpers
- cookies
- multipart body parsing
- streaming body
- middleware
- async/event loop
- TLS
- server benchmarks
- external benchmark tool integration
- CI performance thresholds

## Next
Suggested next milestone:
```text
MVP 0.9     HTTP server/router foundation
```