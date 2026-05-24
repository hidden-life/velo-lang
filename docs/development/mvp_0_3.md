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
0.3.5   docs/examples/release checklist
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

## 0.3.3 scope
Implemented in this step:
- equality support for `string`
- inequality support for `string`
- equality support for `bool`
- inequality support for `bool`
- equality support for struct fields of type `string`
- equality support for struct fields of type `bool`
- semantic rejection for mixed equality operands
- semantic rejection for struct equality
- generic IR equality opcodes
- runtime equality comparison for `int`, `string`, and `bool`
- example

Supported equality operands:
```text
int == int
int != int

string == string
string != string

bool == bool
bool != bool
```

Examples:
```velo
"Alex" == "Alex"
"Alex" != "Bob"

true == true
true != false

user.name == "Alex"
user.active != false
```
Noty supported in 0.3.3:
```velo
user1 == user2
"a" < "b"
1 == "1"
```
Not implemented in 0.3.3:
- struct equality
- string ordering comparisons
- methods
- block-scoped locals
- visibility enforcement for `pub` / private fields

## 0.3.4 scope
Implemented in this step:
- block-scoped local variables for `if` bodies
- block-scoped local variables for `else` bodies
- block-scoped local variables for `while` bodies
- local shadowing in nested block scopes
- semantic tests for block-local visibility
- driver tests for lowerer local slot resolution
- example

Example:
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
This returns:
```text
2
```
The inner `value` shadows the outer `value` only inside the `if` body.

After the block ends, the outer local is visible again:

```velo
fn main(): int {
    let value: int = 1;

    if (true) {
        let value: int = 2;
    }

    return value;
}
```

This returns:

```text
1
```

Block-local variables do not leak outside the block:

```velo
fn main(): int {
    if (true) {
        let value: int = 1;
    }

    return value;
}
```

This is rejected with `SEM007`.

## 0.3.5 scope
Implemented in this step:
- MVP 0.3 documentation review
- examples documentation review
- release checklist update
- release notes draft
- README current status update
- architecture docs synchronization
- diagnostics docs synchronization

## MVP 0.3 completed feature set
MVP 0.3 includes:
- all MVP 0.1 language foundation
- all MVP 0.2 data model foundation
- struct value copy semantics
- field assignment
- nested field assignment
- string equality and inequality
- bool equality and inequality
- generic runtime equality for `int`, `string`, and `bool`
- block-scoped locals for `if`, `else`, and `while`
- local shadowing in nested blocks

## MVP 0.3 limitations
Not implemented in MVP 0.3:
- standalone block statements
- struct equality
- string ordering comparisons
- methods
- visibility enforcement for `pub` / private fields
- arrays
- maps
- nullable types
- generics
- classes
- interfaces
- package manager
- bytecode serialization
- standalone `veloc`
- standalone `velovm`
- HTTP runtime
- JSON parser/serializer
- optimizer