#!/usr/bin/env bash

cd $1/documentation/wget-tmp

for file in ??/*.html; do
  new=`echo $file | sed -e "s/.php?/_/g" | sed -e "s/.php//g"`
  to=$1/documentation/$new
  cat $file | sed -e "s/.php#/.html#/g" | sed -e "s/.php%3F/_/g" | sed -e "s/.php//g" >$to
done
