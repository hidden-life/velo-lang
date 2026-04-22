# Velo
It is an experimental programming language and runtime focused on API and backend development.

## Current status
Implemented so far:
- bootstrap CLI
- source file model
- source locations and ranges
- diagnostic engine
- handwritten lexer
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

Supported lexical elements:
- keywords: `module`, `use`, `as`, `fn`, `return`, `pub`
- identifiers
- integer literals
- string literals
- punctuation: `::`, `:`, `;`, `,`, `(`, `)`, `{`, `}`
- line comments: `// ...`

## Next milestone
Next step: implement AST and parser for the first executable Velo program.