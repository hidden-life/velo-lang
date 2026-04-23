# Velo
It is an experimental programming language and runtime focused on API and backend development.

## Current status
Implemented so far:
- bootstrap CLI
- source file model
- source locations and ranges
- diagnostic engine
- handwritten lexer
- minimal AST for the first executable program
- minimal recursive descent parser
- GoogleTest test setup

## Build
```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

## Run
```bash
./build/debug/apps/velo/velo
```

## Current milestone
The project can now:
- load source text
- map offsets to line/column
- collect diagnostics
- tokenize basic Velo syntax
- build minimal AST
- parse the first executable Velo program shape

Supported parser shape:
- `module app;`
- `use std::console;`
- `use std::console as out;`
- `fn main(): int { ... }`
- `pub fn main(): int { ... }`
- expression statement with call expression
- `return` statement with integer literal

## Next milestone
Next step: add a simple AST printer and a compilation driver that runs source -> lexer -> parser.