#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VELO_BIN="${VELO_BIN:-$ROOT_DIR/build/debug/apps/velo/velo}"
EXAMPLE="${EXAMPLE:-$ROOT_DIR/examples/http_routing_helpers/main.velo}"
REQUESTS="${1:-50}"
SERVER_LOG="${SERVER_LOG:-/tmp/velo-http-curl-loop.log}"
URL="${URL:-http://127.0.0.1:8080/health}"

if [[ ! "$REQUESTS" =~ ^[0-9]+$ ]]; then
    echo "request count must be a positive integer" >&2
    exit 1
fi

if [[ "$REQUESTS" -eq 0 ]]; then
    echo "request count must be greater than zero" >&2
    exit 1
fi

if [[ ! -x "$VELO_BIN" ]]; then
    echo "velo binary not found or not executable: $VELO_BIN" >&2
    echo "Run: cmake --build build/debug" >&2
    exit 1
fi

"$VELO_BIN" serve "$EXAMPLE" >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!

cleanup() {
    kill "$SERVER_PID" >/dev/null 2>&1 || true
    wait "$SERVER_PID" >/dev/null 2>&1 || true
}

trap cleanup EXIT

sleep 1

start_ns="$(date +%s%N)"

for _ in $(seq 1 "$REQUESTS"); do
    curl -sS -o /dev/null "$URL"
done

end_ns="$(date +%s%N)"

elapsed_ns=$((end_ns - start_ns))
elapsed_ms=$((elapsed_ns / 1000000))

if [[ "$elapsed_ms" -eq 0 ]]; then
    elapsed_ms=1
fi

requests_per_second=$((REQUESTS * 1000 / elapsed_ms))

echo "requests=$REQUESTS"
echo "elapsed_ms=$elapsed_ms"
echo "approx_requests_per_second=$requests_per_second"