#!/usr/bin/env bash

build=$(grep BUILD $1  | cut -d\  -f3)
build=$((build+1))
echo "#define BUILD $build" >$1
date=$(date +"%d.%m.%y (%A, %H:%M)")
echo "#define BDATE \"$date\"" >>$1
