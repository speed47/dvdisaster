#!/usr/bin/env bash
build=$(grep BUILDNUM $1 2>/dev/null | cut -d\  -f3)
build=$((build+1))
cat >$1 <<EOF
#ifdef CLI
#define FLAVOR CLI
#else
#define FLAVOR GUI
#endif
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
EOF
echo "#define BUILDNUM $build" >>$1
if git describe >/dev/null 2>/dev/null; then
   gitver=$(git describe --tags --dirty)
   echo "#define BUILD \"$gitver-\" TOSTRING(FLAVOR) \"-speed47.build$build\"" >>$1
else
   echo "#define BUILD TOSTRING(FLAVOR) \"-speed47.build$build\"" >>$1
fi
date=$(date +"%d.%m.%y (%A, %H:%M)")
echo "#define BDATE \"$date\"" >>$1
