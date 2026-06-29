#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VELO_BIN="${VELO_BIN:-$ROOT_DIR/build/debug/apps/velo/velo}"
EXAMPLE="${EXAMPLE:-$ROOT_DIR/examples/http_routing_helpers/main.velo}"
SERVER_LOG="${SERVER_LOG:-/tmp/velo-http-smoke.log}"

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

health_body="$(curl -sS http://127.0.0.1:8080/health)"
if [[ "$health_body" != "OK" ]]; then
    echo "unexpected /health body: $health_body" >&2
    echo "server log:" >&2
    cat "$SERVER_LOG" >&2
    exit 1
fi

hello_body="$(curl -sS http://127.0.0.1:8080/hello)"
if [[ "$hello_body" != "hello" ]]; then
    echo "unexpected /hello body: $hello_body" >&2
    echo "server log:" >&2
    cat "$SERVER_LOG" >&2
    exit 1
fi

missing_status="$(curl -sS -o /tmp/velo-http-missing-body.txt -w "%{http_code}" http://127.0.0.1:8080/missing)"
missing_body="$(cat /tmp/velo-http-missing-body.txt)"

if [[ "$missing_status" != "404" ]]; then
    echo "unexpected /missing status: $missing_status" >&2
    echo "server log:" >&2
    cat "$SERVER_LOG" >&2
    exit 1
fi

if [[ "$missing_body" != "not found" ]]; then
    echo "unexpected /missing body: $missing_body" >&2
    echo "server log:" >&2
    cat "$SERVER_LOG" >&2
    exit 1
fi

echo "HTTP smoke passed"