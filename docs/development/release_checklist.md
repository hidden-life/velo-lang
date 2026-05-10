# MVP 0.1 release checklist
This checklist should be completed before tagging or announcing Velo MVP 0.1.

The goal is to make sure the MVP is small, stable, documented, and reproducible.

## 1. Build
Run a clean debug build:
```bash
cmake --preset debug
cmake --build --preset debug
```

Expected:
```text
build succeeds
```

## 2. Tests
Run the full test suite:
```bash
ctest --preset debug --output-on-failure
```

Expected:
```text
100% tests passed
```

## 3. CLI smoke tests
Check version/help:
```bash
./build/debug/apps/velo/velo --version
./build/debug/apps/velo/velo --help
```

Check all primary CLI modes:
```bash
./build/debug/apps/velo/velo check ./examples/arithmetic/main.velo
./build/debug/apps/velo/velo ast ./examples/arithmetic/main.velo
./build/debug/apps/velo/velo ir ./examples/arithmetic/main.velo
./build/debug/apps/velo/velo run ./examples/arithmetic/main.velo
```

Expected:
- `check` exits with code `0`
- `ast` prints AST and exits with code `0`
- `ir` prints IR and exits with code `0`
- `run` executes the program and exits with code `0` for demo examples

## 4. Backward-compatible CLI shorthand
Run:
```bash
./build/debug/apps/velo/velo ./examples/arithmetic/main.velo
```

Expected:
```text
same behavior as vero run ./examples/arithmetic/main.velo
```

## 5. Example smoke tests
Run the most important examples:
```bash
./build/debug/apps/velo/velo run ./examples/arithmetic/main.velo
./build/debug/apps/velo/velo run ./examples/logical/main.velo
./build/debug/apps/velo/velo run ./examples/short_circuit/main.velo
./build/debug/apps/velo/velo run ./examples/while_loop/main.velo
./build/debug/apps/velo/velo run ./examples/break_continue/main.velo
./build/debug/apps/velo/velo run ./examples/typed_parameters/main.velo
./build/debug/apps/velo/velo run ./examples/std_string/main.velo
./build/debug/apps/velo/velo run ./examples/std_to_string/main.velo
./build/debug/apps/velo/velo run ./examples/mvp_showcase/main.velo
```

Expected:
- each example prints its intended output
- each demo example returns process exit code `0`

**Important**:

Demo examples should usually end with:
```velo
return 0;
```

If the language feature needs a non-zero return value, test it through driver tests
instead of demo examples.

## 6. Error example smoke tests
Run known invalid examples:
```bash
./build/debug/apps/velo/velo check ./examples/errors/unknown_type.velo
```

Expected:
- command exits with non-zero status
- diagnostic is readable
- diagnostic code is stable

## 7. IR smoke tests
Inspect IR for a few representative examples:
```bash
./build/debug/apps/velo/velo ir ./examples/arithmetic/main.velo
./build/debug/apps/velo/velo ir ./examples/short_circuit/main.velo
```

Check for:
- arithmetic instructions
- jump instructions
- builtin calls
- alias resolution

For alias resolution, source like:
```velo
use std::string as str;

str::len("hello");
```

should produce IR like:
```text
CallBuiltin string::len args=1
```
not:
```text
CallBuiltin str::len args=1
```

## 8. Documentation review
Review:
```text
README.md
docs/index.md
docs/language/syntax.md
docs/language/types.md
docs/language/modules.md
docs/architecture/overview.md
docs/architecture/semantic.md
docs/architecture/runtime.md
docs/architecture/ir.md
docs/examples.md
docs/development/mvp_0_1.md
docs/development/adding_builtin.md
docs/development/adding_language_feature.md
docs/development/diagnostics.md
docs/development/example_guidelines.md
docs/development/release_checklist.md
```

Check:
- no outdated limitations remain
- examples compile
- CLI commands match actual behavior
- standard module signatures are correct
- diagnostic codes match tests
- architecture docs match current code

## 9. MVP scope review
MVP 0.1 should stay focused.

Included:
- basic language syntax
- functions
- locals
- scopes
- conditions
- loops
- arithmetic
- logical operators
- short-circuit
- builtins
- standard module foundation
- semantic validation
- IR lowering
- VM execution
- CLI modes
- documentation

Not included:
- structs
- arrays
- maps
- nullable types
- generics
- classes
- interfaces
- async/await
- package manager
- bytecode serialization
- standalone `veloc`
- standalone `velovm`
- HTTP runtime
- JSON parser/serializer
- optimizer
- full control-flow graph analysis

## 10. Git status
Check repository status:
```bash
git status
```

Expected:
```text
only intentional changes
```

## 11. Suggested release commit flow
Before release:
```bash
git add README.md docs examples tests include src apps
git commit -m "chore(docs): prepare MVP 0.1 documentation"
```

Then final release/tag commit can be separate if desired.

## 12. Suggested tag
When ready:
```bash
get tag v0.1.0
```

Push:
```bash
git push origin main
git push origin v0.1.0
```

Use only when the project is actually ready to publish the tag.

## 13. Release notes draft
Release notes are tracked here:
- [Velo v0.1.0 release notes draft](release_notes_v0_1.md)