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

## Notes
This document describes only the syntax and the semantic rules implemented in the current codebase.