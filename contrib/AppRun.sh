#!/bin/sh
DIR="$(readlink -f "$(dirname "$0")")"
export GTK_PATH="$DIR/usr/lib/gtk-2.0"
export DVDISASTER_DOCDIR="$DIR/usr/share/doc/dvdisaster"
exec "$DIR/usr/bin/dvdisaster" "$@"
