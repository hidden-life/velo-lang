# Examples

This document lists useful examples available under `examples/`.

## Guidelines
Example style is documented here:
- [Example guidelines](development/example_guidelines.md)

General rule:
- valid demo examples should return `0`
- non-zero return values should usually be asserted in driver tests

## Run
Demo examples should usually exit with code `0`.
```bash
./build/debug/apps/velo/velo run ./examples/arithmetic/main.velo
```

## Check

```bash
./build/debug/apps/velo/velo check ./examples/arithmetic/main.velo
```

## Print AST

```bash
./build/debug/apps/velo/velo ast ./examples/arithmetic/main.velo
```

## Print IR

```bash
./build/debug/apps/velo/velo ir ./examples/arithmetic/main.velo
```

## MVP showcase
```bash
./build/debug/apps/velo/velo run ./examples/mvp_showcase/main.velo
```

## Current examples

### Arithmetic

```text
examples/arithmetic/main.velo
```

Demonstrates:

- arithmetic operators
- grouped expressions
- local variables
- `console::println`

### Logical operators

```text
examples/logical/main.velo
```

Demonstrates:

- comparison operators
- logical operators
- `if`

### Short-circuit

```text
examples/short_circuit/main.velo
```

Demonstrates:

- short-circuit `&&`
- short-circuit `||`
- avoiding runtime evaluation of the right-hand side

### While loop

```text
examples/while_loop/main.velo
```

Demonstrates:

- mutable locals
- `while`
- assignment

### Break / continue

```text
examples/break_continue/main.velo
```

Demonstrates:

- `break`
- `continue`
- loop control flow

### Typed parameters

```text
examples/typed_parameters/main.velo
```

Demonstrates:

- typed function parameters
- `string` parameters
- `bool` parameters
- user-defined function calls

### Standard string module

```text
examples/std_string_len/main.velo
```

Demonstrates:

- `std::string`
- import alias
- `str::len`

### Standard toString modules

```text
examples/std_to_string/main.velo
```

Demonstrates:

- `std::int::toString`
- `std::bool::toString`
- printing converted values

### Error examples

```text
examples/errors/
```

Contains intentionally invalid programs used to demonstrate diagnostics.

### Struct declarations
```text
examples/struct_declaration/main.velo
```
Demonstrates:
- `struct`
- `pub struct`
- public fields
- private fields
- field type validation

### User-defined type registry
```text
examples/user_defined_type_registry/main.velo
```
Demonstrates:
- struct field using another struct type
- user-defined type registry
- public struct with user-defined field type

### Struct type usage
```text
examples/struct_type_usage/main.velo
```
Demonstrates:
- struct type in function parameter
- struct type as function return type
- struct type in local declaration
- semantic checking of named struct types

### Struct literals
```text
examples/struct_literal/main.velo
```
Demonstrates:
- struct literal syntax
- struct value creation
- passing struct values to functions
- returning struct values from functions

### Field access
```text
examples/field_access/main.velo
```
Demonstrates:
- struct literal creation
- reading `int`, `string`, and `bool` fields
- field access expressions
- printing field values with `console::println`

### Field assignment
```text
examples/field_assignment/main.velo
```

Demonstrates:

- assigning to struct fields
- assigning to `int` fields
- assigning to `string` fields
- preserving struct value semantics when assigning through a copied struct value
- printing original and modified values with `console::println`

### MVP showcase
```text
examples/mvp_showcase/main.velo
```
Demonstrates the current core language feature set:
- imports
- aliases
- user functions
- typed parameters
- typed return values
- mutable locals
- while loop
- if
- comparisons
- logical operators
- standard modules

### Equality

```text
examples/equality/main.velo
```

Demonstrates:

- string equality
- string inequality
- bool equality
- bool inequality
- comparing struct fields of type `string`
- comparing struct fields of type `bool`
- printing bool results with `console::println`

### Block scopes
```text
examples/block_scopes/main.velo
```

Demonstrates:

- local shadowing inside `if`
- local shadowing inside `while`
- block-local variables not overwriting outer local variables
- outer local visibility after nested blocks

### Array literals
```text
examples/array_literal/main.velo
```

Demonstrates:
- array literals
- arrays of `int`
- arrays of `string`
- arrays of `bool`
- arrays of struct values
- runtime array value printing

### Array indexing
```text
examples/array_indexing/main.velo
```

Demonstrates:
- reading array elements by index
- indexing arrays of `int`
- indexing arrays of struct values
- field access after array indexing
- nested array indexing

### Array assignment

```text
examples/array_assignment/main.velo
```

Demonstrates:

- array element assignment
- nested array element assignment
- assigning struct values into arrays
- preserving array value semantics after copying arrays

### Array builtins
```text
examples/array_builtin/main.velo
```

Demonstrates:

- importing `std::array`
- `array::len`
- length of `int` arrays
- length of `string` arrays
- length of struct arrays

### Map literals
```text
examples/map_literal/main.velo
```

Demonstrates:
- map literal expressions
- empty map literals with expected type
- maps of primitive values
- maps of struct values
- runtime map value printing

Expected output:
```text
<map len=2>
<map len=2>
<map len=0>
```

### Map indexing
```text
examples/map_indexing/main.velo
```

Demonstrates:
- map index read expressions
- string map keys
- field access after map indexing
- array indexing after map indexing

Expected output:
```text
20
Bob
4
```

### Map assignment
```text
examples/map_assignment/main.velo
```

Demonstrates:
- map element assignment
- map element insert assignment
- mixed map/array index assignment
- nested map assignment

Expected output:
```text
42
30
99
77
```

### Map builtins

```text
examples/map_builtin/main.velo
```

Demonstrates:

- importing `std::map`
- `map::len`
- map length for primitive maps
- map length for struct maps
- map length for empty maps
- map length after insert assignment

Expected output:

```text
2
2
0
3
```

## MVP 0.7 JSON examples
### JSON stringify primitives
```text
examples/json_stringify_primitives/main.velo
```

Demonstrates:
- importing `std::json`
- `json::stringify`
- primitive JSON serialization
- JSON string output

Expected output:
```text
42
"Alex"
true
false
```

### JSON stringify collections
```text
examples/json_stringify_collections/main.velo
```

Demonstrates:
- array JSON serialization
- map JSON serialization
- nested collection JSON serialization

Expected output:
```text
[1,2,3]
["Alex","Bob"]
[true,false]
{"Alex":10,"Bob":20}
{"a":[1,2],"b":[3,4]}
```

### JSON stringify structs

```text
examples/json_stringify_struct/main.velo
```

Demonstrates:

- struct JSON serialization
- array of structs JSON serialization
- map with struct values JSON serialization

Expected output:

```text
{"active":true,"id":1,"name":"Alex"}
[{"active":true,"id":1,"name":"Alex"},{"active":false,"id":2,"name":"Bob"}]
{"alex":{"active":true,"id":1,"name":"Alex"},"bob":{"active":false,"id":2,"name":"Bob"}}
```

### JSON parse

```text
examples/json_parse/main.velo
```

Demonstrates:

- runtime `json` type
- `json::parse`
- roundtrip through `json::stringify`

Expected output:

```text
{"id":42,"name":"Alex"}
[1,2,3]
"hello"
```

### JSON access helpers

```text
examples/json_access/main.velo
```

Demonstrates:

- `json::has`
- `json::get_int`
- `json::get_string`
- `json::get_bool`
- `json::get_json`
- nested JSON object access

Expected output:

```text
42
Alex
true
17
has name
```

## HTTP JSON flow
```text
examples/http_json_flow/main.velo
```

Demonstrates:
- creating a synthetic HTTP request
- parsing request body as JSON
- reading JSON object fields
- returning JSON HTTP responses
- reading response status/body/header

Expected output:
```text
POST
/users
201
{"ok":true}
application/json
```

## HTTP server smoke
Start a minimal HTTP server:
```bash
./build/debug/apps/velo/velo serve ./examples/http_server_basic/main.velo
```

In another terminal:
```bash
curl -i http://127.0.0.1:8080/health
```

Expected response body:
```text
/health
```

Example source:
```text
examples/http_server_basic/main.velo
```

This example demonstrates:
- conventional `handle(req: http_request): http_response`
- `http::path(req)`
- `http::text_response(...)`
- minimal CLI serve mode

### HTTP routing helpers
```text
examples/http_routing_helpers/main.velo
```

Demonstrates:
- `http::is_route`
- `http::is_path`
- explicit handler branching
- simple `404` fallback

### HTTP JSON server
```text
examples/http_server_json/main.velo
```

Demonstrates:
- `http::is_route`
- `http::json_body`
- `http::json_response`
- JSON echo endpoint
- `404` JSON response

Smoke:
```bash
./build/debug/apps/velo/velo serve ./examples/http_server_json/main.velo
```

In another terminal:
```bash
curl -i http://127.0.0.1:8080/health

curl -i \
  -X POST \
  -H "Content-Type: application/json" \
  -d '{"name":"Alex"}' \
  http://127.0.0.1:8080/echo
```

## HTTP local smoke scripts
```bash
bash benchmarks/http/run_smoke.sh
bash benchmarks/http/run_curl_loop.sh
bash benchmarks/http/run_curl_loop.sh 100
```

These scripts are local development checks, not production benchmark targets.

## Annotation metadata
Path:
```text
examples/annotation_metadata/main.velo
```

This example demonstrates function annotations and IR metadata propagation.

```velo
use std::http as web;

@web::get("/health")
fn health(req: http_request): http_response {
    return web::text_response(200, "OK");
}
```

The source uses the `web` import alias.

The IR output normalizes the annotation name to the canonical module name:
```text
Annotation http::get("/health")
```

Run:
```bash
./build/debug/apps/velo/velo check examples/annotation_metadata/main.velo
./build/debug/apps/velo/velo ast examples/annotation_metadata/main.velo
./build/debug/apps/velo/velo ir examples/annotation_metadata/main.velo
```

## HTTP annotation routes
```text
examples/http_annotation_routes/main.velo
```

This example demonstrates validated HTTP route annotations.

```velo
use std::http;

@http::get("/health")
fn health(req: http_request): http_response {
    return http::text_response(200, "OK");
}

@http::post("/echo")
fn echo(req: http_request): http_response {
    let body: json = http::json_body(req);
    return http::json_response(201, body);
}
```

Run:
```bash
./build/debug/apps/velo/velo check examples/http_annotation_routes/main.velo
./build/debug/apps/velo/velo ast examples/http_annotation_routes/main.velo
./build/debug/apps/velo/velo ir examples/http_annotation_routes/main.velo
```

Expected IR metadata:
```text
Annotation http::get("/health")
Annotation http::post("/echo")
```

Path:
```text
examples/http_annotation_routes/main.velo
```

This example demonstrates annotation-based HTTP routing.
```velo
use std::http;

@http::get("/health")
fn health(req: http_request): http_response {
    return http::text_response(200, "OK");
}

@http::post("/echo")
fn echo(req: http_request): http_response {
    return http::text_response(201, http::request_body(req));
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

Expected:
```text
GET /health     ->  200 OK
POST /echo      ->  201 Created
GET /missing    ->  404 Not Found
```