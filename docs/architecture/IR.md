# Velo IR
Velo IR is the intermediate representation produced after semantic analysis.

Current IR shape:
- `Module`
    - list of functions
- `Function`
    - name
    - parameters
    - instructions
- `Instruction`
    - opcode
    - optional operands

## Example
```velo
fn main(): int {
    return (1 + 2) * 3;
}
```

Possible IR dump:
```text
IRModule
    Function main
        0. PushInt 1
        1. PushInt 2
        2. AddInt
        3. PushInt 3
        4. MulInt
        5. Return
```

## Current instruction categories
#### Constants
- `PushInt`
- `PushString`
- `PushBool`

##### Locals
- `LoadlLocal`
- `StoreLocal`

##### Calls
- `CallBuiltin`
- `CallFunction`

##### Arithmetic
- `AddInt`
- `SubInt`
- `MulInt`
- `DivInt`
- `ModInt`
- `NegInt`

##### Comparisons
- `CompareEqualInt`
- `CompareNotEqualInt`
- `CompareLessInt`
- `CompareGreaterInt`
- `CompareLessEqualInt`
- `CompareGreaterEqualInt`

##### Logic
- `LogicalNot`
- `LogicalAnd`
- `LogicalOr`

**Note:** high-level `&&` and `||` are lowered with short-circuit jumps.
The eager logical opcodes may still be used by low-level tests or future compiler passes.

##### Control-flow
- `JumpIfFalse`
- `Jump`
- `Return`

##### Stack cleanup
- `Pop`