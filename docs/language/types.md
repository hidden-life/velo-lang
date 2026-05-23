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

### Equality support
- `int` supports `==` and `!=`
- `string` supports `==` and `!=`
- `bool` supports `==` and `!=`

Current limitations:
- struct equality is not implemented yet
- string ordering comparisons are not implemented yet

### Local variable scopes

Function parameters and top-level function locals live in the function body scope.

`if`, `else`, and `while` bodies create nested local scopes.

```velo
let value: int = 1;

if (true) {
    let value: int = 2;
}
```

The inner `value` is a different local variable and shadows the outer one only inside the block.

Duplicate local declarations are rejected only inside the same scope. Shadowing from an outer scope is allowed.

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

Current support:
- struct declarations
- struct field types can reference declared structs
- function parameters can use struct types
- function return types can use struct types
- semantic type checking understands specific struct names
- struct literals can create runtime struct values
- field access can read fields from struct values
- field assignment can update fields through mutable local struct values

Example:
```velo
struct User {
    id: int;
    name: string;
}

fn identity(u: User): User {
    return u;
}

fn main(): int {
    let user: User = User {
        id: 42,
        name: "Alex"
    };

    let copy: User = identity(user);

    return copy.id;
}
```
Current limitation:
- field assignment is not implemented yet
- methods are not implemented yet
- arrays/maps/generics are not implemented yet

### Struct value semantics
Velo structs use value semantics at the language level.

Example:
```velo
let a: User = User {
    id: 1
};

let b: User = a;
```
`a` and `b` are independent struct values.

This is important for future field assignment support:
```velo
b.id = 2;
```
The intended behavior is:
```text
a.id == 1
b.id == 2
```
The runtime may internally use pointers for struct storage, but the interpreter
clones struct values at value boundaries to preserve language-level value semantics.

Field assignment preserves value semantics:
```velo
let a: User = User {
    id: 1
};

var b: User = a;

b.id = 2;
```
After this code:
```text
a.id == 1
b.id == 2
```

## Current limitations
Velo does not yet support:
- methods are not implemented yet
- array/maps/generics are not implemented yet