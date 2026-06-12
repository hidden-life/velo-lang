# MVP 0.6 release checklist
This checklist should be completed before tagging or announcing Velo MVP 0.6.

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
./build/debug/apps/velo/velo bytecode ./examples/arithmetic/main.velo
./build/debug/apps/velo/velo bc ./examples/arithmetic/main.velo
./build/debug/apps/velo/velo run ./examples/arithmetic/main.velo
```

Expected:
- `check` exits with code `0`
- `ast` prints AST and exits with code `0`
- `ir` prints IR and exits with code `0`
- `bytecode` prints bytecode disassembly and exist with code `0`.
- `bc` prints bytecode disassembly and exist with code `0`.
- `run` executes the program and exits with code `0`

## 4. Backward-compatible CLI shorthand
Run:
```bash
./build/debug/apps/velo/velo ./examples/arithmetic/main.velo
```
Expected:
```text
same behavior as velo run ./examples/arithmetic/main.velo
```

## 5. MVP 0.6 map example smoke tests
Run:
```bash
./build/debug/apps/velo/velo run ./examples/map_literal/main.velo
./build/debug/apps/velo/velo run ./examples/map_indexing/main.velo
./build/debug/apps/velo/velo run ./examples/map_assignment/main.velo
./build/debug/apps/velo/velo run ./examples/map_builtin/main.velo
```
Expected `map_literal` output:
```text
<map len=2>
<map len=2>
<map len=0>
```

Expected `map_indexing` output:
```text
20
Bob
4
```

Expected `map_assignment` output:
```text
42
30
99
77
```

Expected `map_builtin` output:
```text
2
2
0
3
```

All MVP 0.6 demo examples should exit with code `0`.

## 6. MVP 0.6 map check smoke tests
Run:
```bash
./build/debug/apps/velo/velo check ./examples/map_literal/main.velo
./build/debug/apps/velo/velo check ./examples/map_indexing/main.velo
./build/debug/apps/velo/velo check ./examples/map_assignment/main.velo
./build/debug/apps/velo/velo check ./examples/map_builtin/main.velo
```
Expected:
```text
all commands exit with code 0
```

## 7. MVP 0.6 map IR smoke tests
Run:
```bash
./build/debug/apps/velo/velo ir ./examples/map_literal/main.velo
./build/debug/apps/velo/velo ir ./examples/map_indexing/main.velo
./build/debug/apps/velo/velo ir ./examples/map_assignment/main.velo
./build/debug/apps/velo/velo ir ./examples/map_builtin/main.velo
```

Check for representative fragments:
```text
BuildMap entries=2
LoadIndex
StoreIndexPath indexes=1
StoreIndexPath indexes=2
CallBuiltin map::len args=1
```

## 8. MVP 0.6 bytecode smoke tests
Run:
```bash
./build/debug/apps/velo/velo bytecode ./examples/map_literal/main.velo
./build/debug/apps/velo/velo bytecode ./examples/map_indexing/main.velo
./build/debug/apps/velo/velo bytecode ./examples/map_assignment/main.velo
./build/debug/apps/velo/velo bytecode ./examples/map_builtin/main.velo
```

Check for representative fragments:
```text
BuildMap entries=2
LoadIndex
StoreIndexPath indexes=1
StoreIndexPath indexes=2
CallBuiltin map::len args=1
```

## 9. Focused map tests
Run:
```bash
ctest --test-dir build/debug -R "ParserTest.*Map" --output-on-failure
ctest --test-dir build/debug -R "SemanticAnalyzerTest.*Map" --output-on-failure
ctest --test-dir build/debug -R "DriverTest.*Map" --output-on-failure
ctest --test-dir build/debug -R "BytecodeVmTest.*Map" --output-on-failure
```

Expected:
```text
all selected tests pass
```

## 10. Bytecode regression tests
Run:
```bash
ctest --test-dir build/debug -R "BytecodeTest" --output-on-failure
ctest --test-dir build/debug -R "BytecodeCompilerTest" --output-on-failure
ctest --test-dir build/debug -R "BytecodeVmTest" --output-on-failure
ctest --test-dir build/debug -R "BytecodeDisassemblerTest" --output-on-failure
ctest --test-dir build/debug -R "BytecodeFileFormatTest" --output-on-failure
```
Expected:
```text
all selected tests pass
```

## 11. Previous MVP example smoke tests
Run representative examples from previous milestone:
```bash
./build/debug/apps/velo/velo run ./examples/array_literal/main.velo
./build/debug/apps/velo/velo run ./examples/array_indexing/main.velo
./build/debug/apps/velo/velo run ./examples/array_assignment/main.velo
./build/debug/apps/velo/velo run ./examples/array_builtin/main.velo
```
Expected:
```text
all commands exit with code 0
```

## 12. Error example smoke tests
Run known invalid example:
```bash
./build/debug/apps/velo/velo check ./examples/errors/missing_return.velo
```

Expected:
- command exits with non-zero status
- diagnostic is readable
- diagnostic code is stable

## 13. Documentation review
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
docs/architecture/bytecode.md

docs/development/mvp_0_1.md
docs/development/mvp_0_2.md
docs/development/mvp_0_3.md
docs/development/mvp_0_4.md
docs/development/mvp_0_5.md
docs/development/mvp_0_6.md

docs/development/diagnostics.md
docs/development/example_guidelines.md
docs/development/release_checklist.md

docs/development/release_notes_v0_1.md
docs/development/release_notes_v0_2.md
docs/development/release_notes_v0_3.md
docs/development/release_notes_v0_4.md
docs/development/release_notes_v0_5.md
docs/development/release_notes_v0_6.md
```

Check:
- no outdated limitations remain
- map examples are documented
- map diagnostics are documented
- CLI commands match actual behavior
- `std::map` and `map::len` are documented
- release notes match completed features
- release checklist is reproducible

## 14. MVP scope review
MVP 0.6 includes:
- map type syntax
- map semantic type model
- map literals
- runtime map values
- deep-copy support for maps
- map indexing read
- map element assignment
- map element insert assignment
- nested map assignment
- mixed map/array index assignment
- `std::map`
- `map::len`

Not included:
- non-string map keys
- field assignment through map index, for example `users["bob"].id = 3`
- `map::has`
- `map::keys`
- `map::values`
- `map::remove`
- JSON parser
- JSON serializer
- nullable types
- generics
- methods
- iterators
- `for` loops
- bytecode optimizer
- bytecode verifier
- standalone `veloc`
- standalone `velovm`

## 15. Git status
Check repository status:
```bash
git status
```

Expected:
```text
only intentional changes
```

## 16. Suggested release commit flow
Before release:
```bash
git add README.md docs
git commit -m "docs(mvp): finalize MVP 0.6 documentation"
```

## 17. Suggested tag
When ready:
```bash
git tag -a v0.6.0 -m "Velo MVP 0.6.0"
```

Push:
```bash
git push origin main
git push origin v0.6.0
```

Use only when the project is actually ready to publish the tag.

## 18. Release notes draft
Release notes are tracked here:
- [Velo v0.6.0 release notes draft](release_notes_v0_6.md)