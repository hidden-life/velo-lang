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
0.5.5   bytecode file format draft
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

### Bytecode disassembler
The bytecode disassembler converts `Bytecode::Module` into readable text.

Example output:
```text
fn main
0000 PushInt 42
0001 Return
```

### CLI mode
MVP 0.5.4 adds:
```bash
velo bytecode file.velo
```
Alias:
```bash
velo bc file.velo
```
This mode uses the path:
```text
Source -> AST -> IR -> Bytecode -> Disassembler
```
It does not execute bytecode.

Not implemented in 0.5.4:
- bytecode file writer
- bytecode file reader
- standalone `veloc`
- standalone `velovm`
- switching default `run` to bytecode VM

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

## Bytecode file format draft
MVP 0.5.5 introduces a text-based bytecode file format draft.

Example:
```text
VELO_BYTECODE_TEXT 1
function "main" 0
instruction PushInt "" 42 0 0 0 0
instruction Return "" 42 0 0 0 0
endfunction
```

Instruction line format:
```text
instruction <opcode> <stringOperand> <intOperand> <boolOperand> <argsCount> <indexOperand> <targetOperand>
```
The format is intentionally simple and text-based.

It is not a stable binary format yet.

Not implemented in 0.5.5:
- stable binary bytecode format
- `.vbc` file extension policy
- bytecode file writer CLI
- bytecode file reader CLI
- standalone `veloc`
- standalone `velovm`
- switching default `run` to bytecode VM