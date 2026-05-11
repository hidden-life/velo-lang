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
0.2.2   user-defined type registry
```

## 0.2.1 scope
Implemented in this step:
- `struct` declaraions
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

Not implemented in 0.2.1:
- using structs as types in parameters
- using structs as local variable types
- struct literals
- field access
- runtime struct values

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

Not implemented in 0.2.3:
- struct literals
- runtime struct access
- field access
- field assignment