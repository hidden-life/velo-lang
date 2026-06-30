# Velo
Velo is an experimental programming language and virtual machine focused on backend
and API-oriented development.

The current goal is **Velo MVP 0.10**: a small, strict, statically checked language
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
- array literals
- runtime array values
- arrays of primitive values
- arrays of struct values
- array indexing read
- nested array indexing
- field access after array indexing
- array element assignment
- nested array element assignment
- array value semantics preserved during element assignment
- `std::array` module
- `array::len`
- bytecode compiler
- IR module to bytecode module conversion
- IR instruction to bytecode instruction mapping
- bytecode VM
- bytecode module execution
- bytecode stack and local slots
- bytecode builtin calls
- bytecode user-defined function calls
- bytecode disassembler
- `velo bytecode`
- `velo bc`
- bytecode text file format draft
- bytecode module writer
- bytecode module reader
- bytecode module roundtrip tests
- map literals
- runtime map values
- map value semantics
- map indexing read
- field access after map indexing
- array indexing after map indexing
- map element assignment
- map element insert assignment
- nested map assignment
- mixed map/array index assignment
- `std::map` module
- `map::len`
- `std::json`
- `json::stringify`
- JSON serialization for primitive values
- JSON serialization for arrays
- JSON serialization for maps
- recursive JSON serialization for nested collections
- JSON serialization for structs
- JSON serialization for arrays of structs
- JSON serialization for maps with struct values
- runtime `json` type
- `json::parse`
- JSON roundtrip through `json::stringify`
- `json::has`
- `json::get_int`
- `json::get_string`
- `json::get_bool`
- `json::get_json`
- nested JSON object access
- runtime HTTP request values
- runtime HTTP response values
- `http_request`
- `http_response`
- `std::http`
- `http::response`
- `http::text_response`
- `http::json_response`
- `http::status`
- `http::body`
- `http::has_header`
- `http::header`
- `http::request`
- `http::method`
- `http::path`
- `http::request_body`
- `http::json_body`
- handler-like HTTP/JSON flow examples
- runtime benchmark scaffold
- benchmark plan documentation
- HTTP routing helpers:
  - `http::is_method`
  - `http::is_route`
  - `http::is_path`

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

Print bytecode:
```bash
./build/debug/apps/velo/velo bytecode ./examples/arithmetic/main.velo
./build/debug/apps/velo/velo bc ./examples/arithmetic/main.velo
```

Backward-compatible shorthand:
```bash
./build/debug/apps/velo/velo ./examples/arithmetic/main.velo
```
is equivalent to:
```bash
./build/debug/apps/velo/velo run ./examples/arithmetic/main.velo
```

Start a minimal HTTP server:
```bash
./build/debug/apps/velo/velo serve ./examples/http_server_basic/main.velo
```

Routing helper server example:
```bash
./build/debug/apps/velo/velo serve ./examples/http_routing_helpers/main.velo
```

Smoke:
```bash
curl -i http://127.0.0.1:8080/health
curl -i http://127.0.0.1:8080/hello
curl -i http://127.0.0.1:8080/missing
```

Run the routing helper server example:
```bash
./build/debug/apps/velo/velo serve ./examples/http_routing_helpers/main.velo
```

In another terminal:
```bash
curl -i http://127.0.0.1:8080/health
curl -i http://127.0.0.1:8080/hello
curl -i http://127.0.0.1:8080/missing
```

Run local smoke scripts:
```bash
bash benchmarks/http/run_smoke.sh
bash benchmarks/http/run_curl_loop.sh
```

## MVP showcase
Run the MVP showcase example:
```bash
./build/debug/apps/velo/velo run ./examples/mvp_showcase/main.velo
```

Smoke request:
```bash
curl -i http://127.0.0.1:8080/health
```

Expected response body:
```text
/health
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
- [Benchmark plan](docs/development/benchmark_plan.md)
- [MVP 0.1 status](docs/development/mvp_0_1.md)
- [MVP v0.1.0 release notes draft](docs/development/release_notes_v0_1.md)
- [MVP 0.2 status](docs/development/mvp_0_2.md)
- [MVP v0.2.0 release notes draft](docs/development/release_notes_v0_2.md)
- [MVP 0.3 status](docs/development/mvp_0_3.md)
- [Velo v0.3.0 release notes draft](docs/development/release_notes_v0_3.md)
- [MVP 0.4 status](docs/development/mvp_0_4.md)
- [Velo v0.4.0 release notes draft](docs/development/release_notes_v0_4.md)
- [MVP 0.5 status](docs/development/mvp_0_5.md)
- [Velo v0.5.0 release notes draft](docs/development/release_notes_v0_5.md)
- [MVP 0.6 status](docs/development/mvp_0_6.md)
- [Velo v0.6.0 release notes draft](docs/development/release_notes_v0_6.md)
- [MVP 0.7 status](docs/development/mvp_0_7.md)
- [Velo v0.7.0 release notes draft](docs/development/release_notes_v0_7.md)
- [MVP 0.8 status](docs/development/mvp_0_8.md)
- [Velo v0.8.0 release notes draft](docs/development/release_notes_v0_8.md)
- [MVP 0.9 status](docs/development/mvp_0_9.md)
- [Velo v0.9.0 release notes draft](docs/development/release_notes_v0_9.md)

## Current development focus
The project is moving toward **Velo MVP 0.10**.

MVP 0.10 focuses on annotation foundation and annotation-based HTTP routing.

Planned steps:
```text
0.10.1      lexer/parser/AST support for function annotations
0.10.2      semantic validation for annotations
0.10.3      IR/module metadata propagation
0.10.4      HTTP route annotation validation
0.10.5      serve mode annotation route dispatch
0.10.6      docs/examples/release checklist
```

Main properties:
- keep the language small and strict
- keep the compiler pipeline understandable
- keep modules easy to extend
- keep tests close to every feature
- keep documentation synchronized with each implemented feature
- avoid adding large language features before the MVP foundation is stable

## MVP 0.2 status
MVP 0.2 is tagged as `v0.2.0`.

## MVP 0.3 status
MVP 0.3 is tagged as `v0.3.0`.

## MVP 0.4 status
MVP 0.4 is tagged as `v0.4.0`.

## MVP 0.5 status
MVP 0.5 is tagged as `v0.5.0`.

## MVP 0.6 status
MVP 0.6 is tagged as `v0.6.0`.

## MVP 0.7 status
MVP 0.7 is tagged as `v0.7.0`. 

## MVP 0.8 status
MVP 0.8 is tagged as `v0.8.0`.

## MVP 0.9 status
MVP 0.9 is tagged as `v0.9.0`.

## MVP 0.10 status
MVP 0.10 is in progress.
