#!/usr/bin/env bash

set -euo pipefail

VELO_BIN="${VELO_BIN:-./build/debug/apps/velo/velo}"
ITERATIONS="${ITERATIONS:-100}"

if [[ ! -x "${VELO_BIN}" ]]; then
  echo "Velo binary not found or not executable: ${VELO_BIN}" >&2
  echo "Set VELO_BIN=/path/to/velo" >&2
  exit 1
fi

run_case() {
  local name="$1"
  local source_file="$2"

  if [[ ! -f "${source_file}" ]]; then
    echo "Benchmark source not found: ${source_file}" >&2
    exit 1
  fi

  local start_ns
  local end_ns
  local elapsed_ns
  local avg_ns

  start_ns="$(date +%s%N)"

  for _ in $(seq 1 "${ITERATIONS}"); do
    "${VELO_BIN}" run "${source_file}" >/dev/null
  done

  end_ns="$(date +%s%N)"
  elapsed_ns=$((end_ns - start_ns))
  avg_ns=$((elapsed_ns / ITERATIONS))

  printf "%-36s iterations=%s total_ms=%s avg_us=%s\n" \
    "${name}" \
    "${ITERATIONS}" \
    "$((elapsed_ns / 1000000))" \
    "$((avg_ns / 1000))"
}

echo "Velo runtime smoke benchmarks"
echo "VELO_BIN=${VELO_BIN}"
echo "ITERATIONS=${ITERATIONS}"
echo

run_case "json_stringify_primitives" "examples/json_stringify_primitives/main.velo"
run_case "json_parse" "examples/json_parse/main.velo"
run_case "json_access" "examples/json_access/main.velo"
run_case "http_response_access" "examples/http_response_access/main.velo"
run_case "http_request" "examples/http_request/main.velo"
run_case "http_json_flow" "examples/http_json_flow/main.velo"