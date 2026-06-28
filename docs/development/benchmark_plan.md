# Velo benchmark plan
This document tracks the benchmark strategy for Velo.

Benchmarks should be added gradually and should not become unstable CI gates too early.

The current script measures end-to-end CLI execution time for representative examples.

It includes process startup overhead and should be treated as a smoke benchmark only.

## Dependencies
The MVP 0.8 benchmark scaffold does not require third-party benchmark tools.

Required standard shell tools:
```text
bash
date
seq
chmod
```

No package installation is required for MVP 0.8.

Future milestones may add optional external tools such as:
```text
wrk
hey
ab
```

When these tools are introduced, installation commands and usage instructions must be documented
in this file and in the release checklist.

## Current stage
Current milestone:
```text
MVP 0.8     HTTP runtime foundation
```

Current benchmark scaffold:
```text
benchmarks/runtime/run_smoke.sh
```

## MVP 0.8 benchmark goals
The goal of MVP 0.8 benchmarks is to verify that the HTTP/JSON runtime foundation
can be exercised repeatedly.

Current cases:
- JSON stringify primitives
- JSON parse
- JSON access helpers
- HTTP response access helpers
- HTTP request helpers
- HTTP JSON flow

No performance thresholds are enforced in MVP 0.8.

## MVP 0.9 benchmark goals
When a real HTTP server appears, add local HTTP benchmark scripts.

Candidate routes:
```text
GET     /health
POST    /echo
POST    /json
```

Candidate tools:
```text
curl
hey
wrk
ab
```

Metrics:
```text
requests/sec
average latency
p50 latency
p95 latency
p99 latency
error rate
```

## MVP 0.10 benchmark goals
After the server/router foundation stabilizes:
- add documented baseline runs
- add optional C++ micro-benchmark executable
- add HTTP throughput benchmarks
- add JSON parse/stringify focused benchmarks
- add conservative performance regression checks

Performance thresholds should remain optional until the benchmark environment
is stable.

## Rules
- Benchmarks should be reproducible.
- Benchmarks should not fail normal tests by default.
- Benchmarks should not require external tools unless explicitly documented.
- Benchmarks should separate correctness tests from performance measurements.
- CI thresholds should be introduced only when results are stable across machines.

## MVP 0.9 benchmark direction
MVP 0.9 introduces a real HTTP server foundation.

Benchmark strategy:
1. keep parser/serializer tests as correctness tests
2. add manual server smoke benchmarks after `velo serve`
3. avoid performance thresholds until server behavior is stable

Future optional tools:
```text
curl
ab
wrk
hey
```

When external tools are introduced, installation commands must be documented
in:
```text
docs/development/benchmark_plan.md
docs/development/release_checklist.md
```

MVP 0.9.1 does not add third-party benchmark dependencies.