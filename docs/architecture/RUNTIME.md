# Runtime layer
## Current responsibilities
The runtime layer owns builtin functions and exposes them to the interpreter through a registry.

## Current components
- `Value`
- `ExecutionResult`
- `BuiltinFunction`
- `BuiltinRegistry`
- `Runtime`

## Current builtins
```text
console::println
```

## Design notes
Builtin functions must be registered in the runtime registry instead of being hardcoded inside the interpreter.
This keeps the interpreter generic and prepares the project for future modules such as:
- `std::console`
- `std::json`
- `http::request`
- `http::response`
- `crypto::hash`