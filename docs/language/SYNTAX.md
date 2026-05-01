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

## Current limitations
- parameters are immutable
- there are no local variable declarations yet
- only `+` is supported
- only integer addition is supported
- operator precedence is still minimal

## Notes
This document describes only the syntax and the semantic rules implemented in the current codebase.