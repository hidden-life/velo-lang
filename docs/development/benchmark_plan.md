# Velo benchmark plan
This document tracks the benchmark strategy for Velo.

Benchmarks should be added gradually and should not become unstable CI gates too early.

## Current stage
Current milestone:
```text
MVP 0.8     HTTP runtime foundation
```

Current benchmark scaffold:
```text
benchmarks/runtime/run_smoke.sh
```

The current script measures end-to-end CLI execution time for representative examples.

It includes process startup overhead and should be treated as a smoke benchmark only.

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