#!/bin/sh
# Installs the dependencies and builds dvdisaster inside the container used
# for the AppImage (Arch Linux, or Debian/Ubuntu). Run from the root of the
# source tree.

set -eu

# the mirrors occasionally drop connections mid transaction
retry() {
	n=0
	until "$@"; do
		n=$((n + 1))
		[ "$n" -lt 4 ] || return 1
		sleep 5
	done
}

# librsvg is the SVG loader of gdk-pixbuf: the icons are SVG, and without it
# the AppImage shows missing-image icons on hosts which do not have it
echo "Installing package dependencies..."
echo "---------------------------------------------------------------"
if command -v pacman >/dev/null 2>&1; then
	retry pacman -Syu --noconfirm \
	    file gdk-pixbuf2 gettext glib-networking gtk3 librsvg man-db pkgconf
else
	export DEBIAN_FRONTEND=noninteractive
	retry apt-get update
	retry apt-get install -y --no-install-recommends \
	    build-essential ca-certificates curl file gettext glib-networking \
	    libgdk-pixbuf2.0-dev libglib2.0-dev libgtk-3-dev librsvg2-common man-db \
	    patchelf pkgconf strace xauth xvfb zsync
fi

echo "Building dvdisaster..."
echo "---------------------------------------------------------------"
./configure --prefix=/usr --docsubdir=dvdisaster   # otherwise named after the source directory
make -j"$(nproc)" && make
touch documentation/user-manual/manual.pdf   # the manual is not built here
make install

# Version for the file name, e.g. "0.79.10" or "0.79.10-p6"
./dvdisaster --version | grep -m 1 '^dvdisaster' \
  | sed -E 's/^dvdisaster ([0-9.]+) \(unofficial patchlevel ([0-9]+)\).*/\1-p\2/; s/^dvdisaster ([0-9.]+).*/\1/' \
  > ~/version
echo "Version: $(cat ~/version)"
