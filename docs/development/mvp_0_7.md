# Velo MVP 0.7 status
MVP 0.7 focuses on JSON foundation and serialization.

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
0.7.4   runtime json type and json::parse
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

Not implemented in 0.7.1:
- array serialization
- map serialization
- struct serialization
- `json::parse`
- runtime JSON value type
- JSON access helpers

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

Current limitations:
- JSON floats are not implemented yet
- unicode escape parsing such as `\uXXXX` is not implemented yet
- JSON access helpers are not implemented yet
- JSON parser is intentionally small and MVP-focused