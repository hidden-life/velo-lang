# Bytecode architecture
Velo bytecode is the VM-level representation introduced in MVP 0.5.

## Purpose
The existing IR is useful as compiler/interpreter representation.

Bytecode is intended to become a stable execution format for the future VM.

## Layers
Current pipeline:
```text
Source
Lexer
Parser
SemanticAnalyzer
IR lowerer
IR interpreter
```

Target pipeline:
```text
Source
Lexer
Parser
SemanticAnalyzer
IR lowerer
Bytecode Compiler
Bytecode VM
```

## Model
The bytecode model contains:
- `Bytecode::OpCode`
- `Bytecode::Instruction`
- `Bytecode::Function`
- `Bytecode::Module`

## MVP 0.5.1 opcodes
The initial bytecode opcode set covers the language/runtime features
available by the end of MVP 0.4:
- primitive values
- local variables
- integer arithmetic
- logical operations
- equality and comparison
- jumps
- function calls
- builtin calls
- structs
- arrays
- return
- pop

## Not implemented yet
MVP 0.5.1 does not execute bytecode yet.

Execution will be added later by the bytecode VM.

## IR to bytecode compiler
The bytecode compiler converts:
```text
IR::Module -> Bytecode::Module
```

The compiler is intentionally mechanical in MVP 0.5.2.

It maps IR opcodes to bytecode opcodes and copies instruction operands.

Preserved operands:
- `stringOperand`
- `intOperand`
- `boolOperand`
- `argsCount`
- `indexOperand`
- `targetOperand`

The compiler does not execute bytecode.

Execution will be implemented by the bytecode VM in a later MVP 0.5 step.

## Bytecode VM
The bytecode VM executes `Bytecode::Module`.

Main responsibilities:
- find `main`
- maintain operand stack
- maintain local slots
- execute bytecode instruction
- handle jumps
- call user-defined bytecode functions
- call runtime builtins
- return `Runtime::ExecutionResult`

## Execution model
The VM uses an instruction pointer per function.

Representative function execution loop:
```text
while ip < instructions.size:
    execute instruction
    handle Return
    handle Jump
    handle JumpIfFalse
    ip += 1
```

## Stack and locals
The VM uses:
- operand stack for temporary values
- local slots for function parameters and local variables

Value boundaries use `Runtime::cloneValue(...)` to preserve value semantics
for structs and arrays.

## Builtins
Bytecode builtin calls reuse the existing runtime builtin registry.

Example:
```text
CallBuiltin array::len args=1
```

## Current limitation
The VM exists as a separate execution path in MVP 0.5.3.

The main CLI still uses the existing IR interpreter path.

## Bytecode disassembler
The disassembler converts a `Bytecode::Module` into readable text.

Example:
```text
fn main
0000 PushInt 42
0001 Return
```

The disassembler is intended for:
- debugging compiler output
- comparing IR and bytecode shape
- CLI inspection
- future bytecode file debugging

## CLI bytecode mode
The CLI supports:
```bash
velo bytecode file.velo
```
and alias:
```bash
velo bc file.velo
```

This command compiles source to bytecode and prints bytecode disassembly.

It does not execute bytecode.

## Bytecode file format draft
MVP 0.5.5 introduces a text-based bytecode file format draft.

The format starts with a magic header and version:
```text
VELO_BYTECODE_TEXT 1
```

Functions are written as:
```text
function "main" 0
...
endfunction
```

Function parameters are written as:
```text
param "name"
```

Instructions are written as:
```text
instruction <opcode> <stringOperand> <intOperand> <boolOperand> <argsCount> <indexOperand> <targetOperand>
```

Example:
```text
VELO_BYTECODE_TEXT 1
function "main"
instruction PushInt 42 0 0 0 0
instruction Return 42 0 0 0 0
endfunction
```

This is a draft format.

Current goals:
- roundtrip bytecode modules in tests
- make bytecode inspectable
- prepare for future `veloc`
- prepare for future `velovm`

Current non-goals:
- compact binary encoding
- stable public bytecode ABI
- cross-version compatibility guarantees