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

## Lowering local scopes

The lowerer uses a stack of local scopes for name-to-local-index resolution.

Runtime local slots are still function-level slots, but source-level names are resolved using lexical scopes.

Example source:

```velo
let value: int = 1;

if (true) {
    let value: int = 2;
    return value;
}

return value;
```

Representative IR shape:

```text
PushInt 1
StoreLocal local[0]

PushBool true
JumpIfFalse ...

PushInt 2
StoreLocal local[1]
LoadLocal local[1]
Return

LoadLocal local[0]
Return
```

The two `value` declarations use different local slots.
After the nested block ends, name resolution returns to the outer `value`.

## Array literals
Array literals are lowered to `BuildArray`.

Example source:
```velo
let ids: []int = [1, 2, 3];
```

Representative IR:
```text
PushInt 1
PushInt 2
PushInt 3
BuildArray elements=3
StoreLocal local[0]
```

## Array indexing
Array indexing is lowered to `LoadIndex`.

Example source:

```velo
return ids[1];
```

Representative IR:

```text
LoadLocal local[0]
PushInt 1
LoadIndex
Return
```

Nested indexing:

```velo
return matrix[1][0];
```

Representative IR:

```text
LoadLocal local[0]
PushInt 1
LoadIndex
PushInt 0
LoadIndex
Return
```

## Array element assignment

Array element assignment is lowered to `StoreIndexPath`.

Example source:

```velo
ids[0] = 42;
```

Representative IR:

```text
PushInt 42
LoadLocal local[0]
PushInt 0
StoreIndexPath indexes=1
StoreLocal local[0]
```

Nested array assignment:

```velo
matrix[1][0] = 99;
```

Representative IR:

```text
PushInt 99
LoadLocal local[0]
PushInt 1
PushInt 0
StoreIndexPath indexes=2
StoreLocal local[0]
```

## Map literals
Map literals are lowered to `BuildMap`.

Example source:
```velo
let scores: map<string, int> = map {
    "Alex": 10,
    "Bob": 20
};
```

Representative IR:
```text
PushInt 10
PushInt 20
BuildMap entries=2
StoreLocal local[0]
```

## JSON builtins
JSON helpers are lowered as normal builtin calls.

Examples:
```text
CallBuiltin json::stringify args=1
CallBuiltin json::parse args=1
CallBuiltin json::get_int args=2
```

No dedicated JSON IR instruction exists in MVP 0.7.

## HTTP builtins
HTTP helpers are lowered as normal builtin calls.

Examples:
```text
CallBuiltin http::response args=2
CallBuiltin http::text_response args=2
CallBuiltin http::json_response args=2
CallBuiltin http::status args=1
CallBuiltin http::body args=1
CallBuiltin http::has_header args=2
CallBuiltin http::header args=2
CallBuiltin http::request args=3
CallBuiltin http::method args=1
CallBuiltin http::path args=1
CallBuiltin http::json_body args=1
```

No dedicated HTTP IR instruction exists in MVP 0.8.