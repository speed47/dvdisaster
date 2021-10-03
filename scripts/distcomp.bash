#!/usr/bin/env bash

if test $# != 1 || ! test -d $1; then
  echo "Usage: distcomp <path to alternate distribution>"
  echo 
  echo "- call within source directory, e.g. dvdisaster-0.72"
  echo "- make sure both are in distclean state"
  exit 1
fi

HEAD_LINES=10

ref=$1
new=$(pwd)

# Make sure we're talking about same stuff

echo "Old distribution: $ref"
echo "New distribution: $new"
echo 

# Looks for added files

ADDED=0
echo "Files and dirs ADDED in this distribution:"
for i in $(find .); do
  if test "${i:0:9}" == "./PRIVATE";
    then continue;
  fi
  if test -d $i && ! test -e $ref/$i; then
    ADDED=$((ADDED+1))
    echo "  Dir : $i"
  fi
  if test -f $i && ! test -e $ref/$i; then
    ADDED=$((ADDED+1))
    echo "  File: $i"
  fi
done

if test $ADDED == 0; then
  echo "  None"
fi

# Looks for removed files

cd $ref
REMOVED=0
echo
echo "Files and dirs REMOVED in this distribution:"
for i in $(find .); do
  if test "${i:0:5}" == "./.hg";
    then continue;
  fi
  if test -d $i && ! test -e $new/$i; then
    REMOVED=$((REMOVED+1))
    echo "  Dir : $i"
  fi
  if test -f $i && ! test -e $new/$i; then
    REMOVED=$((REMOVED+1))
    echo "  File: $i"
  fi
done

if test $REMOVED == 0; then
  echo "  None"
fi

cd $new
CHANGED=0
echo 
echo "Files CHANGED in this distribution:"
for i in $(find .); do
  if test -f $i && test -f $ref/$i; then
    if ! cmp -s $i $ref/$i; then
       echo $i
       diff $ref/$i $i | head -n $HEAD_LINES
       echo
       CHANGED=$((CHANGED+1))
    fi
  fi
done

if test $CHANGED == 0; then
  echo "  None"
fi

echo
if test $((CHANGED+ADDED+REMOVED)) == 0; then
  echo "No changes."
else
  echo "$CHANGED changed, $ADDED added and $REMOVED removed."
fi
