# Semantic analyzer

The semantic analyzer validates parsed AST before IR lowering.

## Responsibilities

The semantic analyzer currently checks:

- duplicate imports
- duplicate visible import names
- duplicate functions
- entry point validity
- function parameter types
- function return types
- duplicate parameters
- local variable declarations
- local variable scopes
- assignments
- expression types
- function calls
- builtin calls
- return statements
- `if` conditions
- `while` conditions
- `break` / `continue` placement
- declared type names
- builtin argument types
- struct declarations
- user-defined struct types
- struct literal validation
- field access validation

## Entry point

The program entry point is:

```velo
fn main(): int {
    return 0;
}
```

Rules:

- `main` must exist
- `main` must not have parameters
- `main` must return `int`

## Types

Supported semantic types:

- `int`
- `string`
- `bool`
- `void`
- `struct`
- `unknown`

`unknown` is used internally to reduce cascading diagnostics.

## Declared type validation

The semantic analyzer validates declared types in:

- function return types
- function parameters
- local variable declarations

Rules:

- unknown declared types produce `SEM030`
- `void` is allowed only as a function return type
- `void` parameters and local variables produce `SEM031`

## Struct declarations
Struct declarations are collected before function bodies are analyzed.

This allows forward references between structs:
```velo
struct User {
    profile: Profile;
}

struct Profile {
    id: int;
}
```
Rules:
- duplicate struct names are rejected
- struct names cannot conflict with built-in type names
- duplicate field names are rejected
- field types are validated

## Struct literals
Struct literals create values of user-defined struct types.

Example:
```velo
struct User {
    id: int;
    name: string;
}

fn main(): int {
    let user: User = User {
        id: 42,
        name: "John Doe"
    };
    
    return 0;
}
```

Rules:
- literal type must resolve to a known struct
- duplicate literal fields are rejected
- unknown literal fields are rejected
- each declared field must be provided
- each field initializer type must match the declared field type
- literal field order does not have to match declaration order

## Field access
Field access reads a field from a struct value.

Example:
```velo
return user.id;
```

Rules:
- the target expression must have a struct type
- the field must exist in the target struct
- the expression type is the declared field type

Chained field access is supported:
```velo
return box.user.id;
```

## Parameters

Function parameters are stored with actual semantic types.

Example:

```velo
fn echo(value: string): string {
    return value;
}
```

The parameter `value` resolves to `string`.

## Local scopes

The semantic analyzer uses a scope stack for local variables.

Rules:

- each function creates a function scope
- each `if` branch creates a nested scope
- each `while` body creates a nested scope
- variables resolve from inner scope to outer scope
- duplicate locals are forbidden only in the same scope
- shadowing from outer scopes is allowed

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

## Assignments

Rules:

- only mutable `var` locals can be assigned
- `let` locals are immutable
- assignment value type must match local type

## Return validation

Rules:

- non-void functions must return a value
- void functions must not return a value
- returned expression type must match function return type
- non-void functions must end with a guaranteed return statement

Current limitation:

- full control-flow graph analysis is not implemented yet

## If validation

`if` conditions must be `bool`.

```velo
if (true) {
    return 1;
}
```

## While validation

`while` conditions must be `bool`.

```velo
while (x < 10) {
    x = x + 1;
}
```

## break / continue validation

Rules:

- `break` is valid only inside loops
- `continue` is valid only inside loops
- nested loops are tracked using loop depth

## Arithmetic validation

Binary arithmetic operators require `int` operands:

- `+`
- `-`
- `*`
- `/`
- `%`

Unary `-` requires an `int` operand.

Arithmetic expressions produce `int`.

## Comparison validation

Comparison operators currently require `int` operands:

- `==`
- `!=`
- `<`
- `>`
- `<=`
- `>=`

Comparison expressions produce `bool`.

## Logical validation

Rules:

- `!` requires `bool`
- `&&` requires `bool && bool`
- `||` requires `bool || bool`
- logical expressions produce `bool`

Short-circuit behavior is implemented in IR lowering, not semantic analysis.

## Builtin return type metadata

Builtin return types are mirrored into `ModuleRegistry`.

Example:

```velo
use std::string as str;

fn main(): int {
    return str::len("hello");
}
```

`str::len(...)` resolves to `int`.

## Builtin argument type validation

Builtin functions also expose parameter type metadata.

Example:

```velo
use std::string as str;

fn main(): int {
    return str::len(123); // error
}
```

`str::len` expects `string`, but `123` is `int`.

Special parameter type:

```text
any
```

`any` accepts all current runtime value types.

Used by:

```text
console::println(any): void
```

## Diagnostic philosophy

Semantic diagnostics should be:

- precise
- stable
- non-cascading where possible
- tied to source ranges

When an expression already produced an error, later checks should avoid adding redundant diagnostics.

## User-defined semantic types
The semantic analyzer supports named struct types.

Builtin semantic types:
- `int`
- `string`
- `bool`
- `void`

User-defined semantic type:
```text
Struct(name)
```

Example:
```velo
struct User {
    id: int;
}

fn identity(u: User): User {
    return u;
}
```
The semantic analyzer compares struct types by name.

Examples:
```text
User == User
User != Profile
```
This is used for:
- return type validation
- local initializer validation
- assignment validation
- function argument validation
- struct literal validation
- field access validation