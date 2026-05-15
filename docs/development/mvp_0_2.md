# Velo MVP 0.2 status
MVP 0.2 focuses on data model foundation.

## Roadmap
```text
0.2.1   struct declarations
0.2.2   user-defined type registry
0.2.3   struct type usage in parameters/locals/returns
0.2.4   struct literals
0.2.5   field access
0.2.6   docs/examples/release checklist
```

## Current step
```text
0.2.6 docs/examples/release checklist
```

## 0.2.1 scope
Implemented in this step:
- `struct` declarations
- `pub struct`
- struct fields
- `pub` fields
- duplicate struct diagnostics
- duplicate field diagnostics
- field type validation
- AST printer support
- parser tests
- semantic tests
- example

Example:
```velo
module app;

pub struct User {
    pub id: int;
    pub name: string;
    active: bool;
}

fn main(): int {
    return 0;
}
```

## 0.2.2 scope
Implemented in this step:
- user-defined struct type registry
- struct field types can reference declared structs
- forward references between struct declarations
- builtin type name conflict diagnostics

Example:
```velo
module app;

struct Profile {
    id: int;
}

struct User {
    profile: Profile;
}

fn main(): int {
    return 0;
}
```

Forward references are supported:
```velo
struct User {
    profile: Profile;
}

struct Profile {
    id: int;
}
```

## 0.2.3 scope
Implemented in this step:
- semantic type model with named struct types
- struct types in function parameters
- struct types as function return types
- struct types in local variable declarations
- return type checking for struct types
- local initializer checking for struct types
- user-defined function argument checking

Example:
```velo
module app;

struct User {
    id: int;
}

fn identity(user: User): User {
    return user;
}

fn useUser(user: User): int {
    let copy: User = identity(user);
    return 0;
}

fn main(): int {
    return 0;
}
```

## 0.2.4 scope
Implemented in this step:
- struct literal expressions
- semantic validation for unknown struct literal types
- semantic validation for duplicate literal fields
- semantic validation for unknown literal fields
- semantic validation for struct literal field types
- semantic validation for missing fields
- runtime struct values
- IR lowering for struct literals
- interpreter support for building struct values
- parser tests
- semantic tests
- driver tests
- example

Example:
```velo
module app;

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
Struct literal field order does not have to match declaration order:
```velo
let user: User = {
    active: true,
    name: "John Doe"
    id: 1,
};
```

## 0.2.5 scope
Implemented in this step:
- field access expressions
- chained field access
- semantic validation that field access target is a struct
- semantic validation that the selected field exists
- IR lowering for field access
- interpreter support for loading fields from runtime struct values
- lexer support for `.`
- parser tests
- semantic tests
- driver tests
- example

Example:
```velo
module app;

struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: 42,
        name: "John Doe"
    };
    
    return user.id;
}
```
Chained field access is supported:
```velo
module app;

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

## 0.2.6 scope
Implemented in this step:
- language docs synchronized with MVP 0.2
- architecture docs synchronized with struct literals and field access
- examples documented
- release checklist updated for MVP 0.2
- release notes draft added
- README current status updated

## MVP 0.2 limitations
Not implemented in MVP 0.2:
- field assignment
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