# MVP 0.8 release checklist
This checklist should be completed before tagging or announcing Velo MVP 0.8.

The goal is to make sure the MVP is small, stable, documented, and reproducible.

## 1. Dependencies
MVP 0.8 does not add third-party runtime libraries.

The runtime benchmark smoke script uses standard shell tools:
```text
bash
date
seq
chmod
```

No package installation is required for MVP 0.8.

## 2. Build
Run a debug build:
```bash
cmake --build build/debug
```

Alternative preset flow:
```bash
cmake --preset debug
cmake --build --preset debug
```

Expected:
```text
build succeeds
```

## 3. Full tests
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

## 4. Focused HTTP tests
Runtime module sync:
```bash
ctest --test-dir build/debug -R "RuntimeModuleSyncTest.*Http" --output-on-failure
```

Semantic:
```bash
ctest --test-dir build/debug -R "SemanticAnalyzerTest.*Http" --output-on-failure
```

Driver:
```bash
ctest --test-dir build/debug -R "DriverTest.*Http" --output-on-failure
```

Expected:
```text
all selected tests pass
```

HTTP parser/handler/pipeline/server:
```bash
ctest --test-dir build/debug -R "HttpMessageTest|HttpHandlerTest|HttpPipelineTest|HttpServerTest" --output-on-failure
```

## 5. HTTP example check smoke tests
Run:
```bash
./build/debug/apps/velo/velo check ./examples/http_response/main.velo
./build/debug/apps/velo/velo check ./examples/http_response_access/main.velo
./build/debug/apps/velo/velo check ./examples/http_request/main.velo
./build/debug/apps/velo/velo check ./examples/http_json_flow/main.velo
```

Expected:
```text
all commands exit with code 0
```

## 6. HTTP example run smoke tests
Run:
```bash
./build/debug/apps/velo/velo run ./examples/http_response/main.velo
./build/debug/apps/velo/velo run ./examples/http_response_access/main.velo
./build/debug/apps/velo/velo run ./examples/http_request/main.velo
./build/debug/apps/velo/velo run ./examples/http_json_flow/main.velo
```

Expected `http_response_access` output:
```text
200
Hello
true
text/plain
201
{"ok":true}
application/json
```

Expected `http_request` output:
```text
POST
/users
Alex
```

Expected `http_json_flow` output:
```text
POST
/users
201
{"ok":true}
application/json
```

All commands should exit with code `0`.

## 7. IR smoke tests
Run:
```bash
./build/debug/apps/velo/velo ir ./examples/http_response/main.velo
./build/debug/apps/velo/velo ir ./examples/http_response_access/main.velo
./build/debug/apps/velo/velo ir ./examples/http_request/main.velo
./build/debug/apps/velo/velo ir ./examples/http_json_flow/main.velo
```

Check representative fragments:
```text
CallBuiltin http::response args=2
CallBuiltin http::text_response args=2
CallBuiltin http::json_response args=2
CallBuiltin http::status args=1
CallBuiltin http::body args=1
CallBuiltin http::has_header args=2
CallBuiltin http::header args=2
CallBuiltin http::request args=3
CallBuiltin http::method args=1
CallBuiltin http::path args=1
CallBuiltin http::json_body args=1
```

## 8. Bytecode smoke tests
Run:
```bash
./build/debug/apps/velo/velo bytecode ./examples/http_response/main.velo
./build/debug/apps/velo/velo bytecode ./examples/http_response_access/main.velo
./build/debug/apps/velo/velo bytecode ./examples/http_request/main.velo
./build/debug/apps/velo/velo bytecode ./examples/http_json_flow/main.velo
```

Check representative fragments:
```text
CallBuiltin http::response args=2
CallBuiltin http::text_response args=2
CallBuiltin http::json_response args=2
CallBuiltin http::status args=1
CallBuiltin http::body args=1
CallBuiltin http::has_header args=2
CallBuiltin http::header args=2
CallBuiltin http::request args=3
CallBuiltin http::method args=1
CallBuiltin http::path args=1
CallBuiltin http::json_body args=1
```

## 9. Runtime benchmark smoke
Make sure the benchmark script is executable:
```bash
chmod +x benchmark/runtime/run_smoke.sh
```

Run a short smoke benchmark:
```bash
VELO_BIN=./build/debug/apps/velo/velo ITERATIONS=10 ./benchmarks/runtime/run_smoke.sh
```

Expected output format:
```text
Velo runtime smoke benchmarks
VELO_BIN=./build/debug/apps/velo/velo
ITERATIONS=10

json_stringify_primitives          iterations=10 total_ms=... avg_us=...
json_parse                         iterations=10 total_ms=... avg_us=...
json_access                        iterations=10 total_ms=... avg_us=...
http_response_access               iterations=10 total_ms=... avg_us=...
http_request                       iterations=10 total_ms=... avg_us=...
http_json_flow                     iterations=10 total_ms=... avg_us=...
```

Benchmark notes:
- results are informational
- no performance thresholds are enforced
- script includes CLI process startup overhead
- this benchmark is not a CI gate

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
docs/architecture/bytecode.md

docs/development/mvp_0_1.md
docs/development/mvp_0_2.md
docs/development/mvp_0_3.md
docs/development/mvp_0_4.md
docs/development/mvp_0_5.md
docs/development/mvp_0_6.md
docs/development/mvp_0_7.md
docs/development/mvp_0_8.md

docs/development/diagnostics.md
docs/development/example_guidelines.md
docs/development/release_checklist.md
docs/development/benchmark_plan.md

docs/development/release_notes_v0_1.md
docs/development/release_notes_v0_2.md
docs/development/release_notes_v0_3.md
docs/development/release_notes_v0_4.md
docs/development/release_notes_v0_5.md
docs/development/release_notes_v0_6.md
docs/development/release_notes_v0_7.md
docs/development/release_notes_v0_8.md
```

Check:
- HTTP runtime types are documented
- `std::http` is documented
- response builders are documented
- response access helpers are documented
- request helpers are documented
- HTTP/JSON examples are documented
- benchmark scaffold is documented
- dependency notes are documented
- release notes match completed features
- release checklist is reproducible

## 12. MVP scope review
MVP 0.8 includes:
- `http_request`
- `http_response`
- `std::http`
- response builders
- response access helpers
- request builders/access helpers
- HTTP/JSON handler-like flow
- benchmark smoke scaffold

Not included:
- real HTTP server
- routing
- sockets
- async runtime
- TLS
- middleware
- cookies
- multipart parsing
- streaming bodies
- real server benchmarks

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
git commit -m "docs(mvp): finalize MVP 0.8 documentation"
```

## 15. Suggested tag
When ready:
```bash
git tag -a v0.8.0 -m "Velo MVP 0.8.0"
```

Push:
```bash
git push origin main
git push origin v0.7.0
```

Use only when the project is actually ready to publish the tag.

## 17. Release notes draft
Release notes are tracked here:
- [Velo v0.8.0 release notes draft](release_notes_v0_8.md)