# Velo runtime benchmark scaffold
This directory contains early benchmark scaffolding for Velo runtime flows.

The current benchmark script is intentionally simple:
- no external dependencies
- no pass/fail performance thresholds
- no CI requirement
- manual local runs only

It measures end-to-end CLI execution time for representative examples.

This includes process startup overhead, so results are not pure runtime micro-benchmarks yet.

## Run
From repository root:
```bash
VELO_BIN=./build/debug/apps/velo/velo ITERATIONS=100 ./benchmark/runtime/run_smoke.sh
```

Defaults:
```text
VELO_BIN=./build/debug/apps/velo/velo
ITERATIONS=100
```

## Current cases
- JSON stringify primitives
- JSON parse
- JSON access
- HTTP response access
- HTTP request helpers
- HTTP JSON flow

## Notes
Do not use this script as a strict performance gate.

Future milestones should add:
- C++ runtime micro-benchmark executable
- HTTP server benchmarks
- latency percentiles
- throughput benchmarks
- optional external tool scripts for `wrk` or `hey`