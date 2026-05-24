# Velo v0.3.0 release notes draft
Velo MVP 0.3 focuses on safer and more useful struct values.

It builds on MVP 0.2 by adding value semantics, field assignment, broader equality
support, and block-scoped locals.

## Highlights
- struct value copy semantics
- deep copy support for runtime struct values
- field assignment
- nested field assignment
- string equality and inequality
- bool equality and inequality
- equality checks for comparable struct fields
- block-scoped locals
- local shadowing in nested blocks
- updated examples and documentation

## Language
### Struct value semantics
Struct values use language-level value semantics.

Example:
```velo
let a: User = User {
    id: 1
};

var b: User = a;

b.id = 2;
```

After this code:
```text
a.id == 1
b.id == 2
```
Runtime struct values may internally use pointers, but the interpreter clones struct values
at value boundaries to avoid accidental aliasing.

### Field assignment
Fields can be assigned through mutable local struct values:
```velo
var user: User = User {
    id: 1,
    name: "Alex"
};

user.id = 2;
user.name = "Bob";
```
Nested field assignment is supported:
```velo
box.user.id = 42;
```

Field assignment through immutable locals is rejected:
```velo
let user: User = User {
    id: 1
};

user.id = 2;
```

### Equality operators
Equality and inequality are supported for:
```text
int
string
bool
```
Examples:
```velo
1 == 1
"Alex" == "Alex"
true != false
```
Struct field values can be compared when the field type is comparable:
```velo
user.name == "Alex"
user.active == true
```

### Block-scoped locals
`if`, `else`, and `while` bodies create nested local scopes.
Example:
```velo
let value: int = 1;

if (true) {
    let value: int = 2;
}
```
The inner `value` shadows the outer one only inside the block.

## Semantic validation
MVP 0.3 adds or extends validation for:
- field assignment root mutability
- field assignment target type
- field assignment assigned value type
- equality operand compatibility
- block-local visibility
- local shadowing
- duplicate local declarations in the same scope

## IR and runtime
MVP 0.3 adds or uses:
- `Runtime::cloneValue(...)`
- deep copying for runtime struct values
- `StoreFieldPath`
- `CompareEqual`
- `CompareNotEqual`
- scoped local name resolution in the lowerer

## Examples
New or updated examples:
```text
examples/field_assignment/main.velo
examples/equality/main.velo
examples/block_scopes/main.velo
```

MVP 0.2 examples remain relevant:
```text
examples/struct_literal/main.velo
examples/field_access/main.velo
```

## Known limitations
MVP 0.3 does not include:
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
- HTTP runtime
- JSON parser/serializer
- optimizer