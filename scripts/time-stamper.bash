#!/usr/bin/env bash
build=$(grep BUILDNUM $1 2>/dev/null | cut -d\  -f3)
build=$((build+1))
flavor=$(grep FLAVOR $1 2>/dev/null | cut -d\  -f3)
if [ -z "$flavor" ]; then
   flavor=UNKNOWN
fi
echo "#define FLAVOR $flavor" >$1
echo "#define BUILDNUM $build" >>$1
if git describe >/dev/null 2>/dev/null; then
   gitver=$(git describe --tags --dirty)
   echo "#define BUILD \"$gitver-$flavor-speed47.build$build\"" >>$1
else
   echo "#define BUILD \"$gitver-speed47.build$build\"" >$1
fi
date=$(date +"%d.%m.%y (%A, %H:%M)")
echo "#define BDATE \"$date\"" >>$1
