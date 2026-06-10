# Velo v0.5.0 release notes draft
Velo MVP 0.5 focuses on VM and bytecode foundation.

It introduces a dedicated bytecode layer while keeping the existing IR interpreter
pipeline intact.

## Highlights
- bytecode instruction model
- bytecode function model
- bytecode module model
- IR to bytecode compiler
- bytecode VM
- bytecode disassembler
- CLI bytecode mode
- CLI `bc` alias
- bytecode text file format draft
- bytecode writer
- bytecode reader
- bytecode roundtrip tests

## Bytecode model
MVP 0.5 introduces the `Velo::Bytecode` namespace.

Core model:
```cpp
Velo::Bytecode::OpCode
Velo::Bytecode::Instruction
Velo::Bytecode::Function
Velo::Bytecode::Module
```

The bytecode opcode set covers the language features available by the end
of MVP 0.4:
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

## IR to bytecode compiler
MVP 0.5 adds:
```cpp
Velo::Bytecode::Compiler
```

The compiler converts:
```text
IR::Module -> Bytecode::Module
```

It preserves:
- function names
- function parameters
- instruction order
- instruction operands
- jump targets
- argument counts
- local indexes

## Bytecode VM
MVP 0.5 adds:
```c++
Velo::Bytecode::VM
```

The VM executes:
```text
Bytecode::Module -> Runtime::ExecutionResult
```

The VM supports:
- operand stack
- local slots
- jumps
- user-defined function calls
- builtin calls
- arithmetic operations
- logical operations
- comparisons
- structs
- arrays

## Bytecode disassembler
MVP 0.5 adds:
```c++
Velo::Bytecode::Disassembler
```

Example output:
```text
fn main
0000 PushInt 42
0001 Return
```

## CLI bytecode mode
MVP 0.5 adds:
```bash
velo bytecode file.velo
```

Alias:
```bash
velo bc file.velo
```

This command compiles source to bytecode and prints bytecode disassembly.

It does not execute bytecode.

## Bytecode file format draft
MVP 0.5 adds a text-based bytecode file format draft.

Example:
```text
VELO_BYTECODE_TEXT 1
function "main"
instruction PushInt 42 0 0 0 0
instruction Return 42 0 0 0 0
endfunction
```

Instruction line format:
```text
instruction <opcode> <stringOperand> <intOperand> <boolOperand> <argsCount> <indexOperand> <targetOperand>
```

This is not a stable binary format.

Current goals:
- roundtrip bytecode modules in tests
- make bytecode inspectable
- prepare for future `veloc`
- prepare for future `velovm`

## Current pipeline
Default execution path remains:
```text
Source -> Lexer -> Parser -> SemanticAnalyzer -> IR lowerer -> IR interpreter
```

Bytecode inspection path:
```text
Source -> Lexer -> Parser -> SemanticAnalyzer -> IR lowerer -> Bytecode Compiler -> Bytecode Disassembler
```

Bytecode VM test path:
```text
Bytecode::Module -> Bytecode::VM
```

## Tests
MVP 0.5 adds tests for:
- bytecode model
- bytecode compiler
- bytecode VM
- bytecode disassembler
- bytecode driver mode
- bytecode file format
- bytecode roundtrip
- VM execution from round-tripped bytecode module

## Known limitations
MVP 0.5 does not include:
- default `run` through bytecode VM
- stable binary bytecode format
- stable bytecode ABI
- bytecode optimizer
- bytecode verifier
- bytecode file writer CLI
- bytecode file reader CLI
- standalone `veloc`
- standalone `velovm`
- maps
- nullable types
- generics
- methods
- iterators
- `for` loops
- HTTP runtime
- JSON parser/serializer

## Next
The next planned milestone:
```text
MVP 0.6 maps / object literals
```