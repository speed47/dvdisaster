#!/usr/bin/env bash
cd "$(dirname "$0")"

export DVDISASTER_SCSI_SIMULATED_NODELAY=1

if [ "$REGTEST_PARALLEL" = 1 ]; then
    for i in rs*.bash; do
       ( RETFILE=/tmp/result.$i REGTEST_NO_UTF8=1 ./$i ) &
    done
    wait
else
    for i in rs*.bash; do
       RETFILE=/tmp/result.$i ./$i
    done
fi

allfailed=0
for i in rs*.bash; do
   ret=$(cat /tmp/result.$i)
   [ -z "$ret" ] && ret=1
   rm -f /tmp/result.$i
   allfailed=$((allfailed + ret))
done

echo Failed $allfailed tests
[ $allfailed -ge 256 ] && allfailed=255
exit $allfailed
