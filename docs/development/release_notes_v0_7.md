# Velo v0.7.0 release notes draft

Velo MVP 0.7 focuses on JSON foundation and serialization.

It adds JSON serialization, a runtime JSON value type, JSON parsing, and basic
JSON object access helpers.

## Highlights
- `std::json` module
- `json::stringify`
- JSON serialization for primitive values
- JSON serialization for arrays
- JSON serialization for maps
- JSON serialization for structs
- recursive JSON serialization
- runtime `json` type
- runtime JSON value representation
- `json::parse`
- `json::has`
- `json::get_int`
- `json::get_string`
- `json::get_bool`
- `json::get_json`

## JSON stringify
`json::stringify` serializes supported Velo values to JSON text.

Supported values:
- `int`
- `bool`
- `string`
- structs with supported fields
- arrays of supported values
- maps with string keys and supported values
- parsed runtime `json` values

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

## Collection serialization
Arrays:
```velo
let ids: []int = [1, 2, 3];

json::stringify(ids)
```

Output:
```json
[1,2,3]
```

Maps:
```velo
let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": 20
};

json::stringify(scores)
```

Output:
```json
{"Alex": 10, "Bob": 20}
```

Nested collections are supported:
```velo
let grouped: map<string, []int> = map {
    "a": [1, 2],
    "b": [3, 4]
};

json::stringify(grouped)
```

Output:
```json
{"a": [1,2], "b": [3,4]}
```

## Struct serialization
Struct values can be serialized to JSON objects:
```velo
struct User {
    id: int;
    name: string;
    active: bool;
}

let user: User = User {
    id: 1,
    name: "Alex",
    active: true
};

json::stringify(user)
```

Output:
```json
{"active":true,"id":1,"name":"Alex"}
```

Struct field order follows runtime field storage order.

Arrays and maps containing structs are also supported.

## Runtime JSON type
MVP 0.7 adds the `json` type.

```velo
let value: json = json::parse("{\"id\":42}");
```

Runtime JSON values support:
- null
- bool
- int
- string
- array
- object

## JSON parse
`json::parse` parses JSON text into a runtime JSON value.

```velo
let value: json = json::parse("{\"id\":42,\"name\":\"Alex\"}");
let text: string = json::stringify(value);
```

Supported JSON input in MVP 0.7:
- `null`
- `true`
- `false`
- integer numbers
- strings
- arrays
- objects

Invalid JSON input is a runtime error.

## JSON access helpers
Object field access helpers:
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

Missing keys are runtime errors for `get_*`.

Missing keys return `false` for `json::has`.

Field type mismatches are runtime errors.

## Runtime
MVP 0.7 adds:
- `JsonValuePtr`
- runtime JSON value kinds
- recursive JSON cloning
- JSON serialization helpers
- JSON parser
- JSON object access helpers

## Semantic model
MVP 0.7 adds:
- concrete `json` semantic type
- `json_serializable` builtin parameter marker
- signature validation for JSON helpers

Important distinction:
```text
json                concrete Velo type
json_serializable   builtin marker for json::stringify-compatible values
```

## Examples
New examples:
```text
examples/json_stringify_primitives/main.velo
examples/json_stringify_collections/main.velo
examples/json_stringify_struct/main.velo
examples/json_parse/main.velo
examples/json_access/main.velo
```

## Tests
MVP 0.7 adds tests for:
- runtime module sync for `std::json`
- semantic validation for `json::stringify`
- semantic validation for `json::parse`
- semantic validation for JSON access helpers
- runtime JSON serialization
- runtime JSON parsing
- runtime JSON object field access
- invalid JSON runtime errors
- missing key runtime errors
- field type mismatch runtime errors
- driver smoke tests
- IR and bytecode builtin call smoke tests

## Known limitations
MVP 0.7 does not include:
- JSON float numbers
- unicode escape parsing such as `\uXXXX`
- direct JSON indexing syntax
- JSON array access helpers
- JSON object length helpers
- JSON null helpers
- JSON mutation helpers
- pretty JSON formatting
- custom JSON field names
- schema validation
- HTTP runtime
- file IO
- nullable user-level types
- generics
- methods
- iterators
- `for` loops
- standalone `veloc`
- standalone `velovm`

## Next
Suggested next milestone:
```text
MVP 0.8     HTTP runtime foundation
```