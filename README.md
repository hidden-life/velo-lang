# Velo
Velo is an experimental programming language and runtime focused on API and backend development.

## Current status
Implemented so far:
- bootstrap CLI
- source file model
- source locations and ranges
- diagnostic engine
- handwritten lexer
- minimal AST for the first executable program
- minimal recursive descent parser
- minimal semantic analyzer
- AST printer
- first driver for the source -> lexer -> parser -> semantic pipeline
- GoogleTest test setup

## Build
```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

## Run
```bash
./build/debug/apps/velo/velo ./examples/hello_world/main.velo
```

## Current milestone
The project can now:
- load source text
- map offsets to line/column
- collect diagnostics
- tokenize basic Velo syntax
- build minimal AST
- parse the first executable Velo program shape
- run minimal semantic validation
- print AST from the CLI

Current semantic checks:
- `main` must exist
- `main` must have no parameters
- `main` must return `int`
- visible import names must be unique
- function names must be unique
- qualified call roots must resolve through `use`

Example output:
```
Program
    Module app
    Use std::console
    Function main -> int
        ExprStmt
            Call console::println
                String "Hello, Velo!"
        Return
            Integer 0
```

## Next milestone
Next step: start lowering the validated AST into a very small executable representation.