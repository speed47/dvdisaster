#!/bin/sh
DIR="$(readlink -f "$(dirname "$0")")"

# When adding environment variables in this script, don't forget to sync with the src/show-manual.c
# list, as they need to be cleaned up before calling xdg-open to ensure xdg-open works with all the
# libs from the host and none from the AppImage (which most of the time just doesn't work).
# Also save the original value into an _ORIGINAL variable, which will be restored by dvdisaster
# into the xdg-open's environment before calling execve()

# Point to our own gtk libs
[ "_$GTK_PATH" != _ ] && export GTK_PATH_ORIGINAL="$GTK_PATH"
export GTK_PATH="$DIR/usr/lib/gtk-3.0"

# Load our own modules instead of the host ones,
# an absolute path pointing to the host is unfortunately hardcoded in ./usr/lib/libgio-2.0.so.0,
# but we edited the lib to neutralize said path (replaced gio/modules by :'s):
#
# $ strings ./usr/lib/libgio-2.0.so.0 | grep :::
# /usr/lib/:::::::::::
# /usr/lib/x86_64-linux-gnu/:::::::::::
#
# So the path below should be the only one used in the end:
[ "_$GIO_EXTRA_MODULES" != _ ] && export GIO_EXTRA_MODULES_ORIGINAL="$GIO_EXTRA_MODULES"
export GIO_EXTRA_MODULES="$DIR/usr/lib/gio/modules"

# To avoid getting:
# '''
# (dvdisaster:16170): Gtk-WARNING **: 14:31:41.224: Loading IM context type 'ibus' failed
# (dvdisaster:16170): Gtk-WARNING **: 14:31:41.224: /lib/x86_64-linux-gnu/libibus-1.0.so.5: undefined symbol: g_get_language_names_with_category
# '''
# We use xim instead, which is included in our build, along with the proper immodules cache file referencing our modules
[ "_$GTK_IM_MODULE_FILE" != _ ] && export GTK_IM_MODULE_FILE_ORIGINAL="$GTK_IM_MODULE_FILE"
export GTK_IM_MODULE_FILE="$(find "$DIR/" -name immodules.cache)"
[ "_$GTK_IM_MODULE" != _ ] && export GTK_IM_MODULE_ORIGINAL="$GTK_IM_MODULE"
export GTK_IM_MODULE=xim

# if host has GTK_MODULES set, empty it to prevent it from loading modules from the host
[ "_$GTK_MODULES" != _ ] && export GTK_MODULES_ORIGINAL="$GTK_MODULES"
export GTK_MODULES=''

# To avoid getting:
# '''
# (dvdisaster:16133): GLib-GIO-ERROR **: 14:25:53.270: Settings schema 'org.gnome.settings-daemon.plugins.xsettings' does not contain a key named 'antialiasing'
# Trace/breakpoint trap (core dumped)
# '''
# Under Ubuntu 22.04 and possibly later versions using Wayland
# https://github.com/Ultimaker/Cura/issues/12776
[ "_$GDK_BACKEND" != _ ] && export GDK_BACKEND_ORIGINAL="$GDK_BACKEND_ORIGINAL"
export GDK_BACKEND=x11

# To avoid getting:
# '''
# (evince:172616): dbind-WARNING **: 18:02:34.901: Couldn't connect to accessibility bus: Failed to connect to socket /run/user/1000/at-spi/bus: Permission denied
# '''
[ "_$NO_AT_BRIDGE" != _ ] && export NO_AT_BRIDGE_ORIGINAL="$NO_AT_BRIDGE"
export NO_AT_BRIDGE=1

# To avoid getting:
# '''
# (dvdisaster:20080): Gtk-WARNING **: 15:43:20.719: Could not load a pixbuf from icon theme.
# This may indicate that pixbuf loaders or the mime database could not be found.
# '''
# Point to our own patched cache file for gdk-pixbuf2
[ "_$GDK_PIXBUF_MODULE_FILE" != _ ] && export GDK_PIXBUF_MODULE_FILE_ORIGINAL="$GDK_PIXBUF_MODULE_FILE"
export GDK_PIXBUF_MODULE_FILE="$DIR/usr/lib/gdk-pixbuf2/loaders.cache"
# As the pixbuf loaders depends themselves on other libs, also adjust LD_LIBRARY_PATH so they load properly
[ "_$LD_LIBRARY_PATH" != _ ] && export LD_LIBRARY_PATH_ORIGINAL="$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH="$DIR/usr/lib:$LD_LIBRARY_PATH"

# Change to the proper directory because some .cache files have relative paths starting with "."
# we save the current PWD so that dvdisaster can use it as a default to store image and ecc files
export ORIGINAL_PWD="$PWD"
cd "$DIR" || exit 1

# Now run the program, with 3 vars it uses at runtime
export DVDISASTER_APPIMAGE=1
export DOCDIR="$DIR/usr/share/doc/dvdisaster"
export BINDIR="$DIR/usr/bin"
exec "$DIR/usr/bin/dvdisaster" "$@"
