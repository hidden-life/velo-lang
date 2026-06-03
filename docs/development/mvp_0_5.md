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
0.5.1   bytecode instruction model
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

## Not implemented in 0.5.1
- IR to bytecode compiler
- bytecode VM
- bytecode disassembler
- bytecode CLI mode
- bytecode file writer
- bytecode file reader
- standalone `veloc`
- standalone `velovm`