# Velo
Velo is an experimental programming language and virtual machine focused on backend
and API-oriented development.

The current goal is **Velo MVP 0.2**: a small, strict, statically checked language
with a simple VM, standard modules, and developer-friendly CLI tooling, and basic
data model foundation based on struct.

## Current status
- modules
- `use` imports
- functions
- typed parameters
- typed return values
- `int`, `string`, `bool`, `void`
- `let` immutable locals
- `var` mutable locals
- lexical block scoping
- assignments
- `if / else`
- `while`
- `break`
- `continue`
- arithmetic operators: `+`, `-`, `*`, `/`, `%`
- unary minus
- comparison operators: `==`, `!=`, `<`, `>`, `<=`, `>=`
- logical operators: `&&`, `||`, `!`
- short-circuit evaluation for `&&` and `||`
- user-defined function calls
- builtin function calls
- semantic diagnostics
- IR lowering
- VM execution
- CLI modes: `run`, `check`, `ast`, `ir`
- struct declarations
- user-defined struct types
- struct literals
- field access expressions
- struct field assignment
- nested struct field assignment
- struct value semantics preserved during field assignment
- string equality and inequality
- bool equality and inequality
- equality checks for comparable struct fields
- block-scoped locals for `if`, `else`, and `while`
- local shadowing in nested blocks

## Standard modules
Current standard modules:
- `std::console`
    - `console::println(any): void`
- `std::string`
    - `string::len(string): int`
- `std::int`
    - `int::toString(int): string`
- `std::bool`
    - `bool::toString(bool): string`

Examples:
```velo
use std::console;
use std::string as str;
use std::int as ints;

fn main(): int {
    let text: string = ints::toString(123);
    let length: int = str::len(text);
    
    console::println(length);
    
    return 0;
}
```

## CLI
Run a program:
```bash
./build/debug/apps/velo/velo run ./examples/arithmetic/main.velo
```

Check a program without executing it:
```bash
./build/debug/apps/velo/velo check ./examples/arithmetic/main.velo
```

Print AST:
```bash
./build/debug/apps/velo/velo ast ./examples/arithmetic/main.velo
```

Print IR:
```bash
./build/debug/apps/velo/velo ir ./examples/arithmetic/main.velo
```

Backward-compatible shorthand:
```bash
./build/debug/apps/velo/velo ./examples/arithmetic/main.velo
```
is equivalent to:
```bash
./build/debug/apps/velo/velo run ./examples/arithmetic/main.velo
```

## MVP showcase
Run the MVP showcase example:
```bash
./build/debug/apps/velo/velo run ./examples/mvp_showcase/main.velo
```

Expected output:
```text
x is small
5
true
5
```

## Build
Debug build:
```bash
cmake --preset debug
cmake --build --preset debug
```

Run tests:
```bash
ctest --preset debug --output-on-failure
```

## Project layout
```text
apps/
    velo/           Main CLI application
    veloc/          Reserved for future compiler frontend
    velovm/         Reserved for future standalone VM runner
include/velo/       Public project headers
src/                Implementation files
tests/              GoogleTest-based unit tests
examples/           Velo source examples
docs/               Language, architecture, and development documentation
```

## Documentation
Start here:
- [Documentation index](docs/index.md)
- [Language syntax](docs/language/syntax.md)
- [Types](docs/language/types.md)
- [Modules](docs/language/modules.md)
- [Architecture overview](docs/architecture/overview.md)
- [Semantic analyzer](docs/architecture/semantic.md)
- [Runtime and VM](docs/architecture/runtime.md)
- [IR](docs/architecture/ir.md)
- [Adding a builtin](docs/development/adding_builtin.md)
- [Adding a language feature](docs/development/adding_lang_features.md)
- [Diagnostics](docs/development/diagnostics.md)
- [Example guidelines](docs/development/example_guidelines.md)
- [MVP release checklist](docs/development/release_checklist.md)
- [MVP 0.1 status](docs/development/mvp_0_1.md)
- [MVP v0.1.0 release notes draft](docs/development/release_notes_v0_1.md)
- [MVP 0.2 status](docs/development/mvp_0_2.md)
- [MVP v0.2.0 release notes draft](docs/development/release_notes_v0_2.md)
- [MVP 0.3 status](docs/development/mvp_0_3.md)
- [Velo v0.3.0 release notes draft](docs/development/release_notes_v0_3.md)

## Current development focus
The project has completed **Velo MVP 0.3**.

MVP 0.3 focuses on making struct values safer and more useful before moving toward
larger API-oriented features.

Completed steps:
```text
0.3.1   define struct value copy semantics
0.3.2   field assignment
0.3.3   equality operators for string and bool
0.3.4   block-scoped locals
0.3.5   docs/examples/release checklist
```

Main properties:
- keep the language small and strict
- keep the compiler pipeline understandable
- keep modules easy to extend
- keep tests close to every feature
- keep documentation synchronized with each implemented feature
- avoid adding large language features before the MVP foundation is stable

## MVP 0.2 status
The Velo MVP 0.2 data model foundation is complete when the release checklist passes.

Before tagging or announcing `v0.2.0`, run the release checklist:
- [MVP release checklist](docs/development/release_checklist.md)

Release notes draft:
- [Velo v0.2.0 release notes draft](docs/development/release_notes_v0_2.md)

MVP 0.2 is tagged as `v0.2.0`.

## MVP 0.3 status
MVP 0.3 is in progress.

Current step:
```text
0.3.4   block-scoped locals
```

## MVP 0.4 status
MVP 0.4 is in progress.

Current step:
```text
0.4.1   array type syntax and semantic type model
```