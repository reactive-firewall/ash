#!/bin/sh
# Minimal automated CI script.
# Assumes source repository is at /home/runner/work/.
set -e

printf "%s\n" "Start: $(date -u)" ;

# Run your project tests (example build-ash.sh)
if [ -x ./build-ash.sh ]; then
  ./build-ash.sh 2>&1
else
  printf "%s\n" "No build-ash.sh; smoke test OK" ;
fi

printf "%s\n" "End: $(date -u)" ;
sync
