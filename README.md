# Velo
Velo is an experimental programming language and virtual machine focused on backend
and API-oriented development.

The current goal is **Velo MVP 0.1**: a small, strict, statically checked language
with a simple VM, standard modules, and developer-friendly CLI tooling.

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

## Current development focus
The project is currently moving toward **MVP 0.1**.

Main properties:
- keep the language small and strict
- keep the compiler pipeline understandable
- keep modules easy to extend
- keep tests close to every feature
- avoid adding large language features before the MVP foundation is stable