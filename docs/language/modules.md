# Velo modules
Velo uses `use` declarations to import modules.

## Import
```velo
use std::console;
use std::string as str;
```

The visible module name is the last path segment by default:
```velo
use std::console;

console::println("hello");
```

Aliases are supported:
```velo
use std::strign as str;

str::len("hello");
```

## Current standard modules
## std::console
```velo
use std::console;

console::println(123);
console::println("hello");
console::println(true);
```

Signature:
```text
console::println(any): void
```
`any` means the builtin accepts all current runtime value types:
- `int`
- `string`
- `bool`

## std::string
```velo
use std::string as str;

let length: int = str::len("hello");
```

Signature:
```text
string::len(string): int
```
`str::len` expects a `string` argument and returns an `int`.

## std::int
```velo
use std::int as ints;

let text: string = ints::toString(123);
```

Signature:
```text
int::toString(int): string
```
`ints::toString` expects an `int` argument and returns a `string`.

## std::bool
```velo
use std::bool as bools;

let t1: string = bools::toString(true);
let t2: string = bools::toString(false);
```

Signature:
```text
bool::toString(bool): string
```
`bools::toString` expects a `bool` argument and returns a `string`.

## Builtin argument validation
Builtin functions expose parameter metadata to semantic analysis.

Example:
```velo
use std::string as str;

fn main(): int {
    return str::len(123); // ❌ error
}
```
`str::len` expects a `string`, but `123` is `int`.

## Current limitations
The module system currently supports standard runtime modules only.

Not implemented yet:
- user-defined modules split across files
- package resolution
- module visibility rules beyond current imports
- module initialization

## std::array
The `std::array` module contains helpers for array values.

Import:
```velo
use std::array;
```

### `array::len`
```velo
array::len(values)
```

Returns the number of elements in an array.

Example:

```velo
module app;

use std::array;

fn main(): int {
    let ids: []int = [1, 2, 3];

    return array::len(ids);
}
```

Current limitations:

- no `array::push`
- no `array::pop`
- no `array::is_empty`
- no iterators

## std::map
The `std::map` module contains helpers for map values.

Import:
```velo
use std::map;
```

### `map::len`
```velo
map::len(values)
```

Returns the number of entries in a map.

Example:
```velo
module app;

use std::map;

fn main(): int {
    let scores: map<string, int> = map {
        "Alex": 10,
        "Bob": 20
    };

    return map::len(scores);
}
```

Current limitations:
- no `map::has`
- no `map::keys`
- no `map::values`
- no `map::remove`

## std::json
The `std::json` module contains JSON helpers.

Import:
```velo
use std::json;
```

### `json::stringify`
```velo
json::stringify(value)
```

Serializes a supported Velo value to JSON text.

In MVP 0.7.1, supported values are:
- `int`
- `string`
- `bool`
- arrays of supported values
- maps with string keys and supported values
- structs with supported fields

Example:
```velo
module app;

use std::console;
use std::json;

fn main(): int {
    console::println(json::stringify(42));
    console::println(json::stringify("Alex"));
    console::println(json::stringify(true));

    return 0;
}
```

Expected output:
```text
42
"Alex"
true
```

Collections are supported:
```velo
let ids: []int = [1, 2, 3];

let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": 20
};

console::println(json::stringify(ids));
console::println(json::stringify(scores));
```

Expected output:
```text
[1,2,3]
{"Alex":10,"Bob":20}
```

Structs are supported:
```velo
struct User {
    id: int;
    name: string;
    active: bool;
}

let user: User = {
    id: 1,
    name: "Alex",
    active: true,
};

console::println(json::stringify(user));
```

Expected output:
```text
{"active":true,"id":1,"name":"Alex"}
```

Struct field order follows the runtime field storage order.

`json::stringify` also accepts parsed `json` values.

### `json::parse`
```velo
json::parse(text)
```

Parses JSON text and returns a runtime `json` value.

Example:
```velo
module app;

use std::console;
use std::json;

fn main(): int {
    let value: json = json::parse("{\"id\":42}");

    console::println(json::stringify(value));

    return 0;
}
```

Expected output:
```text
{"id": 42}
```

Supported JSON input in MVP 0.7.4:
- `null`
- `true`
- `false`
- integer numbers
- strings
- arrays
- objects

### JSON object access helpers
`std::json` provides object field access helpers.

```velo
json::has(value, "id")
json::get_int(value, "id")
json::get_string(value, "name")
json::get_bool(value, "active")
json::get_json(value, "profile")
```

Example:
```velo
module app;

use std::json;

fn main(): int {
    let value: json = json::parse("{\"id\":42,\"profile\":{\"age\":17}}");
    let profile: json = json::get_json(value, "profile");

    return json::get_int(profile, "age");
}
```

Runtime behavior:
- missing keys are runtime errors for `get_*`
- missing keys return `false` for `json::has`
- field type mismatch is a runtime error
- access helpers currently work with JSON objects only

Current limitations:
- JSON float numbers are not implemented yet
- unicode escapes such as `\uXXXX` are not implemented yet
- JSON array access helpers are not implemented yet
- direct JSON indexing syntax is not implemented yet
- JSON mutation helpers are not implemented yet

## std::http
The `std::http` module contains HTTP runtime helpers.

Import:
```velo
use std::http;
```

### `http::response`
```velo
http::response(status, body)
```

Creates a raw HTTP response value.

Signature:
```text
int, string -> http_response
```

Example:
```velo
let res: http_response = http::response(200, "OK");
```

### `http::text_response`
```velo
http::text_response(status, body)
```

Creates a text HTTP response value.

Signature:

```text
int, string -> http_response
```

Runtime behavior:

```text
headers["Content-Type"] = "text/plain"
```

Example:

```velo
let res: http_response = http::text_response(200, "Hello");
```

### `http::json_response`

```velo
http::json_response(status, value)
```

Creates a JSON HTTP response value.

Signature:

```text
int, json -> http_response
```

Runtime behavior:

```text
body = json::stringify(value)
headers["Content-Type"] = "application/json"
```

Example:

```velo
let res: http_response = http::json_response(201, json::parse("{\"ok\":true}"));
```

### `http::status`
```velo
http::status(response)
```

Returns response status code.

Signature:
```text
http_response -> int
```

### `http::body`
```velo
http::body(response)
```

Returns response body.

Signature:
```text
http_response -> string
```

### `http::has_header`
```velo
http::has_header(response, key)
```

Returns whether the response has a header with the given key.

Signature:
```text
http_response, string -> bool
```

### `http::header`
```velo
http::header(response, key)
```

Returns response header value.

Signature:
```text
http_response, string -> string
```

Missing header is a runtime error.

Example:
```velo
module app;

use std::console;
use std::http;

fn main(): int {
    let res: http_response = http::text_response(200, "Hello");

    console::println(http::status(res));
    console::println(http::body(res));
    console::println(http::header(res, "Content-Type"));

    return 0;
}
```

### `http::request`
```velo
http::request(method, path, body)
```

Creates an HTTP request value.

Signature:
```text
string, string, string -> http_request
```

Example:
```velo
let req: http_request = http::request("POST", "/users", "{\"name\":\"Alex\"}");
```

### `http::method`
```velo
http::method(request)
```

Returns request method.

Signature:
```text
http_request -> string
```

### `http::path`
```velo
http::path(request)
```

Returns request path.

Signature:
```text
http_request -> string
```

### `http::request_body`
```velo
http::request_body(request)
```

Returns raw request body.

Signature:
```text
http_request -> string
```

### `http::json_body`
```velo
http::json_body(request)
```

Parses request body as JSON and returns a runtime `json` value.

Signature:
```text
http_request -> json
```

Example:
```velo
module app;

use std::http;
use std::json;

fn main(): int {
    let req: http_request = http::request("POST", "/users", "{\"name\":\"Alex\"}");
    let body: json = http::json_body(req);

    return string::len(json::get_string(body, "name"));
}
```

### HTTP and JSON flow
HTTP request and response helpers can be combined with `std::json`.

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

This models backend handler logic without starting a real HTTP server.

### Routing helpers
```velo
http::is_method(req: http_request, method: string): bool
http::is_path(req: http_request, path: string): bool
http::is_route(req: http_request, method: string, path: string): bool
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

These helpers compare exact method and path strings. They do not parse query
strings and do not support path parameters.