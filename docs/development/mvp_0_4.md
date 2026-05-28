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
0.4.1   array type syntax and semantic type model
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