#!/usr/bin/env bash
cd "$(dirname "$0")"

for i in rs*.bash; do
   ( DVDISASTER_SCSI_SIMULATED_NODELAY=1 RETFILE=/tmp/result.$i PARALLEL=1 ./$i ) &
done

wait

allfailed=0
for i in rs*.bash; do
   ret=$(cat /tmp/result.$i)
   rm -f /tmp/result.$i
   allfailed=$((allfailed + ret))
done

echo Failed $allfailed tests
[ $allfailed -ge 256 ] && allfailed=255
exit $allfailed
