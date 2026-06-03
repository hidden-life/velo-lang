# MVP 0.4 release checklist
This checklist should be completed before tagging or announcing Velo MVP 0.4.

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

Alternative command:
```bash
cmake --build build/debug
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

Alternative command:
```bash
ctest --test-dir build/debug --output-on-failure
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
- `run` executes the program and exits with code `0` for demo example

## 4. Backward-compatible CLI shorthand
Run:
```bash
./build/debug/apps/velo/velo ./examples/arithmetic/main.velo
```
Expected:
```text
same behavior as velo run ./examples/arithmetic/main.velo
```

## 5. Core example smoke tests
Run:
```bash
./build/debug/apps/velo/velo run ./examples/arithmetic/main.velo
./build/debug/apps/velo/velo run ./examples/logical/main.velo
./build/debug/apps/velo/velo run ./examples/short_circuit/main.velo
./build/debug/apps/velo/velo run ./examples/while_loop/main.velo
./build/debug/apps/velo/velo run ./examples/break_continue/main.velo
./build/debug/apps/velo/velo run ./examples/typed_parameters/main.velo
./build/debug/apps/velo/velo run ./examples/std_string/main.velo
./build/debug/apps/velo/velo run ./examples/std_to_string/main.velo
```
Expected:
- each example exits with code `0`
- demo examples should usually end with `return 0;`

## 6. MVP 0.2 struct example smoke tests
Run:
```bash
./build/debug/apps/velo/velo run ./examples/struct_declaration/main.velo
./build/debug/apps/velo/velo run ./examples/user_defined_type_registry/main.velo
./build/debug/apps/velo/velo run ./examples/struct_type_usage/main.velo
./build/debug/apps/velo/velo run ./examples/struct_literal/main.velo
./build/debug/apps/velo/velo run ./examples/field_access/main.velo
```
Expected:
```text
all commands exit with code 0
```

## 7. MVP 0.3 example smoke tests
Run:
```bash
./build/debug/apps/velo/velo run ./examples/field_assignment/main.velo
./build/debug/apps/velo/velo run ./examples/equality/main.velo
./build/debug/apps/velo/velo run ./examples/block_scopes/main.velo
```
Expected `field_assignment` output:
```text
1
Alex
2
Bob
```
Expected `equality` output:
```text
true
true
true
true
```
Expected `block_scopes` output:
```text
2
1
3
1
```
All MVP 0.3 demo examples should exit with code `0`.

## 8. MVP 0.4 example and smoke tests
Run:
```bash
./build/debug/apps/velo/velo run ./examples/array_literal/main.velo
./build/debug/apps/velo/velo run ./examples/array_indexing/main.velo
./build/debug/apps/velo/velo run ./examples/array_assignment/main.velo
./build/debug/apps/velo/velo run ./examples/array_builtin/main.velo
```

Expected `array_literal` output:
```text
<array len=3>
<array len=2>
<array len=2>
<array len=2>
```
Expected `array_indexing` output:
```text
20
Bob
3
```

Expected `array_assignment` output:
```text
1
42
99
2
Bob
```
Expected `array_builtin` output:
```text
3
2
2
```
All MVP 0.4 demo examples should exit with code `0`.

## 9. Error example smoke tests
Run known invalid example:
```bash
./build/debug/apps/velo/velo check ./examples/errors/missing_return.velo
```

Expected:
- command exits with non-zero status
- diagnostic is readable
- diagnostic code is stable

## 10. IR smoke tests
Inspect representative IR:
```bash
./build/debug/apps/velo/velo ir ./examples/array_literal/main.velo
./build/debug/apps/velo/velo ir ./examples/array_indexing/main.velo
./build/debug/apps/velo/velo ir ./examples/array_assignment/main.velo
./build/debug/apps/velo/velo ir ./examples/array_builtin/main.velo
```

Check for:
- `BuildArray`
- `LoadIndex`
- `StoreIndexPath`
- `CallBuiltin array::len args=1`

Representative expected fragments:
```text
BuildArray elements=3
LoadIndex
StoreIndexPath indexes=1
StoreIndexPath indexes=2
CallBuiltin array::len args=1
```

## 11. Documentation review
Review:
```text
README.md
docs/index.md
docs/examples.md

docs/language/syntax.md
docs/language/types.md
docs/language/modules.md

docs/architecture/overview.md
docs/architecture/semantic.md
docs/architecture/runtime.md
docs/architecture/ir.md

docs/development/mvp_0_1.md
docs/development/mvp_0_2.md
docs/development/mvp_0_3.md
docs/development/mvp_0_4.md
docs/development/adding_builtin.md
docs/development/adding_lang_features.md
docs/development/diagnostics.md
docs/development/example_guidelines.md
docs/development/release_checklist.md
docs/development/release_notes_v0_1.md
docs/development/release_notes_v0_2.md
docs/development/release_notes_v0_3.md
docs/development/release_notes_v0_4.md
```

Check:
- no outdated limitations remain
- examples compile
- CLI commands match actual behavior
- standard module signatures are correct
- diagnostic codes match tests
- architecture docs match current code
- MVP 0.4 feature list matches tests and examples

## 12. MVP scope review
MVP 0.4 includes:
- MVP 0.1 language foundation
- MVP 0.2 data model foundation
- MVP 0.3 value semantics and block scope foundation
- array type syntax
- nested array type syntax
- array literals
- runtime array values
- deep-copy support for arrays
- array indexing read
- nested array indexing
- array element assignment
- nested array element assignment
- `std::array`
- `array::len`

Not included:
- mixed field/index assignment paths such as `users[0].id = 2`
- `array::push`
- `array::pop`
- `array::is_empty`
- iterators
- `for` loops
- maps
- nullable types
- generics
- methods
- bytecode serialization
- standalone `veloc`
- standalone `velovm`
- HTTP runtime
- JSON parser/serializer
- optimizer

## 13. Git status
Check repository status:
```bash
git status
```

Expected:
```text
only intentional changes
```

## 14. Suggested release commit flow
Before release:
```bash
git add README.md docs
git commit -m "docs(mvp): finalize MVP 0.4 documentation"
```

## 15. Suggested tag
When ready:
```bash
git tag -a v0.4.0 -m "Velo MVP 0.4.0"
```

Push:
```bash
git push origin main
git push origin v0.4.0
```

Use only when the project is actually ready to publish the tag.

## 16. Release notes draft
Release notes are tracked here:
- [Velo v0.4.0 release notes draft](release_notes_v0_4.md)