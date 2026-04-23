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
- AST printer
- first driver for the source -> lexer -> parser pipeline
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
- print AST from the CLI

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
Next step: add a real example `.velo` file and start preparing a semantic layer.