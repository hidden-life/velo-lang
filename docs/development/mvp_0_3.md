# Velo MVP 0.3 status
MVP 0.3 focuses on making struct values safer and more useful before moving toward
larger API-oriented features.

## Roadmap
```text
0.3.1   define struct value copy semantics
0.3.2   field assignment
0.3.3   equality operators for string and bool
0.3.4   block-scoped locals
0.3.5   docs/examples/release checklist
```

## Current step
```text
0.3.2   field assignment
```

## 0.3.1 scope
Implemented in this step:
- explicit runtime value cloning helper
- deep copy support for runtime struct values
- copy semantics across local load/store boundaries
- copy semantics for function arguments
- copy semantics for function return values
- copy semantics for struct literal fields
- copy semantics for loaded struct fields
- runtime tests for deep-copy behavior
- driver regression test for struct values through locals, parameters, return values, and field access

## Struct value semantics
Velo structs use value semantics at the language level.

Example:
```velo
let a: User = User {
    id: 1
};

let b: User = a;
```
After this assignment `a` and `b` are independent values.

This matters for next MVP 0.3 step:
```velo
b.id = 2;
```
The intended future behavior is:
```text
a.id == 1
b.id == 2
```
Internally, runtime struct values are represented with `StructValuePtr`, but
the interpreter must clone struct values at value boundaries to preserve
language-level value semantics.

## Not implemented in 0.3.1
- field assignment
- equality operators for string and bool
- block-scoped locals
- release checklist for MVP 0.3

## 0.3.2 scope
Implemented in this step:
- field assignment statements
- nested field assignment
- parser support for assignment targets based on expressions
- semantic validation for field assignment root mutability
- semantic validation for field assignment target type
- semantic validation for field assignment value type
- IR lowering for field assignment
- runtime support for storing values into struct field paths
- regression tests for value semantics after field assignment
- example

Example:
```velo
struct User {
    id: int;
}

fn main(): int {
    var user: User = User {
        id: 1
    };

    user.id = 42;

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
Field assignment requires the root value to be mutable local variable:
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
Value semantics are preserved:
```velo
let a: User = User {
    id: 1
};

var b: User = a;
b.id = 2;

// a.id is still 1
```

## Not implemented in 0.3.2
- methods
- equality operators for string and bool
- block-scoped locals
- visibility enforcement for `pub` / private fields