# Velo MVP 0.7 status
MVP 0.7 focuses on JSON foundation and serialization.

The goal is to add JSON serialization, runtime JSON values, JSON parsing, and
basic JSON object access helpers.

## Roadmap
```text
0.7.1   std::json module and json::stringify primitives
0.7.2   json::stringify arrays and maps
0.7.3   json::stringify structs
0.7.4   runtime json type and json::parse
0.7.5   json access helpers and examples
0.7.6   docs/examples/release checklist
```

## Current step
```text
0.7.6   docs/examples/release checklist
```

## 0.7.1 scope
Implemented in this step:
- `std::json` module
- `json::stringify` builtin
- JSON serialization for `int`
- JSON serialization for `string`
- JSON serialization for `bool`
- JSON string escaping foundation
- semantic validation for primitive JSON-serializable values
- runtime module sync test
- semantic tests
- driver tests
- example
- documentation

Supported examples:
```velo
json::stringify(42)
json::stringify("Alex")
json::stringify(true)
json::stringify(false)
```
`json::stringify` return `string`.

## 0.7.2 scope
Implemented in this step:
- JSON serialization for arrays
- JSON serialization for maps
- recursive JSON serialization for nested arrays/maps
- semantic validation for JSON-serializable collection types
- driver tests
- semantic tests
- example
- documentation

Supported examples:
```velo
json::stringify([1, 2, 3])
json::stringify(["Alex", "Bob"])
json::stringify([true, false])
json::stringify(scores)
json::stringify(grouped)
```

Example output:
```json
[1,2,3]
["Alex","Bob"]
[true,false]
{"Alex":10,"Bob":20}
{"a":[1,2],"b":[3,4]}
```

## 0.7.3 scope
Implemented in this step:
- JSON serialization for structs
- JSON serialization for arrays of structs
- JSON serialization for maps with struct values
- recursive JSON serialization for nested struct collections
- semantic validation for struct JSON serialization
- driver tests
- semantic tests
- example
- documentation

Supported examples:
```velo
json::stringify(user)
json::stringify(users)
json::stringify(usersByName)
```

Example output:
```json
{"active":true,"id":1,"name":"Alex"}
[{"id":1},{"id":2}]
{"a":{"id":1},"b":{"id":2}}
```

Struct field order follows runtime field storage order.

## 0.7.4 scope
Implemented in this step:
- `json` semantic type
- runtime JSON value representation
- deep-copy support for runtime JSON values
- `json::parse` builtin
- JSON parsing for `null`
- JSON parsing for `bool`
- JSON parsing for `int`
- JSON parsing for `string`
- JSON parsing for arrays
- JSON parsing for objects
- `json::stringify` support for parsed JSON values
- runtime error for invalid JSON input
- semantic tests
- driver tests
- example
- documentation

Example:
```velo
let value: json = json::parse("{\"id\":42}");
let text: string = json::stringify(value);
```

Supported JSON input in MVP 0.7:
```text
null
true
false
42
"Alex"
[1,2,3]
{"id": 42,"name":"Alex"}
```

## 0.7.5 scope
Implemented in this step:
- `json::has`
- `json::get_int`
- `json::get_string`
- `json::get_bool`
- `json::get_json`
- semantic validation for JSON access helper signatures
- runtime JSON object field access
- runtime errors for missing JSON keys
- runtime errors for JSON field type mismatch
- nested object access through `json::get_json`
- semantic tests
- driver tests
- example
- documentation

Example:
```velo
let value: json = json::parse("{\"id\":42,\"name\":\"Alex\"}");

let id: int = json::get_int(value, "id");
let name: string = json::get_string(value, "name");
```

Nested example:
```velo
let profile: json = json::get_json(value, "profile");
let age: int = json::get_int(profile, "age");
```

## 0.7.6 scope
Implemented in this step:
- MVP 0.7 documentation review
- examples documentation review
- release checklist update
- release notes draft
- README current status update
- architecture docs synchronization
- language docs synchronization
- smoke-test command list
- tag instructions

## MVP 0.7 completed feature set
MVP 0.7 includes:
- all MVP 0.1 language foundation
- all MVP 0.2 struct foundation
- all MVP 0.3 value semantics and block scope foundation
- all MVP 0.4 array foundation
- all MVP 0.5 bytecode foundation
- all MVP 0.6 map foundation
- `std::json` module
- `json::stringify`
- JSON serialization for primitives
- JSON serialization for arrays
- JSON serialization for maps
- JSON serialization for structs
- recursive JSON serialization
- runtime `json` type
- runtime JSON value representation
- deep-copy support for runtime JSON values
- `json::parse`
- `json::has`
- `json::get_int`
- `json::get_string`
- `json::get_bool`
- `json::get_json`

## MVP 0.7 examples
```text
examples/json_stringify_primitives/main.velo
examples/json_stringify_collections/main.velo
examples/json_stringify_struct/main.velo
examples/json_parse/main.velo
examples/json_access/main.velo
```

## MVP 0.7 limitations
Not implemented in MVP 0.7:
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

## next milestone
Suggested next milestone:
```text
MVP 0.8     HTTP runtime foundation
```

Reason:
- MVP 0.7 gives JSON serialization/parsing
- HTTP runtime becomes much more useful after JSON foundation
- API/backend direction needs request/response primitives next