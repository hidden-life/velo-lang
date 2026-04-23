# Architecture overview
## Current layers
At the current stage Velo has the following layers:
1. source
2. diagnostic
3. lexer
4. AST
5. parser

## Current pipeline
```text
source text -> source model -> lexer -> tokens -> parser -> AST
```

## Notes
- The parser is handwritten and recursive descent.
- The AST is intentionally minimal and only covers the first executable program shape.
- Semantic analysis and bytecode generation are not implemented yet.