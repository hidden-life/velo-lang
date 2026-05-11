# Runtime and VM

The Velo runtime owns builtin functions, module metadata, and VM execution support.

## Runtime

The runtime owns:

- `BuiltinRegistry`
- `ModuleRegistry`

Builtin functions are registered first.

Then module metadata is built from builtin metadata.

```text
BuiltinRegistry
  -> ModuleRegistry
```

This allows semantic analysis to resolve builtin modules and functions.

## Builtin functions

Builtin functions currently have:

- full name
- parameter types
- return type
- handler

Example signatures:

```text
console::println(any): void
string::len(string): int
int::toString(int): string
bool::toString(bool): string
```

## Builtin return values

Builtins return `ExecutionResult`.

Void builtins leave `returnValue` empty:

```text
console::println(value) -> void
```

Value-returning builtins set `returnValue`:

```text
string::len(value) -> int
```

After successful builtin execution, the interpreter pushes `returnValue` onto the operand stack when it is present.

## Builtin parameter metadata

Builtin functions declare parameter types.

Examples:

- `console::println(any): void`
- `string::len(string): int`
- `int::toString(int): string`
- `bool::toString(bool): string`

The runtime still validates builtin arguments defensively.

Semantic analysis also uses builtin metadata to catch invalid builtin calls before execution.

## Runtime values

Current runtime value type:

```cpp
std::variant<int, std::string, bool>
```

Supported runtime values:

- `int`
- `string`
- `bool`

## Interpreter

The interpreter executes IR instructions.

It uses:

- operand stack
- local slots
- instruction pointer
- current IR module pointer
- runtime builtin registry

## Locals

Function parameters and local variables are stored in local slots.

IR instructions:

```text
LoadLocal
StoreLocal
```

read and write local slots.

## Calls

User-defined function calls use:

```text
CallFunction
```

Builtin calls use:

```text
CallBuiltin
```

Qualified calls are lowered to builtin calls.

Single-segment calls are lowered to user-defined function calls.

## Program exit code

The interpreter returns `ExecutionResult`.

It contains:

- success flag
- exit code
- runtime error message
- optional return value

For `main(): int`, returning an integer produces the process exit code through `DriverResult.exitCode`.

## Integer arithmetic instructions

The VM supports integer arithmetic instructions:

- `AddInt`
- `SubInt`
- `MulInt`
- `DivInt`
- `ModInt`
- `NegInt`

Division and modulo by zero are runtime errors.

## Comparison instructions

The VM supports integer comparison instructions:

- `CompareEqualInt`
- `CompareNotEqualInt`
- `CompareLessInt`
- `CompareGreaterInt`
- `CompareLessEqualInt`
- `CompareGreaterEqualInt`

These instructions produce `bool`.

## Logical instructions

The VM supports:

- `LogicalNot`
- `LogicalAnd`
- `LogicalOr`

High-level `&&` and `||` are lowered with short-circuit jumps.

The eager logical opcodes may still be used by low-level tests or future compiler passes.

## Control flow

The VM supports:

- `JumpIfFalse`
- `Jump`

`if`, `while`, `break`, `continue`, `&&`, and `||` are lowered into jumps.

## Short-circuit logical lowering

`a && b`:

```text
a
JumpIfFalse false_branch
b
Jump end
false_branch:
PushBool false
end:
```

`a || b`:

```text
a
JumpIfFalse right_branch
PushBool true
Jump end
right_branch:
b
end:
```

This prevents unnecessary evaluation of the right-hand side.

## CLI run mode

Runtime execution happens only in `run` mode.

These modes do not execute the program:

- `check`
- `ast`
- `ir`

Therefore runtime errors such as division by zero are not triggered in those modes.

## Struct values
Runtime values now include struct values.

Conceptually:
```text
StructValue
    typeName
    fields
```
Struct values are stored through shared pointers inside `Runtime::Value`.

Current limitation:
- field access is not implemented yet