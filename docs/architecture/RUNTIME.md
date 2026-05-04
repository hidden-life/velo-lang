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

## Module generation
ModuleRegistry is automatically derived from BuiltinRegistry.
Each builtin function defines:
- module name
- function name

Example:
`console::println -> module: console, function: println`

## Builtin arity
Each builtin function stores its expected argument count.

The runtime uses this information for interpreter validation, and the module registry mirrors it for semantic validation.

Example:
```text
console::println(value) -> arity: 1
```

## Module registry mutability
ModuleRegistry now provides a mutable API (`findMutable`) for safe updates during runtime initialization.

This removes previous unsafe `const_cast` usage.

## Stack discipline
The interpreter follows a simple rule:
- expressions push values onto the stack
- expression statements remove their result via `Pop`
- return statements consume the top value as exit code

This prevents stack pollution during execution.

## Function call frames
Each user-defined function call creates an isolated stack frame:
- the caller stack is saved
- a new stack is used for the callee
- the return value is transferred back to the caller stack

This ensures proper return value propagation and prevents stack interference.

## Local loading
Function parameters are currently represented as local values inside the callee stack frame.

The `LoadLocal` instruction copies a local value from the current frame onto the stack.

Example:
```text
parameter value at local slot 0
LoadLocal 0
```

## Integer addition
The `AddInt` instruction consumes two integer values from the stack and pushes their sum back.
```text
PushInt 20
PushInt 22
AddInt
Return
```

## Builtin return types
Builtin functions now store their return type in metadata.

This metadata is mirrored into `ModuleRegistry`, so semantic analysis can resolve builtin call types without hardcoded checks.

Example:
```text
console::println(value) -> void
```

## Local storage
`StoreLocal` stores a value from the stack into a local slot.

`LoadLocal` reads from a local slot.

Current implementation:
- locals share the current function frame
- parameters occupy initial slots
- `let` variables append new slots

## Conditional jumps
The interpreter now executes functions through an instruction pointer.

Supported jump instructions:
- `JumpIfFalse`
- `Jump`

This enables basic `if/else` execution.