# Adding builtin
This guide describes how to add a new runtime builtin to Velo.

Use checklist when adding functions such as:
```text
string::len(string): int
int::toString(int): string
bool::toString(bool): string
```

## Current builtin model
A builtin currently has:
- full runtime name
- parameter type metadata
- return type metadata
- runtime handler

Example signature:
```text
string::len(string): int
```

Runtime name:
```text
string::len
```

Imported is source as:
```velo
use std::string as str;

str::len("hello");
```

Lowered IR call:
```text
CallBuiltin string::len args=1
```

## Files usually touched
Typical files:
```text
include/velo/runtime.h
src/runtime/runtime.cpp

include/velo/runtime/builtin_function.h
src/runtime/builtin_function.cpp

include/velo/module/module_symbol.h
src/module/module_symbol.cpp

tests/unit/runtime/
tests/unit/interpreter/
tests/unit/semantic/
tests/unit/driver/

examples/

docs/language/modules.md
docs/architecture/runtime.md
docs/architecture/semantic.md
```

For most new builtins, `BuiltinFunction` and `ModuleSymbol` do not need changes
anymore, because parameter metadata and return values are already supported.

### Step 1. Add registration method
In:
```text
include/velo/runtime/runtime.h
```
Add a private registration method:
```cpp
void registerStdExample();
```

Example:
```cpp
private:
    void registerStdConsole();
    void registerStdString();
    void registerStdInt();
    void registerStdBool();
    void registerStdExample(); // <- new method
```

### Step 2. Call registration from Runtime constructor
In:
```text
src/runtime/runtime.cpp
```

Register all builtins before building module metadata:
```cpp
Runtime::Runtime() {
    void registerStdConsole();
    void registerStdString();
    void registerStdInt();
    void registerStdBool();
    void registerStdExample(); // <- here
    
    buildModulesFromBuultins();
}
```

**Important**:
```text
buildModulesFromBuiltins();
```
must be called after builtin registration.

### Step 3. Register builtin function
Example:
```cpp
void Runtime::registerStdExample() {
    _registry.registerFunc(
        BuiltinFunction {
            "example::identity",
            {"string"},
            "string",
            [](const std::vector<Value> &args) -> ExecutionResult {
                if (args.size() != 1U) {
                    return ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "example::identity expects exactly one argument."
                    };
                }

                const auto &value = args.front();

                if (!std::holds_alternative<std::string>(value)) {
                    return ExecutionResult {
                        .success = false,
                        .exitCode = 1,
                        .error = "example::identity expects a string argument."
                    };
                }

                return ExecutionResult {
                    .success = true,
                    .exitCode = 0,
                    .error = {},
                    .returnValue = std::get<std::string>(value)
                };
            }
        }
    );
}
```

## Builtin constructor
Current shape:
```cpp
BuiltinFunction {
    "module::function",
    {"parameterType"},
    "returnType",
    handler
}
```

Examples:
```cpp
BuiltinFunction {
    "console::println",
    {"any"},
    "void",
    handler
}
```

```cpp
BuiltinFunction {
    "string::len",
    {"string"},
    "int",
    handler
}
```

## Supported parameter metadata types
Current metadata strings:
```text
int
string
bool
any
```

`any` accepts all current runtime value types.

Used by:
```text
console::println(any): void;
```

## Return values
Void builtin:
```cpp
return ExecutionResult {};
```

Value-returning builtins:
```cpp
return ExecutionResult {
    .success = true,
    .exitCode = 0,
    .error = {},
    .returnValue = 123
};
```

The interpreter pushes `returnValue` onto the operand stack whe it exists.

## Runtime validation
Even if semantic validates builtin argument types, runtime handlers must still validate
arguments defensively.

Reason:
- invalid IR could still be produced by a compiler bug
- future bytecode/IR loading may bypass source-level semantic analysis
- VM should not blindly trust input

Always check:
```cpp
args.size()
std::holds_alternative<T>(args[index])
```

## Module metadata
`Runtime::buildModulesFromBuiltins()` mirrors builtin metadata into `ModuleRegistry`.

Important rule:
```cpp
_modules.findMUtable(moduleName)
```
must use `moduleName`, not full builtin name.

Correct:
```cpp
auto *module = _modules.findMutable(moduleName);
```

Wrong:
```cpp
auto *module = _modules.findMutable(name);
```

## Semantic behavior
Semantic analysis uses module metadata to validate:
- module exists
- function exists
- arity matches
- argument types match
- return type is known

Example:
```velo
use std::string as str;

fn main(): int {
    return str::len(123); // ❌ semantic error
}
```

## lowering behavior
Source:
```velo
use std::string as str;

str::len("hello");
```

Lowered builtin call:
```text
CallBuiltin string::len args=1
```

The lower resolves imported aliases before emitting builtin calls.

## Required tests
When adding a builtin, add or update tests in these layers.

### Runtime module sync test
Verify builtin metadata is mirrored into `ModuleRegistry`.

Example:
```cpp
const auto *module = modules.find("string");
ASSERT_NE(module, nullptr);

const auto *func = module->findFunction("len");
ASSERT_NE(func, nullptr);

EXPECT_EQ(func->name, "len");
EXPECT_EQ(func->arity, 1U);
EXPECT_EQ(func->returnType, "int");
ASSERT_EQ(func->parameterTypes.size(), 1U);
EXPECT_EQ(func->parameterTypes[0], "string");
```

### Interpreter test
Verify runtime execution.

For string-returning builtins, use `string::len(...)` to convert the result `int`
if you need to assert `exitCode`.

Example:
```velo
int::toString(123) -> "123"
string::len("123") -> 3
```

### Semantic positive test
Verify valida call passes semantic analysis.

```velo
use std::string as str;

fn main(): int {
    return str::len("hello");
}
```

### Semantic negative test
Verify invalid argument type fails.

```velo
use std::string as str;

fn main(): int {
    return str::len(123);
}
```

Expected diagnostics:
```text
SEM033
```

### Driver test
Verify full pipeline execution.

```velo
use std::string as str;

fn main(): int {
    return str::len("hello");
}
```

Expected:
```text
exitCode == 5
```

### IR alias test

For aliases, verify lowerer emits runtime module name.

Source:

```velo
use std::string as str;

str::len("hello");
```

Expected IR:

```text
CallBuiltin string::len args=1
```

Not:

```text
CallBuiltin str::len args=1
```

## Required docs

Update:

```text
docs/language/modules.md
docs/architecture/runtime.md
docs/architecture/semantic.md
docs/examples.md
```

If the builtin is visible in README-level examples, update:

```text
README.md
```

## Required example

Add an example under:

```text
examples/
```

Example:

```text
examples/std_example/main.velo
```

## Common mistakes

## Forgetting to call registration method

Symptom:

```text
Unknown module
```

or:

```text
Unknown function
```

Fix:

```cpp
Runtime::Runtime() {
    registerStdExample();
    buildModulesFromBuiltins();
}
```

## Wrong runtime builtin name

Source alias:

```velo
use std::int as ints;
ints::toString(123);
```

Runtime builtin name must still be:

```text
int::toString
```

Not:

```text
ints::toString
std::int::toString
```

## Returning string from main

This is invalid:

```velo
fn main(): int {
    return ints::toString(123);
}
```

Because:

```text
ints::toString(123) -> string
main(): int
```

Use:

```velo
fn main(): int {
    return str::len(ints::toString(123));
}
```

## Forgetting runtime validation

Even if semantic validates arguments, builtin handler must still validate runtime values.

## Commit message format

Suggested format:

```text
feat(std): add <module> <function> builtin
```

Example:

```text
feat(std): add string len builtin
```