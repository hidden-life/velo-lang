# Velo IR

Velo IR is the intermediate representation produced after semantic analysis.

## Current IR shape

```text
Module
  Function[]
    name
    parameters
    instructions
```

## Instruction

Each instruction has:

- `code`
- `stringOperand`
- `intOperand`
- `boolOperand`
- `argsCount`
- `indexOperand`
- `targetOperand`

The opcode field is named:

```cpp
code
```

not:

```cpp
op
```

## Example

Source:

```velo
fn main(): int {
    return (1 + 2) * 3;
}
```

Possible IR:

```text
IRModule
  Function main
    0: PushInt 1
    1: PushInt 2
    2: AddInt
    3: PushInt 3
    4: MulInt
    5: Return
```

## Constants

- `PushInt`
- `PushString`
- `PushBool`

## Locals

- `LoadLocal`
- `StoreLocal`
- `BuildStruct`
- `LoadField`

## Calls

- `CallBuiltin`
- `CallFunction`

Qualified calls are builtin calls:

```velo
console::println("hello");
```

Single-segment calls are user-defined function calls:

```velo
helper();
```

## Struct literals
Struct literals are lowered to:
```text
BuiltStruct <encoded-fields> args=<field-count>
```
The lowerer evaluates field initializer expressions first, then emits `BuildStruct`.

Example source:
```velo
let user: User = User {
    id: 42,
    name: "John Doe"
};
```
Representative IR shape:
```text
PushInt 42
PushString "John Doe"
BuildStruct User:id,name args=2
StoreLocal local[0]
```
The exact encoded operand is an implementation detail, bu it must contain enough
information for the interpreter to build a runtime struct value.

## Field access
Field access is lowered to `LoadField`.

Example source:
```velo
return user.id;
```

Representative IR:
```text
LoadLocal local[0]
LoadField id
Return
```

Chained field access:
```velo
return box.user.id;
```

Representative IR:
```text
LoadLocal local[0]
LoadField user
LoadField id
Return
```

## Field assignment
Field assignment is lowered using `StoreFieldPath`.

Example source:

```velo
user.id = 42;
```

Representative IR:

```text
PushInt 42
LoadLocal local[0]
StoreFieldPath id
StoreLocal local[0]
```

Nested field assignment:

```velo
box.user.id = 42;
```

Representative IR:

```text
PushInt 42
LoadLocal local[0]
StoreFieldPath user.id
StoreLocal local[0]
```

The operand of `StoreFieldPath` contains only the field path inside the root local value.
The root local itself is selected by the surrounding `LoadLocal` and `StoreLocal`.

## Import alias lowering

The lowerer resolves imported module aliases.

Example:

```velo
use std::string as str;

str::len("hello");
```

is lowered to:

```text
CallBuiltin string::len args=1
```

not:

```text
CallBuiltin str::len args=1
```

## Arithmetic

- `AddInt`
- `SubInt`
- `MulInt`
- `DivInt`
- `ModInt`
- `NegInt`

## Comparisons

- `CompareEqualInt` (legacy compatibility)
- `CompareNotEqualInt` (legacy compatibility)
- `CompareLessInt`
- `CompareGreaterInt`
- `CompareLessEqualInt`
- `CompareGreaterEqualInt`
- `CompareEqual`
- `CompareNotEqual`

## Logic

- `LogicalNot`
- `LogicalAnd`
- `LogicalOr`

High-level `&&` and `||` use short-circuit jump lowering.

## Control flow

- `JumpIfFalse`
- `Jump`
- `Return`

## Stack cleanup

- `Pop`

Expression statements emit `Pop` because their result is unused.

## IR dump mode

IR can be inspected with:

```bash
velo ir file.velo
```

This runs:

```text
lexer -> parser -> semantic -> lowerer -> IR printer
```

The program is not executed in IR mode.

## Struct construction
Struct literals lower to:
```text
PushInt 1
PushString "John Doe"
BuildStruct User:id,name fields=2
```
**BuildStruct** uses:
- `stringOperand`: encoded struct type and field names
- `argsCount`: number of field values on stack

Current encoding:
```text
TypeName:field1,field2,field3
```

This is an MVP representation and may be replaced by richer IR metadata later.

## Generic equality
The lowerer emits generic equality opcodes for `==` and `!=`:

```text
CompareEqual
CompareNotEqual
```

Example:

```velo
"Alex" == "Alex"
```

Representative IR:

```text
PushString "Alex"
PushString "Alex"
CompareEqual
```

The lowerer does not need semantic type information to choose a type-specific equality opcode.
Semantic analysis validates operands before lowering, and the interpreter compares actual runtime values.