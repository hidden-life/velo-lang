# Adding language features
This guide describes the recommended process for adding a new Velo language feature.

Use this guide for features such as:
- new operator
- new statement
- new expression
- new syntax form
- new semantic rule
- new IR behavior

## General rule
Add features through the full pipeline:
```text
Lexer
    -> Parser
    -> AST
    -> SemanticAnalyzer
    -> IR Lowerer
    -> Interpreter / Runtime
    -> CLI / Driver
    -> Tests
    -> Docs
    -> Examples
```

Not every feature touches every layer, but every layer should be considered.

## Step 1. Define syntax
Before writing code, define examples.

Example:
```velo
while(x < 10) {
    x = x + 1;
}
```

Also defined invalid examples:
```velo
break;
```
if `break` is only valid inside loops.

## Step 2. Lexer
If the feature introduces new keywords or operators, update lexer.

Typical files:
```text
include/velo/lexer/token_kind.h
src/lexer/token_kind.cpp
src/lexer/lexer.cpp
tests/unit/lexer/
```

Checklist:
- add `TokenKind`
- update `toString(TokenKind)`
- update `Lexer::nextToken()`
- add lexer test

Important project-specific detail:
```text
Lexer uses nextToken() and makeToken(...)
```
Do not invent `addToken(...)`.

## Step 3. AST
If the feature needs a new syntax node, update AST.

Typical file:
```text
include/velo/ast/ast.h
```

Checklist:
- add `StatementKind` or `ExpressionKind`
- add AST node struct
- add fields with ownership rules
- keep `std::unique_ptr` ownership consistent

Example:
```cpp
struct BreakStatement final: Statement {
    explicit BreakStatement(Source::SourceRange range) :
        Statement(StatementKind::Break, range) {}
};
```

## Step 4. Parser
Typical files:
```text
include/velo/parser/parser.h
src/parser/parser.cpp
tests/unit/parser/
```

Checklist:
- add parser method if needed
- wire into existing parse chain
- preserve precedence rules
- consume required punctuation
- use existing diagnostic style
- add parser tests

Expression precedence should remain explicit.

Current expression chain includes logical, comparison, arithmetic, unary, and primary layers.

## Step 5. AST printer
Typical files:
```text
src/ast/ast_printer.cpp
```

Checklist:
- print new statement/expression kind
- include enough detail to debug parser output
- add/update AST printer test if needed

## Step 6. Semantic analyzer
Typical files:
```text
include/velo/semantic/semantic_analyzer.h
src/semantic/semantic_analyzer.cpp
tests/unit/semantic/
```

Checklist:
- validate type rules
- validate scope rules
- validate control-flow rules
- avoid cascading diagnostics
- add positive semantic test
- add negative semantic test

Use semantic diagnostics for invalid source-level programs.

Runtime should not be the first place that catches source-level mistakes when
semantic can catch them.

## Step 7. IR lowerer
Typical files:
```text
include/velo/ir/lowerer.h
src/ir/lowerer.cpp
tests/unit/driver/
```

Checklist:
- lower new AST node to IR
- preserve source-level behavior
- handle aliases if the feature touches qualified names
- add IR dump test when useful

Project-specific details:
```text
Instruction field is named code, not op.
```

Example:
```cpp
Instruction {
    .code = OpCode::PushInt,
    .intOperand = 1
};
```

## Step 8. IR instruction
If the feature needs new VM behavior, update IR.

Typical files:
```text
include/velo/ir/instruction.h
src/ir/ir_printer.cpp
docs/architecture/ir.md
```

Checklist:
- add `OpCode`
- update IR printer
- update interpreter
- add interpreter test
- update IR docs

## Step 9. Interpreter / VM
Typical files:
```text
include/velo/interpreter/interpreter.h
src/interpreter/interpreter.cpp
tests/unit/interpreter/
```

Checklist:
- implement opcode execution
- validate stack shape
- validate runtime value types
- return `ExecutionResult` on runtime errors
- add VM-level tests

Runtime failures should return:
```cpp
ExecutionResult {
    .success = false,
    .exitCode = 1,
    .error = "..."
};
```

## Step 10. Driver tests
Typical files:
```text
tests/unit/driver/driver_test.cpp
```

Driver tests validates the full pipeline:
```text
source -> lexer -> parser -> semantic -> lowerer -> interpreter
```

Use `DriverResult.exitCode` for execution assertions.

Example:
```cpp
EXPECT_EQ(result.exitCode, 42);
```

Use `DriverMode::Ir` for checking generated IR.

Example:
```cpp
EXPECT_NE(result.irText.find("CallBuiltin string::len args=1"), std::string::npos);
```

## Step 11. CLI
If the feature affects CLI behavior, update:
```text
apps/velo/main.cpp
```

Current CLI modes:
```bash
velo run file.velo
velo check file.velo
velo ast file.velo
velo ir file.velo
```

## Step 12. Examples
Add or update examples under:
```text
examples/
```

Examples should usually return `0` so shell treats them as successful commands.

If you need to assert returned value, do that in driver tests instead.

## Step 13. Documentation

Update relevant docs:
```text
docs/language/syntax.md
docs/language/types.md
docs/language/modules.md
docs/architecture/overview.md
docs/architecture/semantic.md
docs/architecture/runtime.md
docs/architecture/ir.md
docs/examples.md
docs/development/mvp_0_1.md
```

README should stay high-level.

## Recommended test matrix
For a new language feature, add tests in this order:
1. lexer test
2. parser test
3. semantic positive test
4. semantic negative test
5. interpreter test if VM behavior changed
6. driver test
7. IR dump test if lowering is non-trivial

## Example: adding an operator
For a new binary operator:
```text
TokenKind
Lexer::nextToken()
AST::BinaryOperator
Parser precedence layer
AST printer
Semantic type validation
OpCode
Lowerer
Interpreter
IR printer
Lexer test
Parser test
Semantic tests
Interpreter test
Driver test
Docs
Example
```

## Example: adding a statement
For a new statement:
```text
TokenKind if keyword
AST::StatementKind
Statement AST node
Parser parseStatement()
AST printer
Semantic validation
Lowerer
Interpreter behavior if needed
Parser test
Semantic tests
Driver test
Docs
Example
```

## Common mistakes
### Adding parser support without semantic validation
Bad:
```text
parser accepts syntax
semantic ignores it
lowerer assumes it is valid
```

Good:
```text
parser accepts syntax
semantic validate rules
lowerer assumes semantic validity
```

### Adding IR opcode without IR printer support.
If a new opcode exists, `velo ir` should be able to print it.

### Adding runtime behavior without driver test
Interpreter tests are useful, but driver tests prove the full pipeline works.

### Forgetting docs
Every feature should update docs in the same commit or immediately after.

## Commit message format
Suggested formats:
```text
feat(parser): add <feature>
feat(semantic): validate <feature>
feat(ir): lower <feature>
feat(runtime): execute <feature>
feat(language): add <feature>
```

For full feature slices:
```text
feat(expressions): add integer arithmetic operators
feat(control-flow): add break and continue statements
```