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

## Struct declarations
Struct declarations define named data shapes.
```velo
pub struct User {
    pub id: int;
    pub name: string;
    active: bool;
}
```
Rules:
- structs are private by default
- `pub struct` marks a struct as public
- fields are private by default
- `pub` fields are public
- field types are validated
- duplicate field names are rejected

Struct fields can reference other declared structs:
```velo
struct Profile {
    id: int;
}

struct User {
    profile: Profile;
}
```
Forward references are allowed:
```velo
struct User {
    profile: Profile;
}

struct Profile {
    id: int;
}
```
Struct names cannot conflict with built-in type names:
```velo
struct int {
    value: int
}
```
This is invalid!

Struct types can be used in function parameters, return types, and local declarations:
```velo
struct User {
    id: int;
}

fn identity(u: User): User {
    return u;
}

fn useUser(u: User): int {
    let copy: User = identity(u);
    
    return 0;
}
```

### Struct literals
Struct values can be created with struct literals:
```velo
struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: 1,
        name: "John Doe"
    };
    
    return 0;
}
```
Rules:
- struct type must exist
- field names must exist
- duplicate fields are rejected
- all fields must be provided
- field values must match declared field types
- field order does not matter

### Struct field access
Field access is supported:
```velo
fn main(): int {
    let user: User = User {
        id: 42,
        name: "Alex",
        active: true
    };

    return user.id;
}
```
Chained field access is supported:
```velo
struct User {
    id: int;
}

struct Box {
    user: User;
}

fn main(): int {
    let box: Box = Box {
        user: User {
            id: 42
        }
    };

    return box.user.id;
}
```
Current limitation:
- field assignment is not implemented yet
- visibility rules for private/public fields are not enforced yet

Field assignment is supported for mutable local struct values:
```velo
struct User {
    id: int;
    name: string;
}

fn main(): int {
    var user: User = User {
        id: 1,
        name: "Alex"
    };

    user.id = 42;
    user.name = "Bob";

    return user.id;
}
```
Nested field assignment is supported:
```velo
struct User {
    id: int;
}

struct Box {
    user: User;
}

fn main(): int {
    var box: Box = Box {
        user: User {
            id: 1
        }
    };

    box.user.id = 42;

    return box.user.id;
}
```
Field assignment requires the root variable to be mutable:
```velo
var user: User = User {
    id: 1
};

user.id = 2;
```

This is rejected:

```velo
let user: User = User {
    id: 1
};

user.id = 2;
```

Current limitation:
- visibility rules for private/public fields are not enforced yet
- methods are not implemented yet

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

## Equality operators
Velo supports equality operators for `int`, `string`, and `bool`.
```velo
1 == 1
1 != 2

"Alex" == "Alex"
"Alex" != "Bob"

true == true
false != true
```

Struct fields can be compared when their field type is comparable:
```velo
struct User {
    name: string;
    active: bool;
}

fn main(): int {
    let user: User = User {
        name: "Alex",
        active: true
    };

    if (user.name == "Alex") {
        return 1;
    }

    return 0;
}
```
Current limitations:
- struct equality is not implemented
- string ordering comparisons are not implemented
- mixed-type equality is rejected