# Velo v0.4.0 release notes draft
Velo MVP 0.4 focuses on arrays as the first collection type in the language.

It builds on MVP 0.3 by adding array types, array literals, runtime array values, indexing
element assignment, and the first array builtin.

## Highlights
- array type syntax
- nested array type syntax
- array semantic type model
- array literals
- empty array literals with expected type
- runtime array values
- deep-copy support for arrays
- array indexing read
- nested array indexing
- array element assignment
- nested array element assignment
- arrays of struct values
- `std::array` module
- `array::len`

## Language

### Array type syntax

Arrays use `[]T` syntax:

```velo
[]int
[]string
[]bool
[]User
[][]int
```

Arrays can be used in:

function parameters
function return types
local variable declarations
struct fields

Example:
```velo
struct User {
    id: int;
}

struct Group {
    users: []User;
}
```

### Array literals
Array literals use square brackets:
```velo
let ids: []int = [1, 2, 3];
let names: []string = ["Alex", "Bob"];
let flags: []bool = [true, false];
```

Arrays of structs are supported:
```velo
let users: []User = [
    User { id: 1, name: "Alex" },
    User { id: 2, name: "Bob" }
];
```

Empty array literals require an expected array type:
```velo
let ids: []int = [];
```

### Array indexing
Array elements can be read with index expressions:
```velo
let ids: []int = [10, 20, 30];

return ids[1];
```

Arrays of structs can be indexed an then accessed with field access:
```velo
return users[0].id;
```

Nested arrays can be indexed repeatedly:
```velo
return matrix[1][0];
```

### Array element assignment
Array elements can be assigned through mutable local array values:
```velo
var ids: []int = [1, 2, 3];

ids[0] = 42;
```
Nested arrays can be assigned:
```velo
var matrix: [][]int = [
    [1, 2],
    [3, 4]
];

matrix[1][0] = 99;
```
Arrays preserve value semantics:
```velo
let original: []int = [1, 2, 3];
var copy: []int = original;

copy[0] = 42;
```
After this code:
```text
original[0] == 1
copy[0] == 42
```

### `std::array`
MVP 0.4 adds the `std::array` module.
```velo
use std::array;
```

### `array::len`
```velo
array::len(values)
```
Returns the number of elements in an array.

Example:
```velo
module app;

use std::array;

fn main(): int {
    let ids: []int = [1, 2, 3];

    return array::len(ids);
}
```

## Semantic validation
MVP 0.4 adds validation for:
- array element type resolution
- `void` as invalid array element type
- array literal element type consistency
- empty array literal type inference
- array index target type
- array index expression type
- array element assignment root mutability
- array element assignment target type
- array element assignment value type
- array builtin argument type

## IR and runtime
MVP 0.4 adds or uses:
- `ArrayValuePtr`
- deep-copy support for runtime arrays
- `BuildArray`
- `LoadIndex`
- `StoreIndexPath`
- `array::len`

## Examples
New examples:
```text
examples/array_literal/main.velo
examples/array_indexing/main.velo
examples/array_assignment/main.velo
examples/array_builtin/main.velo
```

## Known limitations
MVP 0.4 does not include:
- mixed field/index assignment paths such as `users[0].id = 2`
- `array::push`
- `array::pop`
- `array::is_empty`
- iterators
- `for` loops
- maps
- nullable types
- generics
- methods
- visibility enforcement for `pub` / private fields
- struct equality
- string ordering comparisons
- bytecode serialization
- standalone `veloc`
- standalone `velovm`
- HTTP runtime
- JSON parser/serializer
- optimizer

## Next
The next planned milestone is:
```text
MVP 0.5     VM + bytecode foundation
```