# MVP 0.7 release checklist
This checklist should be completed before tagging or announcing Velo MVP 0.7.

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

## 5. MVP 0.7 JSON example smoke tests
Run:
```bash
./build/debug/apps/velo/velo run ./examples/json_stringify_primitives/main.velo
./build/debug/apps/velo/velo run ./examples/json_stringify_collections/main.velo
./build/debug/apps/velo/velo run ./examples/json_stringify_struct/main.velo
./build/debug/apps/velo/velo run ./examples/json_parse/main.velo
./build/debug/apps/velo/velo run ./examples/json_access/main.velo
```
Expected `json_stringify_primitives` output:
```text
42
"Alex"
true
false
```

Expected `json_stringify_collections` output:
```text
[1,2,3]
["Alex","Bob"]
[true,false]
{"Alex":10,"Bob":20}
{"a":[1,2],"b":[3,4]}
```

Expected `json_stringify_struct` output:
```text
{"active":true,"id":1,"name":"Alex"}
[{"active":true,"id":1,"name":"Alex"},{"active":false,"id":2,"name":"Bob"}]
{"alex":{"active":true,"id":1,"name":"Alex"},"bob":{"active":false,"id":2,"name":"Bob"}}
```

Expected `json_parse` output:
```text
{"id":42,"name":"Alex"}
[1,2,3]
"hello"
```

Expected `json_access` output:
```text
42
Alex
true
17
has name
```

All MVP 0.7 demo examples should exit with code `0`.

## 6. MVP 0.7 JSON check smoke tests
Run:
```bash
./build/debug/apps/velo/velo check ./examples/json_stringify_primitives/main.velo
./build/debug/apps/velo/velo check ./examples/json_stringify_collections/main.velo
./build/debug/apps/velo/velo check ./examples/json_stringify_struct/main.velo
./build/debug/apps/velo/velo check ./examples/json_parse/main.velo
./build/debug/apps/velo/velo check ./examples/json_access/main.velo
```
Expected:
```text
all commands exit with code 0
```

## 7. MVP 0.6 JSON IR smoke tests
Run:
```bash
./build/debug/apps/velo/velo ir ./examples/json_stringify_primitives/main.velo
./build/debug/apps/velo/velo ir ./examples/json_stringify_collections/main.velo
./build/debug/apps/velo/velo ir ./examples/json_stringify_struct/main.velo
./build/debug/apps/velo/velo ir ./examples/json_parse/main.velo
./build/debug/apps/velo/velo ir ./examples/json_access/main.velo
```

Check for representative fragments:
```text
CallBuiltin json::stringify args=1
CallBuiltin json::parse args=1
CallBuiltin json::get_int args=2
CallBuiltin json::get_string args=2
CallBuiltin json::get_bool args=2
CallBuiltin json::get_json args=2
CallBuiltin json::has args=2
```

## 8. MVP 0.7 JSON bytecode smoke tests
Run:
```bash
./build/debug/apps/velo/velo bytecode ./examples/json_stringify_primitives/main.velo
./build/debug/apps/velo/velo bytecode ./examples/json_stringify_collections/main.velo
./build/debug/apps/velo/velo bytecode ./examples/json_stringify_struct/main.velo
./build/debug/apps/velo/velo bytecode ./examples/json_parse/main.velo
./build/debug/apps/velo/velo bytecode ./examples/json_access/main.velo
```

Check for representative fragments:
```text
CallBuiltin json::stringify args=1
CallBuiltin json::parse args=1
CallBuiltin json::get_int args=2
CallBuiltin json::get_string args=2
CallBuiltin json::get_bool args=2
CallBuiltin json::get_json args=2
CallBuiltin json::has args=2
```

## 9. Focused JSON tests
Run:
```bash
ctest --test-dir build/debug -R "RuntimeModuleSyncTest.*Json" --output-on-failure
ctest --test-dir build/debug -R "SemanticAnalyzerTest.*Json" --output-on-failure
ctest --test-dir build/debug -R "DriverTest.*Json" --output-on-failure
```

Expected:
```text
all selected tests pass
```

## 10. Regression tests from previous milestones
Run representative previous examples:
```bash
./build/debug/apps/velo/velo run ./examples/array_literal/main.velo
./build/debug/apps/velo/velo run ./examples/array_indexing/main.velo
./build/debug/apps/velo/velo run ./examples/array_assignment/main.velo
./build/debug/apps/velo/velo run ./examples/array_builtin/main.velo

./build/debug/apps/velo/velo run ./examples/map_literal/main.velo
./build/debug/apps/velo/velo run ./examples/map_indexing/main.velo
./build/debug/apps/velo/velo run ./examples/map_assignment/main.velo
./build/debug/apps/velo/velo run ./examples/map_builtin/main.velo
```

Expected:
```text
all commands exit with code 0
```

## 11. Bytecode regression tests
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

## 12. Documentation review
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
docs/development/mvp_0_7.md

docs/development/diagnostics.md
docs/development/example_guidelines.md
docs/development/release_checklist.md

docs/development/release_notes_v0_1.md
docs/development/release_notes_v0_2.md
docs/development/release_notes_v0_3.md
docs/development/release_notes_v0_4.md
docs/development/release_notes_v0_5.md
docs/development/release_notes_v0_6.md
docs/development/release_notes_v0_7.md
```

Check:
- no outdated limitations remain
- JSON examples are documented
- `std::json` is documented
- `json::stringify` is documented
- `json::parse` is documented
- JSON access helpers are documented
- release notes match completed features
- release checklist is reproducible

## 13. MVP scope review
MVP 0.7 includes:
- `std::json`
- `json::stringify`
- JSON serialization for primitives
- JSON serialization for arrays
- JSON serialization for maps
- JSON serialization for structs
- runtime `json` type
- `json::parse`
- JSON object access helpers

Not included:
- JSON float numbers
- unicode escape parsing such as `\uXXXX`
- direct JSON indexing syntax
- JSON array access helpers
- JSON object length helpers
- JSON null helpers
- JSON mutation helpers
- pretty JSON formatting
- HTTP runtime
- file IO
- nullable user-level types
- generics
- methods
- iterators
- `for` loops
- standalone `veloc`
- standalone `velovm`

## 14. Git status
Check repository status:
```bash
git status
```

Expected:
```text
only intentional changes
```

## 15. Suggested release commit flow
Before release:
```bash
git add README.md docs
git commit -m "docs(mvp): finalize MVP 0.7 documentation"
```

## 16. Suggested tag
When ready:
```bash
git tag -a v0.7.0 -m "Velo MVP 0.7.0"
```

Push:
```bash
git push origin main
git push origin v0.7.0
```

Use only when the project is actually ready to publish the tag.

## 17. Release notes draft
Release notes are tracked here:
- [Velo v0.7.0 release notes draft](release_notes_v0_7.md)