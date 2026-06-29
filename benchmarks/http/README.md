# HTTP benchmark scripts
These scripts are local development smoke checks for MVP 0.9.

They are not production benchmarks and do not define performance targets.

## Requirements
```text
bash
curl
date
seq
```

## Smoke
```bash
bash benchmarks/http/run_smoke.sh
```

The smoke script:
1. starts `velo serve`
2. sends curl requests to local endpoints
3. checks response status body
4. stops the server

## Curl loop
```bash
bash benchmarks/http/run_curl_loop.sh
```

Optional request count:
```bash
bash benchmarks/http/run_curl_loop.sh 100
```

The curl loop script prints:
```text
requests
elapsed_ms
approx_requests_per_second
```

## Notes
The default server address is currently fixed:
```text
127.0.0.1:8080
```

If port `8080` is already occupied, stop the other process first.

Future milestones may add configurable host/port flags.