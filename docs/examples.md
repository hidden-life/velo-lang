# Examples

This document lists useful examples available under `examples/`.

## Guidelines
Example style is documented here:
- [Example guidelines](development/example_guidelines.md)

General rule:
- valid demo examples should return `0`
- non-zero return values should usually be asserted in driver tests

## Run
Demo examples should usually exit with code `0`.
```bash
./build/debug/apps/velo/velo run ./examples/arithmetic/main.velo
```

## Check

```bash
./build/debug/apps/velo/velo check ./examples/arithmetic/main.velo
```

## Print AST

```bash
./build/debug/apps/velo/velo ast ./examples/arithmetic/main.velo
```

## Print IR

```bash
./build/debug/apps/velo/velo ir ./examples/arithmetic/main.velo
```

## MVP showcase
```bash
./build/debug/apps/velo/velo run ./examples/mvp_showcase/main.velo
```

## Current examples

### Arithmetic

```text
examples/arithmetic/main.velo
```

Demonstrates:

- arithmetic operators
- grouped expressions
- local variables
- `console::println`

### Logical operators

```text
examples/logical/main.velo
```

Demonstrates:

- comparison operators
- logical operators
- `if`

### Short-circuit

```text
examples/short_circuit/main.velo
```

Demonstrates:

- short-circuit `&&`
- short-circuit `||`
- avoiding runtime evaluation of the right-hand side

### While loop

```text
examples/while_loop/main.velo
```

Demonstrates:

- mutable locals
- `while`
- assignment

### Break / continue

```text
examples/break_continue/main.velo
```

Demonstrates:

- `break`
- `continue`
- loop control flow

### Typed parameters

```text
examples/typed_parameters/main.velo
```

Demonstrates:

- typed function parameters
- `string` parameters
- `bool` parameters
- user-defined function calls

### Standard string module

```text
examples/std_string_len/main.velo
```

Demonstrates:

- `std::string`
- import alias
- `str::len`

### Standard toString modules

```text
examples/std_to_string/main.velo
```

Demonstrates:

- `std::int::toString`
- `std::bool::toString`
- printing converted values

### Error examples

```text
examples/errors/
```

Contains intentionally invalid programs used to demonstrate diagnostics.

### Struct declarations
```text
examples/struct_declaration/main.velo
```
Demonstrates:
- `struct`
- `pub struct`
- public fields
- private fields
- field type validation

### User-defined type registry
```text
examples/user_defined_type_registry/main.velo
```
Demonstrates:
- struct field using another struct type
- user-defined type registry
- public struct with user-defined field type

### Struct type usage
```text
examples/struct_type_usage/main.velo
```
Demonstrates:
- struct type in function parameter
- struct type as function return type
- struct type in local declaration
- semantic checking of named struct types

### Struct literals
```text
examples/struct_literal/main.velo
```
Demonstrates:
- struct literal syntax
- struct value creation
- passing struct values to functions
- returning struct values from functions

### Field access
```text
examples/field_access/main.velo
```
Demonstrates:
- struct literal creation
- reading `int`, `string`, and `bool` fields
- field access expressions
- printing field values with `console::println`

### Field assignment
```text
examples/field_assignment/main.velo
```

Demonstrates:

- assigning to struct fields
- assigning to `int` fields
- assigning to `string` fields
- preserving struct value semantics when assigning through a copied struct value
- printing original and modified values with `console::println`

### MVP showcase
```text
examples/mvp_showcase/main.velo
```
Demonstrates the current core language feature set:
- imports
- aliases
- user functions
- typed parameters
- typed return values
- mutable locals
- while loop
- if
- comparisons
- logical operators
- standard modules

### Equality

```text
examples/equality/main.velo
```

Demonstrates:

- string equality
- string inequality
- bool equality
- bool inequality
- comparing struct fields of type `string`
- comparing struct fields of type `bool`
- printing bool results with `console::println`

### Block scopes
```text
examples/block_scopes/main.velo
```

Demonstrates:

- local shadowing inside `if`
- local shadowing inside `while`
- block-local variables not overwriting outer local variables
- outer local visibility after nested blocks