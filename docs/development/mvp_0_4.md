# Velo MVP 0.4 status
MVP 0.4 focuses on arrays as the first collection type in Velo.

## Roadmap
```text
0.4.1   array type syntax and semantic type model
0.4.2   array literals and runtime array values
0.4.3   array indexing read
0.4.4   array element assignment
0.4.5   array builtins and examples
0.4.6   docs/examples/release checklist
```

## Current step
```text
0.4.2   array literals and runtime array values
```

## 0.4.1 scope
Implemented in this step:
- array type syntax
- nested array type syntax
- array semantic type model
- array element type validation
- arrays in function parameters
- arrays in function return types
- arrays in local declarations
- arrays in struct fields
- parser tests
- semantic tests
- documentation

Supported type syntax:
```text
[]int
[]string
[]bool
[]User
[][]int
```

Example:
```velo
module app;

struct User {
    id: int;
}

struct Group {
    users: []User;
}

fn count(ids: []int): int {
    return 0;
}

fn main(): int {
    return 0;
}
```

## Not implemented in 0.4.1
- array literals
- runtime array values
- array indexing
- array element assignment
- array builtins

## 0.4.2 scope
Implemented in this step:
- array literal expressions
- empty array literals with an expected array type
- semantic validation for array literal element types
- runtime array values
- deep-copy support for array literals
- IR lowering for array literals
- interpreter support for building runtime arrays
- parser tests
- semantic tests
- runtime tests
- driver tests
- example

Examples:
```velo
let ids: []int = [1, 2, 3];
let names: []string = ["Alex", "Bob", "John"];
let flags: []bool = [true, false];
```

Arrays of structs are supported:
```velo
struct User {
    id: int;
    name: string;
}

let users: []User = [
    User { id: 1, name: "Alex" },
    User { id: 2, name: "Bob" }
];
```

Empty array literals require an expected array type:
```velo
let ids: []int = [];
```

Not implemented in 0.4.2:
- array indexing
- array element assignment
- array builtins