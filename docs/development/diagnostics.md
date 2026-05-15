# Diagnostics

This document describes diagnostic conventions in Velo.

## Goal

Diagnostics should be:

- precise
- stable
- tied to source ranges
- understandable
- minimally cascading

## Diagnostic format

CLI output currently follows this general format:

```text
error[SEM007] 7:12-7:18 Unknown symbol 'value'.
```

Parts:

```text
severity[code] startLine:startColumn-endLine:endColumn message
```

## Severity

Current severities:

- `error`
- `warning`
- `note`

Most diagnostics are currently errors.

## Code prefixes

Recommended diagnostic prefixes:

```text
LEXxxx  lexer diagnostics
PARxxx  parser diagnostics
SEMxxx  semantic diagnostics
RUNxxx  future runtime diagnostics
```

Runtime currently reports plain error strings through `ExecutionResult::error`.

## Current semantic diagnostics

Common semantic diagnostics include:

```text
SEM007  Unknown symbol
SEM010  Wrong builtin/function arity
SEM014  Return type mismatch
SEM015  Non-void function must return a value
SEM016  Void function must not return a value
SEM017  Non-void function must end with return
SEM018  Duplicate local variable
SEM019  Local variable initializer type mismatch
SEM022  Assignment type mismatch
SEM023  If condition must be bool
SEM025  While condition must be bool
SEM026  break used outside loop
SEM027  continue used outside loop
SEM028  Operator ! requires bool operand
SEM029  Logical operator requires bool operands
SEM030  Unknown declared type
SEM031  void cannot be used as value type
SEM032  Unary - requires int operand
SEM033  Builtin argument type mismatch
SEM034  Duplicate struct declaration
SEM035  Duplicate struct field
SEM036  Struct name conflicts with built-in type
SEM037  User-defined function argument type mismatch
SEM038  Unknown struct literal type
SEM039  Duplicate field in struct literal
SEM040  Unknown field in struct literal
SEM041  Struct field initializer type mismatch
SEM042  Missing field in struct literal
SEM043  Field access target must be struct
SEM044  Unknown field in struct field access
```

This list should be updated when adding new diagnostics.

## Avoiding cascading diagnostics

When one expression already produced a diagnostic, later checks should avoid adding noisy follow-up diagnostics.

Example:

```velo
fn main(): int {
    return missing;
}
```

Preferred:

```text
SEM007 Unknown symbol 'missing'
```

Avoid also producing:

```text
SEM014 Return type mismatch
```

unless it adds useful information.

## Checked expression analysis

The semantic analyzer uses checked expression analysis to run structural/name validation before type validation.

Conceptually:

```text
analyzeExpression(expr)
if no new errors:
    analyzeExpressionType(expr)
else:
    return Unknown
```

This avoids duplicated or cascading diagnostics.

## Unknown type

`ExpressionType::Unknown` is used internally to prevent cascades.

If an expression has unknown type because of an earlier error, later type checks should usually skip mismatch diagnostics.

Example rule:

```cpp
if (
    expected != ExpressionType::Unknown &&
    actual != ExpressionType::Unknown &&
    actual != expected
) {
    report mismatch
}
```

## Source ranges

Diagnostics should use the most specific useful range.

Examples:

- unknown variable: name range
- invalid argument: argument expression range
- wrong return type: return statement range or expression range
- invalid declared type: type name range

## Parser diagnostics

Parser diagnostics should explain what was expected.

Good:

```text
Expected ')' after grouped expression.
```

Bad:

```text
Unexpected token.
```

## Semantic diagnostics

Semantic diagnostics should describe the violated rule.

Good:

```text
Builtin argument type mismatch. Expected 'string', actual 'int'.
```

Bad:

```text
Bad argument.
```

## Runtime errors

Runtime errors currently go through:

```cpp
ExecutionResult {
    .success = false,
    .exitCode = 1,
    .error = "..."
}
```

Examples:

```text
Division by zero.
Modulo by zero.
Local index is out of range.
Unknown builtin function: string::missing
```

Future improvement:

- add structured runtime diagnostics
- add source mapping from IR back to AST/source ranges

## Adding a new diagnostic

When adding a new diagnostic:

1. choose a stable code
2. use the correct prefix
3. make the message specific
4. attach the best source range
5. add a negative test
6. update this document

## Testing diagnostics

A diagnostic test should usually check:

```cpp
ASSERT_TRUE(diagnosticEngine.hasErrors());
EXPECT_EQ(diagnosticEngine.diagnostics().front().code(), "SEM033");
```

When debugging a failing test, temporarily print diagnostics:

```cpp
for (const auto& diagnostic : diagnosticEngine.diagnostics()) {
    ADD_FAILURE()
        << diagnostic.code()
        << ": "
        << diagnostic.message();
}
```

For driver tests:

```cpp
if (!result.success) {
    ADD_FAILURE() << "Driver error: " << result.error;

    for (const auto& diagnostic : result.diagnostics) {
        ADD_FAILURE()
            << diagnostic.code()
            << ": "
            << diagnostic.message();
    }
}
```