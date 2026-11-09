#!/usr/bin/env sh
set -eu
BASE_URL="${BASE_URL:-http://127.0.0.1:8080}"

check() {
  expected="$1"
  method="$2"
  path="$3"
  actual="$(curl -sS -o /dev/null -w '%{http_code}' -X "$method" "${BASE_URL}${path}")"
  printf '%-5s %-25s -> %s (esperado %s)\n' "$method" "$path" "$actual" "$expected"
  [ "$actual" = "$expected" ]
}

check 200 GET  /health.txt
check 200 GET  /api/admin
check 403 POST /api/admin
check 403 POST '/api/admin?teste=1'

echo 'OK: milestone 01 validado.'
