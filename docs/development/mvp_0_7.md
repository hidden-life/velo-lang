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
0.7.1   std::json module and json::stringify primitives
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