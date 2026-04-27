# Architecture overview
## Current layers
At the current stage Velo has the following layers:
1. source
2. diagnostic
3. lexer
4. AST
5. parser
6. semantic
7. driver
8. CLI
9. IR
10. interpreter
11. runtime

## Current pipeline
```text
source text 
    -> source model 
    -> lexer 
    -> tokens 
    -> parser 
    -> AST 
    -> semantic checks 
    -> IR 
    -> interpreter 
    -> runtime builtins
```

## Notes
- The parser is handwritten and recursive descent.
- The AST is intentionally minimal and only covers the first executable program shape.
- The semantic layer is intentionally small and currently validates only the entry point and import-based name resolution.
- Bytecode generation and execution are not implemented yet.
- The interpreter can now execute user-defined function calls without parameters.
- Calls with arguments are still limited to builtins.
- Expressions statements now emit a Pop instruction to prevent stack pollution.