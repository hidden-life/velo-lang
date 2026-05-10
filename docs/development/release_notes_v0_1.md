# Velo v0.1.0 release notes draft

Velo v0.1.0 is the first MVP release of the Velo language and virtual machine.

This release focuses on building a small, stable foundation for future backend/API-oriented language development.

## Highlights

- hand-written lexer
- hand-written parser
- AST model
- semantic analyzer
- IR lowerer
- stack-based VM
- runtime builtin registry
- module metadata registry
- CLI modes
- standard module foundation
- GoogleTest coverage
- documentation and development guides

## CLI

Supported commands:

```bash
velo run file.velo
velo check file.velo
velo ast file.velo
velo ir file.velo
```

Backward-compatible shorthand:

```bash
velo file.velo
```

is equivalent to:

```bash
velo run file.velo
```

## Language features

Velo v0.1.0 supports:

- modules
- imports
- import aliases
- functions
- typed parameters
- typed return values
- `int`
- `string`
- `bool`
- `void`
- `let`
- `var`
- assignment
- lexical scopes
- `if / else`
- `while`
- `break`
- `continue`
- arithmetic operators
- comparison operators
- logical operators
- short-circuit `&&` and `||`
- user-defined function calls
- builtin function calls

## Standard modules

Current standard modules:

```text
std::console
std::string
std::int
std::bool
```

Builtin signatures:

```text
console::println(any): void
string::len(string): int
int::toString(int): string
bool::toString(bool): string
```

## Example

```velo
module app;

use std::console;
use std::string as str;
use std::int as ints;
use std::bool as bools;

fn isSmall(value: int): bool {
    return value > 0 && value < 10;
}

fn main(): int {
    var x: int = 0;

    while (x < 5) {
        x = x + 1;
    }

    if (isSmall(x)) {
        console::println("x is small");
        console::println(ints::toString(x));
        console::println(bools::toString(true));
        console::println(str::len("hello"));

        return 0;
    }

    console::println("x is not small");
    return 0;
}
```

Expected output:

```text
x is small
5
true
5
```

## Architecture

Current pipeline:

```text
Source
  -> Lexer
  -> Parser
  -> AST
  -> SemanticAnalyzer
  -> IR Lowerer
  -> Interpreter / VM
  -> Runtime
```

## Testing

Velo v0.1.0 includes tests for:

- lexer
- parser
- semantic analyzer
- runtime
- module metadata
- interpreter
- driver pipeline
- AST printer
- IR printer behavior through driver tests

## Known limitations

Velo v0.1.0 intentionally does not include:

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
- standalone compiler app
- standalone VM app
- HTTP runtime
- JSON runtime
- optimizer
- full control-flow graph analysis

## Next direction

Likely v0.2 candidates:

- structs
- arrays
- maps
- `std::json`
- file-based modules
- bytecode format
- `veloc`
- `velovm`
- better diagnostics