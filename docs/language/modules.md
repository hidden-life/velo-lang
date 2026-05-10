# Velo modules
Velo uses `use` declarations to import modules.

## Import
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
use std::strign as str;

str::len("hello");
```

## Current standard modules
## std::console
```velo
use std::console;

console::println(123);
console::println("hello");
console::println(true);
```

Signature:
```text
console::println(any): void
```
`any` means the builtin accepts all current runtime value types:
- `int`
- `string`
- `bool`

## std::string
```velo
use std::string as str;

let length: int = str::len("hello");
```

Signature:
```text
string::len(string): int
```
`str::len` expects a `string` argument and returns an `int`.

## std::int
```velo
use std::int as ints;

let text: string = ints::toString(123);
```

Signature:
```text
int::toString(int): string
```
`ints::toString` expects an `int` argument and returns a `string`.

## std::bool
```velo
use std::bool as bools;

let t1: string = bools::toString(true);
let t2: string = bools::toString(false);
```

Signature:
```text
bool::toString(bool): string
```
`bools::toString` expects a `bool` argument and returns a `string`.

## Builtin argument validation
Builtin functions expose parameter metadata to semantic analysis.

Example:
```velo
use std::string as str;

fn main(): int {
    return str::len(123); // ❌ error
}
```
`str::len` expects a `string`, but `123` is `int`.

## Current limitations
The module system currently supports standard runtime modules only.

Not implemented yet:
- user-defined modules split across files
- package resolution
- module visibility rules beyond current imports
- module initialization