# Example guidelines
This document describes how examples under `examples/` should be written.

Examples are part of the developer experience. They should be simple, stable, and easy to run.

## Goals
Examples should:
- demonstrate one or two features clearly
- be easy to run from CLI
- return process exit code `0` unless intentionally demonstrating exit codes
- avoid mixing too many unrelated features
- stay aligned with documentation

## Recommended example shape
```velo
module app;

use std::console;

fn main(): int {
    console::println("hello");
    
    return 0;
}
```

## Return code rule
Demo examples should usually return:
```velo
return 0;
```

Reason:

The CLI returns `main()` result as the process exit code.

This means:
```velo
return 1;
```
makes the shell treat the command as failure.

If the feature needs to prove a non-zero value, use a driver test:
```cpp
EXPECT_EQ(result.exitCode, 42);
```
rather than making the demo example return `42`.

## Naming
use clear directories:
```text
examples/arithmetic/
examples/logical/
examples/short_circuit/
examples/std_string_len/
examples/std_to_string/
examples/errors/
```

Each example should usually have:
```text
main.velo
```

## One feature per example
Good:
```text
examples/arithmetic/main.velo
```
demonstrates:
- arithmetic
- grouping
- printing result

Avoid examples that demonstrates too much at once.

Bad:
```text
arithmetic + std module + loops + function calls + errors
```
unless the example is intentionally and integration showcase.

## Error examples
Invalid programs should go under:
```text
examples/errors/
```

Error examples should demonstrates one diagnostic clearly.

Example:
```velo
module app;

fn broken(val: mystery): int {
    return 0;
}

fn main(): int {
    return 0;
}
```

Expected:
```text
SEM030 Unknown declared type
```

## CLI commands
Every valid example should work with:
```bash
./build/debug/apps/velo/velo check ./examples/<name>/main.velo
./build/debug/apps/velo/velo ast ./examples/<name>/main.velo
./build/debug/apps/velo/velo ir ./examples/<name>/main.velo
./build/debug/apps/velo/velo run ./examples/<name>/main.velo
```

For error examples, use:
```bash
./build/debug/apps/velo/velo check ./examples/errors/<name>.velo
```

## Standard module examples
When demonstrating standard modules, prefer aliases when the module name may conflict
visually with language types.

Example:
```velo
use std::string as str;
use std::int as ints;
use std::bool as bools;
```

Then:
```velo
str::len("hello");
ints::toString(123);
bools::toString(true);
```

## Output
Examples that print output should have predictable output.

Good:
```text
7
```

Good:
```text
short-circuit works
```

Avoid output that depends on environment, time, paths, or random values.

## Adding a new example
When adding a new example:
1. create `examples/<feature>/main.velo`
2. make sure it returns `0`
3. run `velo check`
4. run `velo run`
5. optionally run `velo ast`
6. optionally run `velo ir`
7. add it to `docs/examples.md`
8. mention it in README if it is important

## Example checklist
Before commiting an example:
```text
[ ] example has clear purpose
[ ] example has module declaration
[ ] example has main(): int
[ ] example returns 0
[ ] example runs with velo run
[ ] example passes velo check
[ ] example os documented in docs/examples.md
```

## Current recommended examples
Keep these examples healthy:
```text
examples/arithmetic/main.velo
examples/logical/main.velo
examples/short_circuit/main.velo
examples/while_loop/main.velo
examples/break_continue/main.velo
examples/typed_parameters/main.velo
examples/std_string/main.velo
examples/std_to_string/main.velo
```

## Integration showcase example
At some point before or after MVP 0.1, it may be useful to add:
```text
examples/mvp_showcase/main.velo
```

Purpose:
- demonstrate functions
- demonstrate loops
- demonstrate conditions
- demonstrate std modules
- demonstrate conversion builtins

This should be treated as a showcase, not as the primary place to test every feature.