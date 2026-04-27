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
- minimal runtime layer
- builtin function registry
- registered `console::println` builtin
- interpreter execution result model
- user-defined function calls without parameters
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
- lower AST into minimal IR
- execute minimal IR through the interpreter
- call registered builtin functions through the runtime registry
- lower user-defined function calls into IR
- execute user-defined function calls through the interpreter
- prevents stack pollution via Pop instruction

Current semantic checks:
- `main` must exist
- `main` must have no parameters
- `main` must return `int`
- visible import names must be unique
- function names must be unique
- qualified call roots must resolve through `use`
- builtin function argument count must match module metadata

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

## Examples
```bash
./build/debug/apps/velo/velo ./examples/hello_world/main.velo
./build/debug/apps/velo/velo ./examples/user_funtion/main.velo
```

## Next milestone
Next step: start lowering the validated AST into a very small executable representation.