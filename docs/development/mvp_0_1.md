# Velo MVP 0.1 status
This document tracks the current MVP 0.1 scope and release status.

## Status
```text
MVP 0.1 core scope: complete
Release readiness: ready for final verification
```

Velo MVP 0.1 is the first working language and VM foundation.

The goal of MVP 0.1 is not to provide a large production language.

The goal is to provide a small, stable, tested foundation for future language
and runtime development.

## MVP 0.1 goal
Velo MVP 0.1 should support:
- a minimal but useful language syntax
- static semantic validation
- IR lowering
- VM execution
- builtin standard modules
- developer-friendly CLI tooling
- tests for each important feature
- documentation for future development

## Implemented project foundation
- CMake build
- GoogleTest-based unit tests
- modular source layout
- examples directory
- documentation directory
- main CLI application: `apps/velo`

Reserved for future versions:
```text
apps/veloc
apps/velovm
```

Current active executable:
```text
apps/velo
```

## Implemented CLI
Supported commands:
- `velo run`
- `velo check`
- `velo ast`
- `velo ir`
- backward-compatible `velo file.velo`

## Implemented language features

### Modules and imports

- `module app;`
- `use std::console;`
- `use std::string as str;`
- import aliases

### Functions

- function declarations
- public functions with `pub`
- typed parameters
- typed return values
- user-defined function calls
- builtin function calls

### Types

Supported types:

- `int`
- `string`
- `bool`
- `void`

Rules:

- `void` is valid only as function return type
- parameters preserve declared semantic types
- local variables preserve declared semantic types

### Variables and scopes

- immutable locals with `let`
- mutable locals with `var`
- assignment to mutable locals
- lexical block scopes
- nested scope shadowing
- duplicate locals rejected within the same scope

### Control flow

- `return`
- `if / else`
- `while`
- `break`
- `continue`

### Expressions

- integer literals
- string literals
- boolean literals
- names
- calls
- grouped expressions
- unary expressions
- binary expressions

### Operators

Arithmetic:

```text
+ - * / %
```

Unary:

```text
-
!
```

Comparisons:

```text
== != < > <= >=
```

Logical:

```text
&& || !
```

`&&` and `||` use short-circuit evaluation.

## Implemented semantic validation

The semantic analyzer validates:

- duplicate imports
- duplicate visible import names
- duplicate functions
- entry point existence
- entry point signature
- unknown declared types
- invalid `void` usage
- duplicate parameters
- parameter type resolution
- local variable type resolution
- local scope lookup
- unknown symbols
- immutable local assignment
- assignment type mismatch
- local initializer type mismatch
- return type mismatch
- missing return in non-void functions
- invalid return value in void functions
- `if` condition type
- `while` condition type
- `break` outside loop
- `continue` outside loop
- arithmetic operand types
- comparison operand types
- logical operand types
- builtin module/function existence
- builtin arity
- builtin argument types
- builtin return types

## Implemented IR and VM

Implemented IR model:

- `Module`
- `Function`
- `Instruction`

Instruction field:

```cpp
code
```

Implemented instruction categories:

- constants
- locals
- calls
- arithmetic
- comparisons
- logic
- jumps
- return
- pop

Implemented VM behavior:

- operand stack
- local slots
- user-defined function calls
- builtin function calls
- builtin return values
- function return values
- conditional jumps
- loop execution
- short-circuit logical lowering
- runtime errors

## Implemented standard library foundation

Current builtin modules:

```text
std::console
std::string
std::int
std::bool
```

Current builtin signatures:

```text
console::println(any): void
string::len(string): int
int::toString(int): string
bool::toString(bool): string
```

## Implemented examples

Important valid examples:

```text
examples/arithmetic/main.velo
examples/logical/main.velo
examples/short_circuit/main.velo
examples/while_loop/main.velo
examples/break_continue/main.velo
examples/typed_parameters/main.velo
examples/std_string_len/main.velo
examples/std_to_string/main.velo
examples/mvp_showcase/main.velo
```

Error examples:

```text
examples/errors/
```

## Implemented documentation

Documentation entry point:

```text
docs/index.md
```

Language docs:

```text
docs/language/syntax.md
docs/language/types.md
docs/language/modules.md
```

Architecture docs:

```text
docs/architecture/overview.md
docs/architecture/semantic.md
docs/architecture/runtime.md
docs/architecture/ir.md
```

Development docs:

```text
docs/development/adding_builtin.md
docs/development/adding_language_feature.md
docs/development/diagnostics.md
docs/development/example_guidelines.md
docs/development/release_checklist.md
docs/development/mvp_0_1.md
```

Examples docs:

```text
docs/examples.md
```

## Final verification

Before calling MVP 0.1 complete, run:

```bash
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

Then run CLI smoke tests:

```bash
./build/debug/apps/velo/velo --help
./build/debug/apps/velo/velo --version
./build/debug/apps/velo/velo check ./examples/mvp_showcase/main.velo
./build/debug/apps/velo/velo ast ./examples/mvp_showcase/main.velo
./build/debug/apps/velo/velo ir ./examples/mvp_showcase/main.velo
./build/debug/apps/velo/velo run ./examples/mvp_showcase/main.velo
```

Expected showcase output:

```text
x is small
5
true
5
```

## MVP 0.1 release checklist

Before tagging MVP 0.1, follow:

- [MVP release checklist](release_checklist.md)

## Explicitly out of scope for MVP 0.1

These are intentionally not included:

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
- HTTP runtime
- JSON parser/serializer
- optimizer
- full control-flow graph analysis
- user-defined modules across files

## Recommended MVP 0.2 direction

Good candidates for MVP 0.2:

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
- JSON helpers
- validation helpers
- routing helpers
- result/error type
- structured error handling

## MVP 0.1 conclusion

Velo MVP 0.1 provides a working foundation:

```text
source
  -> lexer
  -> parser
  -> AST
  -> semantic analyzer
  -> IR lowerer
  -> VM
  -> runtime builtins
```

The MVP 0.1 scope is considered complete once:

- all tests pass
- examples run
- documentation matches behavior
- release checklist is completed