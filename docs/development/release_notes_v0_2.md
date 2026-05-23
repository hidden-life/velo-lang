# Velo v0.2.0 release notes draft
Velo MVP 0.2 focuses on the first data model foundation for the language.

## Highlights
- struct declarations
- user-defined struct type registry
- struct types in function parameters
- struct types in function return values
- struct types in local declarations
- struct literals
- runtime struct values
- field access expressions
- chained field access expressions
- documentation and examples for the MVP 0.2 feature set

## Language
### Struct declarations
Velo now supports struct declarations:
```velo
struct User {
    id: int;
    name: string;
}
```
Public structs and public fields can be parsed:
```velo
pub struct User {
    pub id: int;
    name: string;
}
```
Visibility is parsed and represented in the AST, buy full visibility
enforcement is not part of MVP 0.2.

### User-defined struct types
Structs can be used as named types:
```velo
struct User {
    id: int;
}

fn identity(u: User): User {
    return u;
}
```
Supported locations:
- function parameters
- function return types
- local variable declarations
- struct fields

### Struct literals
Struct literals create runtime struct values:
```velo
let user: User = User {
    id: 42,
    name: "John Doe"
}
```
Field order does not have to match declaration order.

### Field access
Field can be read from struct values:
```velo
return user.id;
```
Chained field access is supported:
```velo
return box.user.id;
```

## Semantic validation
MVP 0.2 adds semantic validation for:
- duplicate struct declarations
- duplicate struct fields
- struct names conflicting with built-in types
- unknown declared struct types
- struct literal type resolution
- duplicate fields in struct literals
- unknown fields in struct literals
- missing fields in struct literals
- field initializer type mismatch
- field access on non-struct values
- unknown field access

## IR and runtime
MVP 0.2 adds:
- runtime struct values
- `BuildStruct`
- `LoadField`
These are enough to construct struct values and read fields from them.

## Examples
New or updated examples:
```text
examples/struct_declaration/main.velo
examples/user_defined_type_registry/main.velo
examples/struct_type_usage/main.velo
examples/struct_literal/main.velo
examples/field_access/main.velo
```

## Known limitations
MVP 0.2 does not include:
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
- HTTP runtime
- JSON parser/serializer
- optimizer