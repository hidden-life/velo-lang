# Architecture overview
Velo currently uses a simple source-to-VM pipeline.

## Pipeline
```text
Source text 
    -> Lexer 
    -> Parser 
    -> AST 
    -> SemanticAnalyzer 
    -> IR lowerer
    -> Interpreter / VM 
    -> Runtime
```

## Main components
### Source
The source layer owns source files and source ranges.

Source ranges are used by diagnostics.

### Lexer
The lexer converts source text into tokens.

It supports:
- keywords
- identifiers
- integer literals
- string literals
- boolean literals
- punctuation
- operators
- comments

### Parser
The parser converts tokens into AST.

It currently uses a hand-written recursive descent parser.

### AST
The AST represents Velo source structure:
- program
- imports
- functions
- structs
- statements
- expressions
- type names
- qualified names

### Semantic analyzer
The semantic analyzer validates the AST before IR lowering.

It checks:
- imports
- functions
- entry point
- types
- scopes
- assignments
- function calls
- builtin calls
- return statements
- control-flow constraints
- builtin argument metadata
- struct declarations
- user-defined struct types
- struct literals
- field access expressions

### IR lowerer
The lowerer converts semantically valid AST into executable IR.

It resolves:
- local indexes
- function calls
- builtin calls
- import aliases
- control-flow jumps
- short-circuit logical expressions
- struct literals
- field access expressions

### Interpreter / VM
The interpreter executes IR instructions.

It owns:
- operand stack
- local slots
- current module pointer
- instruction pointer
- runtime calls

### Runtime
The runtime owns:
- builtin registry
- module registry

Builtin are registered in runtime and mirrored into module metadata for semantic analysis.

## CLI modes
The `velo` CLI supports multiple modes:
```bash
velo run file.velo
velo check file.velo
velo ast file.velo
velo ir file.velo
```

Driver modes:
- `Run`: lexer -> parser -> semantic -> IR lowerer -> interpreter
- `Check`: lexer -> parser -> semantic
- `Ast`: lexer -> parser -> semantic -> AST printer
- `Ir`: lexer -> parser -> semantic -> IR lowerer -> IR printer

## Future app separation
The repository reserves:
```text
apps/
    velo/
    veloc/
    velovm/
```

Planned roles:
- `velo`: high-level developer CLI
- `veloc`: future compiler frontend
- `velovm`: future standalone bytecode/IR runner

For MVP 0.1, active development remains focused on `apps/velo`.