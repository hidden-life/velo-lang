# Velo MVP 0.5 status
MVP 0.5 focuses on VM and bytecode foundation.

The goal is to introduce a dedicated bytecode layer without breaking the existing
IR interpreter pipeline.

## Roadmap
```text
0.5.1   bytecode instruction model
0.5.2   IR to bytecode compiler
0.5.3   bytecode VM execution
0.5.4   bytecode disassembler and CLI mode
0.5.5   bytecode file format draft
0.5.6   docs/examples/release checklist
```

## Current step
```text
0.5.3   bytecode VM execution
```

## 0.5.1 scope
Implemented in this step:
- `Velo::Bytecode` namespace
- bytecode opcode model
- bytecode instruction model
- bytecode function model
- bytecode module model
- opcode string conversion helper
- bytecode unit tests
- documentation

## Bytecode model
MVP 0.5 introduces a new bytecode-level representation.

Current pipeline before MVP 0.5:
```text
Source -> Lexer -> Parser -> SemanticAnalyzer -> IR lowerer -> IR interpreter
```

Target pipeline during MVP 0.5:
```text
Source -> Lexer -> Parser -> SemanticAnalyzer -> IR lowerer -> Bytecode Compiler -> Bytecode VM
```
MVP 0.5 keeps the current IR interpreter intact while the bytecode path is added
step by step.

Not implemented in 0.5.1
- IR to bytecode compiler
- bytecode VM
- bytecode disassembler
- bytecode CLI mode
- bytecode file writer
- bytecode file reader
- standalone `veloc`
- standalone `velovm`

## 0.5.2 scope
Implemented in this step:
- bytecode compiler class
- `IR::Module` to `Bytecode::Module` compilation
- IR function metadata
- IR instruction opcode mapping
- IR instruction operand copying
- support for arithmetic instructions
- support for logical instructions
- support for comparison instructions
- support for jumps
- support for function calls
- support for builtin calls
- support for struct instructions
- support for array instructions
- bytecode compiler unit tests

## IR to bytecode compiler
The compiler converts the current IR representation into bytecode representation.

```text
IR::Module -> Bytecode::Module
```

The compiler preserves:
- function names
- function parameters
- instruction order
- instruction operands
- jump targets
- argument counts
- local indexes

Not implemented in 0.5.2:
- bytecode VM
- bytecode execution
- bytecode disassembler
- bytecode CLI mode
- bytecode file writer
- bytecode file reader
- standalone `veloc`
- standalone `velovm`

## 0.5.3 scope
Implemented in this step:
- bytecode VM class
- bytecode module execution
- bytecode `main` lookup
- bytecode instruction execution lookup
- bytecode operand stack
- bytecode local slots
- bytecode jumps
- bytecode user-defined function calls
- bytecode arithmetic execution
- bytecode logical execution
- bytecode comparison execution
- bytecode struct execution support
- bytecode array execution support
- VM unit tests
- IR-to-bytecode-to-VM smoke test

## Bytecode VM
The bytecode VM executes `Bytecode::Module`.

```text
Bytecode::Module -> Bytecode::VM -> Runtime::ExecutionResult
```

The VM is intentionally separate from the current IR interpreter.

The existing runtime path remains available:
```text
IR::Module -> IR::Interpreter
```

The new bytecode path is:
```text
IR::Module -> Bytecode::Compiler -> Bytecode::Module -> Bytecode::VM
```

Not implemented in 0.5.3:
- bytecode disassembler
- bytecode CLI mode
- bytecode file writer
- bytecode file reader
- standalone `veloc`
- standalone `velovm`
- switching the default driver execution path to bytecode VM