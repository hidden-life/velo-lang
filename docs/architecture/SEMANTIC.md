# Semantic layer
## Current responsibilities
The current semantic layer performs only a minimal set of checks:
1. verifies that `main` exists
2. verifies that `main` has no parameters
3. verifies that `main` returns `int`
4. validates duplicate visible import names
5. validates duplicate function declarations
6. resolves qualified names in expressions through `use`

## Current limitations
The current semantic layer does not yet implement:
- local scopes
- variable resolution
- type checking for expressions
- return type validation inside function bodies
- module registry
- standard library symbol validation