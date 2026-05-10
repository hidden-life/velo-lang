# Velo MVP 0.1 status
This document tracks the current MVP 0.1 scope.

## Goal
Velo MVP 0.1 should be a small but working language and VM capable of executing
simple backend/API-oriented programs.

The goal is not to implement a large language.

The goal is to build a stable foundation.

## Already implemented
### Project foundation
- CMake build
- GoogleTest
- CLI app: `velo`
- examples
- docs
- modular project layout

### CLI
- `velo run`
- `velo check`
- `velo ast`
- `velo ir`
- backward-compatible `velo file.velo`

### Language
- module declaration
- imports
- import aliases
- functions
- public functions
- typed parameters
- typed return values
- `return`
- `let`
- `var`
- assignment
- lexical scopes
- `if / else`
- `while`
- `break / continue`
- integer arithmetic
- comparisons
- logical operators
- short-circuit `&&` and `||`
- function calls
- builtin calls

### Types
- `int`
- `string`
- `bool`
- `void`

### Semantic analyzer
- entry point validation
- duplicate declarations
- declared type validation
- parameter type resolution
- local scope resolution
- shadowing
- assignment validation
- return validation
- condition validation
- loop-control validation
- builtin return type validation
- builtin argument type validation

### IR / VM
- IR module/function/instruction model
- local slots
- operand stack
- arithmetic
- comparisons
- logic
- jumps
- function calls
- builtin calls
- builtin return values
- IR dump mode

### Standard library foundation
Current builtins:
```text
console::println(any): void
string::len(string): int
int::toString(int): string
bool::toString(bool): string
```

## Remaining before MVP 0.1
Recommended remaining work:
1. cleanup diagnostics wording and codes
2. refresh examples and make sure demo examples return `0`
3. ensure README and docs match actual behavior
4. run the MVP release checklist

## Explicitly out of scope for MVP 0.1
- structs
- arrays
- maps
- nullable types
- generics
- classes
- interfaces
- async/await
- package manager
- bytecode serialization
- standalone `veloc`
- standalone `velovm`
- HTTP server runtime
- JSON parser/serializer
- optimizer
- full control-flow graph analysis

## Next likely milestones after MVP 0.1
### MVP 0.2 candidates
- user-defined structs
- arrays
- maps
- `std::json`
- file-based modules
- better diagnostics
- bytecode format
- `veloc`
- `velovm`

## API-language direction
Future API-oriented features may include:
- request/response types
- HTTP module
- JSON module
- validation helpers
- routing helpers
- result/error types

## Release checklist
Before considering MVP 0.1 complete, run:
```bash
cmake --build --preset debug
ctest --preset debug --output-on-failure
```
Then follow:
- [MVP release checklist](release_checklist.md)