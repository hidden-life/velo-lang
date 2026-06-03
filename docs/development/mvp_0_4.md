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
0.4.4   array element assignment
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

## 0.4.3 scope
Implemented in this step:
- array index expressions
- indexing arrays by `int`
- semantic validation that index target is an array
- semantic validation that index expression is `int`
- runtime bounds checks for array indexing
- IR lowering for array indexing
- interpreter support for loading array elements
- parser tests
- semantic tests
- driver tests
- example

Examples:
```velo
let ids: []int = [10, 20, 30];

return ids[1];
```

Arrays of structs can be indexed:
```velo
let users: []User = [
    User { id: 1, name: "Alex" },
    User { id: 2, name: "Bob" }
];

return users[1].id;
```

Nested arrays can be indexes:
```velo
let matrix: [][]int = [
    [1, 2],
    [3, 4]
];

return matrix[1][0];
```

Not implemented in 0.4.3:
- array element assignment
- array builtins

## 0.4.4 scope
Implemented in this step:
- array element assignment statements
- nested array element assignment
- assignment into arrays of primitive values
- assignment into arrays of struct values
- semantic validation that the root local is mutable
- semantic validation that the target is an array element
- semantic validation that assigned value type matches element type
- runtime bounds checks for array element assignment
- IR lowering for array element assignment
- interpreter support for storing values into array index paths
- value semantics regression tests
- example

Example:
```velo
var ids: []int = [1, 2, 3];

ids[0] = 42;
```

Nested arrays are supported:
```velo
var matrix: [][]int = [
    [1, 2],
    [3, 4]
];

matrix[1][0] = 99;
```

Arrays of structs can replace whole elements:
```velo
var users: []User = [
    User {id: 1, name: "Alex"}
];

users[0] = User {id: 2, name: "Bob"};
```

Not implemented in 0.4.4:
- mixed field/index assignment paths like `users[0].id = 2`
- array builtins