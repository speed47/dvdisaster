#!/usr/bin/env bash
cd "$(dirname "$0")"

TMPDIR=/dev/shm
if ! test -d $TMPDIR; then
    TMPDIR=/var/tmp
fi
LOGDIR="$TMPDIR"

INTERACTIVE_DIR="${TMPDIR}/regtest_interactive"
mkdir -p "$INTERACTIVE_DIR"

export REGTEST_RUNNER=1

# Run all codec test scripts in parallel
pids=()
scripts=()
for i in rs*.bash; do
   ( DVDISASTER_SCSI_SIMULATED_NODELAY=1 ./$i ) &
   pids+=($!)
   scripts+=($i)
done

# Wait for all and collect exit codes
allfailed=0
for idx in "${!pids[@]}"; do
   wait "${pids[$idx]}"; ret=$?
   if [ $ret -ne 0 ]; then
       echo "${scripts[$idx]}: $ret test(s) failed"
   fi
   allfailed=$((allfailed + ret))
done

# Handle interactive diffs deferred from backgrounded codec scripts
if [ $allfailed -gt 0 ] && find "$INTERACTIVE_DIR" -maxdepth 1 -name '*.interactive' -print -quit | grep -q .; then
    . ./interactive-diff.bash
    echo ""
    echo "=== Interactive diff review ==="
    _handle_interactive_diffs
    accepted=$?
    allfailed=$((allfailed - accepted))
    [ $allfailed -lt 0 ] && allfailed=0
fi
rm -rf "$INTERACTIVE_DIR"

echo ""
echo "Failed $allfailed tests total"
[ $allfailed -ge 256 ] && allfailed=255
exit $allfailed
