# Velo types
Velo currently supports a small set of built-in types.

## Supported types
- `int`
- `string`
- `bool`
- `void`

## int
Integer value.
```velo
let value: int = 42;
```

Supported operations:
```velo
1 + 2
5 - 3
2 * 4
8 / 2
7 % 3
```

## string
String value.
```velo
let text: string = "hello";
```
Current string functionality is provided by `std::string`.
```velo
use std::string as str;

let length: int = str::len("hello"); // 5
```

## bool
Boolean value.
```velo
let enabled: bool = true;
let disabled: bool = false;
```
Supported logical operations:
```velo
!enabled
enabled && true
disabled || true
```

## void
`void` means no value.

It is valid only as a function return type:
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
```
Invalid:
```velo
let value: void = 0;
```

## Declared type validation
The semantic analyzer validates declared types in:
- function return types
- function parameters
- local variable declarations

Unknown types produce semantic diagnostics.

Example:
```velo
fn broken(value: mystery): int {
    return 0;
}
```

This is invalid because `mystery` is not a known type.

## User-defined struct types
Struct declarations are supported.

Example:
```velo
struct Profile {
    id: int;
}

struct User {
    profile: Profile;
    name: string;
}
```

A struct field can reference another declared struct type.

Forward references between struct declarations are supported:
```velo
struct User {
    profile: Profile;
}

struct Profile {
    id: int;
}
```

Current limitation:
- struct type declarations exist
- struct field types can reference declared structs
- using struct types in parameters, local variables and return values is planned for MVP 0.2.3
- struct literals are planned for MVP 0.2.4
- field access is planned for MVP 0.2.5

## Current limitations
Velo does not yet support:
- arrays
- maps
- nullable types
- generics
- user-defined types