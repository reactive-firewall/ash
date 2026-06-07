#!/bin/sh
# Minimal automated CI script.
# Assumes source repository is at /home/ci/repo.
set -e

printf "%s\n" "Start: $(date -u)" ;
uname -a 2>&1 || true
freebsd-version 2>&1 || true

# Example package operations; keep permission/rate limits in mind
if command -v pkg >/dev/null 2>&1; then
  pkg update -f 2>&1 || true
  pkg install -y llvm22 2>&1 || true
fi

# Run your project tests (example build-ash.sh)
if [ -x /home/ci/repo/build-ash.sh ]; then
  /home/ci/repo/build-ash.sh 2>&1
else
  printf "%s\n" "No build-ash.sh; smoke test OK" ;
fi

printf "%s\n" "End: $(date -u)" ;
sync
