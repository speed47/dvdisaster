#!/usr/bin/env bash
cd "$(dirname "$0")"
allfailed=0
for i in rs*.bash; do
   ( DVDISASTER_SCSI_SIMULATED_NODELAY=1 REGTEST_NO_UTF8=1 ./$i ); ret=$?
   allfailed=$((allfailed + ret))
done
echo Failed $allfailed tests
exit $allfailed
