#!/bin/sh
# Turns the installed dvdisaster into an AppImage which brings its own libc
# and therefore does not depend on the glibc of the host.
# Run from the root of the source tree after get-dependencies.sh;
# OUTPUT_VERSION (default: the dvdisaster version) goes into the file name.

set -eu

ARCH=$(uname -m)
VERSION=${OUTPUT_VERSION:-$(cat ~/version)}
export ARCH
export OUTPATH=./dist
export OUTNAME="dvdisaster-${VERSION}-${ARCH}.AppImage"
export ICON=./contrib/dvdisaster64.png
export DESKTOP=./contrib/dvdisaster.desktop

# Deploy dependencies
quick-sharun /usr/bin/dvdisaster /usr/share/doc/dvdisaster

# dvdisaster looks for its documentation and binary directory in the environment
echo 'DVDISASTER_APPIMAGE=1'                      >> ./AppDir/.env
echo 'DOCDIR=${SHARUN_DIR}/share/doc/dvdisaster' >> ./AppDir/.env
echo 'BINDIR=${SHARUN_DIR}/bin'                  >> ./AppDir/.env

# Turn AppDir into AppImage
quick-sharun --make-appimage

# Start it for a few seconds as a smoke test
quick-sharun --simple-test "$OUTPATH/$OUTNAME"
