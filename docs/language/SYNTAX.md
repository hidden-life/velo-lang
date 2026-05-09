# Velo Syntax Draft
## Imports
```velo
use std::console;
use std::console as out;
```

## Module declaration
```velo
module app;
```

## Function declaration
```velo
fn main(): int {
    console::println("Hello, Velo!");
    return 0;
}
```

## Visibility
```velo
pub fn main(): int {
    return 0;
}
```

## Expression statement
```velo
console::println("Hello, Velo!");
```

## Return statement
```velo
return 0;
```

## Current semantic rules
- a program must contain `main`
- `main` must return `int`
- `main` must not have parameters
- qualified module calls must resolve through `use`

## User-defined function calls
```velo
fn helper(): int {
    return 0;
}

fn main(): int {
    helper();
    return 0;
}
```

## Parameter references
```velo
fn identity(value: int): int {
    return value;
}

fn main(): int {
    return identity(42);
}
```

## Integer addition
```velo
fn add(a: int, b: int): int {
    return a + b;
}
```

## Type checking
```velo
"a" + 1 // ❌ invalid
1 + 2 // ✔️ valid
```

Return type must match function signature:
```velo
fn broken(): int {
    return "Hello"; // ❌
}
```

## Function call return types
```velo
fn text(): string {
    return "hello";
}

fn main(): int {
    return text(); // ❌ invalid: string returned from int function
}
```

Builtin `console::println(...)` is currently treated as `void`

## Void functions
```velo
fn log(): void {
    return;
}
```
Rules:
- `void` functions may use `return;`
- `void` functions must not return a value
- non-void functions must return a value

## Required return in non-void functions
```velo
fn broken(): int {
    console::println("oops");
}
```
Invalid because `int` functions must end with `return ...;`.

Current implementation:
- final statement must be `return`
- full control-flow analysis is not implemented yet

## Local variables
```velo
fn main(): int {
    let x: int = 42;
    return x;
}
```

Current limitations:
- immutable only
- function scope only
- initializer required

## Mutable variables
```velo
fn main(): int {
    var x: int = 1;
    x = x + 41;
    
    return x;
}
```
Rules:
- `let` -> immutable
- `var` -> mutable

## If / else
```velo
fn main(): int {
    if (true) {
        return 42;
    } else {
        return 0;
    }
}
```
Rules:
- condition must be `bool`
- `else` is optional
- non-void functions can satisfy return requirement through if/else branches

## Comparisons
```velo
if (x > 10) {
    return 1;
}
```
Supported operators:
- `==`
- `!=`
- `>`
- `<`
- `<=`
- `>=`

Current limitation:
- comparisons currently support `int` operands only

## While loops
```velo
fn main(): int {
    var x: int = 0;
    
    while (x < 5) {
        x = x + 1;
    }
    
    return x;
}
```
Rules:
- condition must be `bool`
- loop body is a statement block
- `while` does not currently guarantee function return for non-void functions

## Scope
Variables declared inside `{...}` are scoped to that block.
```velo
if (true) {
    let x: int = 42;
}

return x; // ❌ error
```

## break / continue
```velo
while(true) {
    break;
}
```

```velo
while(x < 10) {
    x = x + 1;
    continue;
}
```
Rules:
- `break` exits the nearest enclosing loop
- `continue` jumps to the next iteration of the nearest enclosing loop
- both are valid only inside loops

## Logical operators
```velo
if (x > 0 && x < 10) {
    return 1;
}

if (!false || true) {
    return 1;
}
```

Supported operators:
- `&&`
- `||`
- `!`

`&&` and `||` use short-circuit evaluation:
```velo
false && explode() // explode is not called
true || explode() // explode is not called
```

## Typed parameters
Function parameters preserve their declared type during semantic analysis.
```velo
fn echo(value: string): string {
    return value;
}

fn identity(value: bool): bool {
    return value;
}
```
Supported parameter types:
- `int`
- `string`
- `bool`

## Declared types
Supported built-in types:
- `int`
- `string`
- `bool`
- `void`

`void` is only valid as a function return type:
```velo
fn log(): void {
    return;
}
```

Invalid:
```velo
fn bad(value: void): int {
    return 0;
}

fn main(): int {
    let x: void = 0;
    return 0;
}
```

## Arithmetic
```velo
fn main(): int {
    return (1 + 2) * 3 - 4 / 2;
}
```

Supported integer operators:
- `+`
- `-`
- `*`
- `/`
- `%`
- unary `-`

Grouped expressions are supported:
```velo
(1 + 2) * 3
```

Current limitation:
- arithmetic currently supports `int` only

## Current limitations
- parameters are immutable
- there are no local variable declarations yet
- only `+` is supported
- only integer addition is supported
- operator precedence is still minimal

## Notes
This document describes only the syntax and the semantic rules implemented in the current codebase.