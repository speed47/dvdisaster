#!/bin/bash
set -e
set -x

github_ref="$1"
arch="${2:-$(uname -m)}"

if ./dvdisaster --version | grep -q NOGUI; then
  suffix="-cli-only"
  GUI=0
else
  suffix=""
  GUI=1
fi

archive="dvdisaster-$(echo "$github_ref" | grep -Eo '[^/]+$')-macos-$arch$suffix.dmg"
[ -n "$GITHUB_OUTPUT" ] && echo "archive=$archive" >> "$GITHUB_OUTPUT"
echo "mac dmg is <$archive>"

mkdir -p dist

BREW_PREFIX="$(brew --prefix)"
REPO_DIR="$(pwd -P)"

# Create directory structure for the macOS application bundle
mkdir -p dvdisaster.app/Contents/{MacOS,Resources}

# Copy the main executable; rename it so the launcher script can wrap it
cp dvdisaster dvdisaster.app/Contents/MacOS/dvdisaster.bin
chmod +x dvdisaster.app/Contents/MacOS/dvdisaster.bin

# Use dylibbundler to bundle dynamic libraries into the application bundle
dylibbundler -od -cd -b -x dvdisaster.app/Contents/MacOS/dvdisaster.bin -d 'dvdisaster.app/Contents/libs/' -s "$BREW_PREFIX/lib"

# Copy the Info.plist file to define application metadata
cp macinst/Info.plist dvdisaster.app/Contents/

if [ "$GUI" = 1 ]; then
  # --- Bundle GDK-Pixbuf loaders ---
  # Find the versioned loaders directory inside the Homebrew gdk-pixbuf install
  pixbuf_loaders_src="$(find "$BREW_PREFIX/lib/gdk-pixbuf-2.0" -maxdepth 2 -name "loaders" -type d | head -1)"
  [ -z "$pixbuf_loaders_src" ] && { echo "ERROR: could not find gdk-pixbuf loaders directory under $BREW_PREFIX"; exit 1; }
  pixbuf_version="$(basename "$(dirname "$pixbuf_loaders_src")")"
  pixbuf_bundle_dir="dvdisaster.app/Contents/Resources/lib/gdk-pixbuf-2.0/$pixbuf_version"
  mkdir -p "$pixbuf_bundle_dir/loaders"

  # Copy all loader shared objects (.so on macOS Homebrew, possibly .dylib)
  cp "$pixbuf_loaders_src/"*.so   "$pixbuf_bundle_dir/loaders/" 2>/dev/null || true
  cp "$pixbuf_loaders_src/"*.dylib "$pixbuf_bundle_dir/loaders/" 2>/dev/null || true

  # Verify that at least one loader was copied before proceeding
  loader_count=$(find "$pixbuf_bundle_dir/loaders" -maxdepth 1 -type f | wc -l)
  [ "$loader_count" -eq 0 ] && { echo "ERROR: no pixbuf loaders found in $pixbuf_loaders_src"; exit 1; }

  # Fix up dylib dependencies for each loader so they use our bundled libs
  for loader in "$pixbuf_bundle_dir/loaders/"*; do
    [ -f "$loader" ] && dylibbundler -od -cd -b -x "$loader" -d 'dvdisaster.app/Contents/libs/' -s "$BREW_PREFIX/lib" || true
  done

  # Generate a loaders.cache with a placeholder for the loaders directory path.
  # At runtime the launcher script will sed-replace the placeholder with the
  # actual absolute path inside the (possibly moved) application bundle.
  gdk-pixbuf-query-loaders "$pixbuf_bundle_dir/loaders/"* | \
    sed "s|$REPO_DIR/$pixbuf_bundle_dir/loaders|@GDK_PIXBUF_MODULEDIR@|g" \
    > "$pixbuf_bundle_dir/loaders.cache"

  # --- Bundle icon themes ---
  # Adwaita provides the standard GTK icon set; hicolor is the required fallback.
  mkdir -p dvdisaster.app/Contents/Resources/share/icons
  for theme in Adwaita hicolor; do
    [ -d "$BREW_PREFIX/share/icons/$theme" ] && \
      cp -r "$BREW_PREFIX/share/icons/$theme" dvdisaster.app/Contents/Resources/share/icons/
  done

  # --- Bundle GLib/GTK GSettings schemas ---
  mkdir -p dvdisaster.app/Contents/Resources/share/glib-2.0
  cp -r "$BREW_PREFIX/share/glib-2.0/schemas" dvdisaster.app/Contents/Resources/share/glib-2.0/

  # --- Create the GUI launcher script ---
  # This wrapper sets the GTK/GDK environment variables that point to the
  # bundled resources before exec-ing the real binary.
  cat > dvdisaster.app/Contents/MacOS/dvdisaster << EOF
#!/bin/bash
DIR="\$(cd "\$(dirname "\$0")/../.." && pwd)"

# Patch the pixbuf loaders cache with the actual runtime path and export it
GDK_PIXBUF_LOADERS_DIR="\$DIR/Contents/Resources/lib/gdk-pixbuf-2.0/$pixbuf_version/loaders"
GDK_PIXBUF_CACHE_TEMPLATE="\$DIR/Contents/Resources/lib/gdk-pixbuf-2.0/$pixbuf_version/loaders.cache"
if [ -f "\$GDK_PIXBUF_CACHE_TEMPLATE" ]; then
  GDK_PIXBUF_CACHE="\$(mktemp -t dvdisaster-pixbuf)"
  sed "s|@GDK_PIXBUF_MODULEDIR@|\$GDK_PIXBUF_LOADERS_DIR|g" "\$GDK_PIXBUF_CACHE_TEMPLATE" > "\$GDK_PIXBUF_CACHE"
  export GDK_PIXBUF_MODULE_FILE="\$GDK_PIXBUF_CACHE"
fi

# Point GTK to our bundled icon themes and other data files
export XDG_DATA_DIRS="\$DIR/Contents/Resources/share:\${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

# Point GSettings to our bundled compiled schemas
export GSETTINGS_SCHEMA_DIR="\$DIR/Contents/Resources/share/glib-2.0/schemas"

exec "\$DIR/Contents/MacOS/dvdisaster.bin" "\$@"
EOF
  chmod +x dvdisaster.app/Contents/MacOS/dvdisaster

else
  # --- Create the CLI launcher script ---
  cat > dvdisaster.app/Contents/MacOS/dvdisaster << 'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")/../.." && pwd)"
exec "$DIR/Contents/MacOS/dvdisaster.bin" "$@"
EOF
  chmod +x dvdisaster.app/Contents/MacOS/dvdisaster
fi

# Create a directory for documentation resources
mkdir -p dvdisaster.app/Contents/Resources/documentation

# Copy documentation files to the documentation directory
cp CHANGELOG TODO COPYING CREDITS.* documentation/dvdisaster.*.1 documentation/user-manual/manual.pdf dvdisaster.app/Contents/Resources/documentation

# Create locale directory and copy localization files
mkdir -p dvdisaster.app/Contents/Resources/locale
find locale/* -maxdepth 0 -type d -exec cp -r {} dvdisaster.app/Contents/Resources/locale/ \;

# Copy the application icon to the resources directory
cp macinst/dvdisaster.icns dvdisaster.app/Contents/Resources/

# Move the application bundle to the 'dist' directory
mv dvdisaster.app dist

create-dmg "$archive" dist

echo "dist done ($archive)"
