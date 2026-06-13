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
0.7.2   json::stringify arrays and maps
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

Current limitations:
- structs are not supported by `json::stringify` yet
- arrays of structs are not supported yet
- maps with struct values are not supported yet
- `json::parse` is not implemented yet