# MVP 0.3 release checklist
This checklist should be completed before tagging or announcing Velo MVP 0.3.

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
- `run` executes the program and exits with code `0` for demo examples

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

## 8. Error example smoke tests
Run known invalid examples:
```bash
./build/debug/apps/velo/velo check ./examples/errors/missing_return.velo
```
Expected:
- command exits with non-zero status
- diagnostic is readable
- diagnostic code is stable

## 9. IR smoke tests
Inspect representative IR:
```bash
./build/debug/apps/velo/velo ir ./examples/field_access/main.velo
./build/debug/apps/velo/velo ir ./examples/field_assignment/main.velo
./build/debug/apps/velo/velo ir ./examples/equality/main.velo
./build/debug/apps/velo/velo ir ./examples/block_scopes/main.velo
```

Check for:
- `BuildStruct`
- `LoadField`
- `StoreFieldPath`
- `CompareEqual`
- `CompareNotEqual`
- different local slots for shadowed locals

Representative expected fragments:
```text
LoadField id
StoreFieldPath id
CompareEqual
CompareNotEqual
StoreLocal local[0]
StoreLocal local[1]
```

## 10. Documentation review
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
docs/development/adding_builtin.md
docs/development/adding_lang_features.md
docs/development/diagnostics.md
docs/development/example_guidelines.md
docs/development/release_checklist.md
docs/development/release_notes_v0_1.md
docs/development/release_notes_v0_2.md
docs/development/release_notes_v0_3.md
```

Check:
- no outdated limitations remain
- examples compile
- CLI commands match actual behavior
- standard module signatures are correct
- diagnostic codes match tests
- architecture docs match current code
- MVP 0.3 feature list matches tests and examples

## 11. MVP scope review
MVP 0.3 includes:
- MVP 0.1 language foundation
- MVP 0.2 data model foundation
- struct value semantics
- field assignment
- nested field assignment
- equality operators for `string`
- equality operators for `bool`
- generic runtime equality for `int`, `string`, and `bool`
- block-scoped locals
- local shadowing in nested blocks

Not included:
- standalone block statements
- struct equality
- string ordering comparisons
- methods
- visibility enforcement for `pub` / private fields
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

## 12. Git status
Check repository status:
```bash
git status
```
Expected:
```text
only intentional changes
```

## 13. Suggested release commit flow
Before release:
```bash
git add README.md docs examples
git commit -m "docs(mvp): finalize MVP 0.3 documentation"
```

## 14. Suggested tag
When ready:
```bash
git tag -a v0.3.0 -m "Velo MVP 0.3.0"
```

Push:
```bash
git push origin main
git push origin v0.3.0
```
Use only when the project is actually ready to publish the tag.

## 15. Release notes draft
Release notes are tracked here:
- [Velo v0.3.0 release notes draft](release_notes_v0_3.md)