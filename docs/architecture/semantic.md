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
- field assignment validation
- equality operand validation

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

## Field assignment
Field assignment updates a field inside a struct value.

Example:
```velo
user.id = 42;
```
Rules:
- assignment target must be a field access expression
- root expression must be a local variable
- root local variable must be mutable
- target expression must resolve to an existing struct field
- assigned value type must match the target field type

Nested field assignment is supported:
```velo
box.user.id = 42;
```

Field assignment is rejected for immutable root locals:

```velo
let user: User = User {
    id: 1
};

user.id = 2;
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

The semantic analyzer uses a stack of local scopes.

Function body analysis starts with a root scope containing function parameters.

Nested scopes are created for:

- `if` body
- `else` body
- `while` body

Rules:

- duplicate local declarations are rejected in the current scope
- shadowing locals from outer scopes is allowed
- name resolution searches from the innermost scope to the outermost scope
- block-local variables are not visible after the block ends

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

## Equality validation
Equality operators are supported for comparable primitive types:
```text
int
string
bool
```
Allowed:
```velo
1 == 1
"Alex" == "Alex"
true != false
```
Rejected:
```velo
1 == "1"
user1 == user2
```
Rules:
- both operands must have the same type
- the type must be equality-comparable
- supported equality-comparable types are `int`, `string`, and `bool`
- struct equality is not part of MVP 0.3.3

Ordering comparison operators remain integer-only:
```velo
1 < 2
```

String ordering is rejected:
```velo
"a" < "b"
```

## Array types
Array types are represented in the semantic type model as `Array`.

Each array type stores its element type.

Example:
```text
[]int       -> Array<Int>
[]User      -> Arrayy<Struct User>
[][]int     -> Array<Array<Int>>
```

Validation rules:
- array element type must be a known value type
- `void` cannot be used as an array element type
- arrays can appear in function parameters
- arrays can appear in function return types
- arrays can appear in local declarations
- arrays can appear in struct fields

Runtime array values are not part of MVP 0.4.1.

## Array literals
Array literals are checked by inferring their element type.

Rules:
- non-empty array literals infer their element type from the first element
- all following elements must have the same type
- empty array literals require an expected array type
- array literal type is `[]T`

Examples:
```velo
let ids: []int = [1, 2, 3];
let empty: []int = [];
```

Rejected:
```velo
let values: []int = [1, "bad"];
```