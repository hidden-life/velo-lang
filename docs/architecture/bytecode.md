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