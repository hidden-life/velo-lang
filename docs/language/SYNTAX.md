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

## Notes
This document describes only the syntax implemented in the current codebase.