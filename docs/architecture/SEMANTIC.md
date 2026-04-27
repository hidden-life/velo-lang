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