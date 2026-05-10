# Examples

This document lists useful examples available under `examples/`.

## Run

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