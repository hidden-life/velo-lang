# Velo syntax
This documentation describes the currently supported Velo syntax.

## Module declaration
Every file currently starts with a module declaration:
```velo
module app;
```

## Imports
Standard modules are imported with `use`:
```velo
use std::console;
use std::string as str;
```
The visible module name is the last path segment by default:
```velo
use std::console;

console::println("hello");
```
Aliases are supported:
```velo
use std::string as str;

str::len("hello");
```

## Functions
Function declaration:
```velo
fn main(): int {
    return 0;
}
```

Function with parameters:
```velo
fn add(a: int, b: int): int {
    return a + b;
}
```

Void functions:
```velo
fn log(): void {
    return;
}
```

Public functions are marked with `pub`:
```velo
pub fn handler(): int {
    return 0;
}
```

## Local variables
Immutable local:
```velo
let value: int = 10;
```

Mutable local:
```velo
var counter: int = 0;

counter = counter + 1;
```

## Scope
Variables declared inside a block are visible only inside that block:
```velo
fn main(): int {
    if (true) {
        let value: int = 42;
    }
    
    return 0;
}
```

Using variable outside its scope is a semantic error:
```velo
fn main(): int {
    if (true) {
        let value: int = 42;
    }
    
    return value; // ❌ error
}
```

Shadowing in nested scopes is allowed:
```velo
fn main(): int {
    let value: int = 1;
    if (true) {
        let value: int = 2;
        return value;
    }
    
    return value;
}
```

## Return
Return with value:
```velo
return 42;
```

Void return:
```velo
return;
```

Non-void functions must end with a guaranteed return statement.

## If / else
```velo
if (value > 10) {
    return 1;
} else {
    return 0;
}
```

The condition must be `bool`.

## While
```velo
var x: int = 0;
while(x < 5) {
    x = x + 1;
}
```

The condition must be `bool`.

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
- `continue` jumps to the next loop iteration
- both are valid only inside loops

## Arithmetic
Supported integer arithmetic operators:
- `+`
- `-`
- `*`
- `/`
- `%`
- unary `-`

Example:
```velo
fn main(): int {
    return (1 + 2) * 3 - 4 / 2;
}
```

Grouped expressions are supported:
```velo
(1 + 2) * 3
```

Current limitation:
- arithmetic currently supports `int` only

## Comparisons
Supported comparison operators:
- `==`
- `!=`
- `<`
- `>`
- `<=`
- `>=`

Example:
```velo
if (x > 0) {
    return 1;
}
```

Current limitation:
- comparisons currently supports `int` operands only

## Logical operators
Supported logical operators:
- `&&`
- `||`
- `!`

Example:
```velo
if (x > 0 && x < 10) {
    return 1;
}
```

`&&` and `||` use short-circuit evaluation:
```velo
false && explode(); // explode is not called
true || explode();  // explode is not called
```

## Function calls
User-defined function call:
```velo
fn add(a: int, b: int): int {
    return a + b;
}

fn main(): int {
    return add(1, 2);
}
```

Builtin/module function call:
```velo
use std::console;

fn main(): int {
    console::println("hello");
    return 0;
}
```