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
0.5.6   docs/examples/release checklist
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

## 0.5.4 scope
Implemented in this step:
- bytecode disassembler
- bytecode text output
- driver bytecode mode
- CLI command `bytecode`
- CLI alias `bc`
- disassembler unit tests
- driver bytecode mode tests
- documentation

## 0.5.5 scope
Implemented in this step:
- bytecode text file format draft
- bytecode file magic
- bytecode file version
- bytecode module writer
- bytecode module reader
- bytecode instruction roundtrip tests
- bytecode VM execution from round-tripped module
- invalid file format tests
- documentation

## 0.5.6 scope
Implemented in this step:
- MVP 0.5 documentation review
- bytecode architecture documentation review
- release checklist update
- release notes draft
- README current status update
- smoke-test command list
- tag instructions

## MVP 0.5 completed feature set
MVP 0.5 includes:
- all MVP 0.1 language foundation
- all MVP 0.2 data model foundation
- all MVP 0.3  value semantics and block scope foundation
- all MVP 0.4 arrays foundation
- bytecode instruction model
- bytecode function model
- bytecode module model
- IR to bytecode compiler
- bytecode VM
- bytecode disassembler
- CLI bytecode mode
- CLI bytecode alias
- bytecode text file format draft
- bytecode writer
- bytecode reader
- bytecode roundtrip tests

## Current execution paths
Default run path:
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

Bytecode file format test path:
```text
Bytecode::Module -> FileFormat::write -> text -> FileFormat::read -> Bytecode::Module
```

## MVP 0.5 limitations
Not implemented in MVP 0.5:
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

## Next milestone
The next planned milestone is:
```text
MVP 0.6 maps / object literals
```