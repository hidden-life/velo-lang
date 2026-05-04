# Semantic layer
## Current responsibilities
The current semantic layer performs only a minimal set of checks:
1. verifies that `main` exists
2. verifies that `main` has no parameters
3. verifies that `main` returns `int`
4. validates duplicate visible import names
5. validates duplicate function declarations
6. resolves qualified names in expressions through `use`
7. validates builtin function argument count
8. validates parameter references
9. rejects duplicate parameter name
10. recursively validates binary expression operands structurally

## Current limitations
The current semantic layer does not yet implement:
- local scopes
- variable resolution
- type checking for expressions
- return type validation inside function bodies
- module registry
- standard library symbol validation
- full expression type checking
- user-defined function call validation
- local variable declarations

## Type checking
The semantic analyzer now performs basic type validation:
- integer literals -> `int`
- string literals -> `string`
- binary '+' -> `int` + `int` only
- return expressions must match function return type

## Call expression types
Call expression types are now resolved from declarations:
- user-defined function calls use the function return type
- `console::println(...)` is treated as `void`

This removes the previous temporary behavior where every call expression was treated as `int`

## Builtin call metadata
Semantic analysis now reads builtin function return types from `ModuleRegistry`.

This removes hardcoded knowledge such as:
```text
console::println -> void
```

Import aliases are resolved through the original `use` declaration:
```velo
use std::console as out;

out::println("hello");
```

## Void return validation
The semantic analyzer validates empty and value returns:
- `return;` is valid only in `void` functions
- `return value;` is invalid in `void` functions
- non-void functions must return a value

## Final return requirement
For MVP semantic validation:
- non-void functions must end with `return`
- void functions are exempt

This is currently a structural check based on the final statement only.

## Local mutability
Local symbols now track:
- type
- mutability

This enables:
- immutable `let`
- mutable `var`
- reassignment validation

Future work:
- branch-aware control-flow analysis
- guaranteed return path analysis

## If / else validation
The semantic analyzer validates:
- if condition type must be `bool`
- if/else can satisfy final return requirement when both branches guarantee return