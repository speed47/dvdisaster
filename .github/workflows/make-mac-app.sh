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

  # --- Rewrite Homebrew/rpath deps to @executable_path/../libs/ ---
  #
  # We can't use dylibbundler for the loaders because it errors out when
  # destination files already exist (from the main binary bundling step)
  # and -od wipes the destination directory.  Instead we use install_name_tool
  # directly.  The helper below handles both absolute Homebrew paths and
  # @rpath references: it copies the lib into Contents/libs/ if not already
  # present, then rewrites the reference in the target binary.
  rewrite_deps() {
    local target="$1"
    for dep in $(otool -L "$target" | tail -n +2 | awk '{print $1}'); do
      case "$dep" in
        /opt/homebrew/*|/usr/local/Cellar/*|/usr/local/opt/*)
          libname="$(basename "$dep")"
          if [ ! -f "dvdisaster.app/Contents/libs/$libname" ]; then
            echo "Bundling dep: $libname from $dep"
            cp "$dep" "dvdisaster.app/Contents/libs/$libname"
            chmod +w "dvdisaster.app/Contents/libs/$libname"
            install_name_tool -id "@executable_path/../libs/$libname" "dvdisaster.app/Contents/libs/$libname"
          fi
          install_name_tool -change "$dep" "@executable_path/../libs/$libname" "$target"
          ;;
        @rpath/*)
          libname="$(basename "$dep")"
          if [ ! -f "dvdisaster.app/Contents/libs/$libname" ]; then
            src="$(find "$BREW_PREFIX/lib" "$BREW_PREFIX/opt" -name "$libname" -not -type d 2>/dev/null | head -1)"
            if [ -n "$src" ]; then
              echo "Bundling @rpath dep: $libname from $src"
              cp "$src" "dvdisaster.app/Contents/libs/$libname"
              chmod +w "dvdisaster.app/Contents/libs/$libname"
              install_name_tool -id "@executable_path/../libs/$libname" "dvdisaster.app/Contents/libs/$libname"
            else
              echo "WARNING: could not find @rpath dep $libname"
            fi
          fi
          install_name_tool -change "$dep" "@executable_path/../libs/$libname" "$target"
          ;;
      esac
    done
  }

  # Rewrite deps in each pixbuf loader
  for loader in "$pixbuf_bundle_dir/loaders/"*; do
    [ -f "$loader" ] || continue
    chmod +w "$loader"
    rewrite_deps "$loader"
  done

  # Newly bundled libs (e.g. librsvg) may have their own transitive deps
  # still pointing to Homebrew — fix those up the same way.
  for lib in dvdisaster.app/Contents/libs/*; do
    [ -f "$lib" ] || continue
    rewrite_deps "$lib"
  done

  # Ad-hoc re-sign all loaders and libs — install_name_tool invalidates
  # existing signatures, and on Apple Silicon unsigned code cannot be
  # dlopen'd or executed.
  for f in "$pixbuf_bundle_dir/loaders/"* dvdisaster.app/Contents/libs/*; do
    [ -f "$f" ] && codesign --force --sign - "$f"
  done

  # Generate a loaders.cache with a placeholder for the loaders directory path.
  # At runtime the launcher script will sed-replace the placeholder with the
  # actual absolute path inside the (possibly moved) application bundle.
  # We query the ORIGINAL Homebrew loaders (not the bundled copies) because
  # the bundled copies have rewritten @executable_path deps that can't be
  # resolved by gdk-pixbuf-query-loaders on the build machine.
  gdk-pixbuf-query-loaders "$pixbuf_loaders_src/"* | \
    sed "s|$pixbuf_loaders_src|@GDK_PIXBUF_MODULEDIR@|g" \
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
  # When adding environment variables in this script, don't forget to sync with
  # the src/show-manual.c namelist, as they need to be cleaned up before calling
  # "open" to ensure the spawned viewer uses host libraries, not our bundled ones.
  # Also save the original value into a _ORIGINAL variable, which will be restored
  # by dvdisaster before calling execvp().
  cat > dvdisaster.app/Contents/MacOS/dvdisaster << EOF
#!/bin/bash
DIR="\$(cd "\$(dirname "\$0")/../.." && pwd)"

# Signal to dvdisaster that we are running from a macOS .app bundle
export DVDISASTER_MACOS_APP=1

# Patch the pixbuf loaders cache with the actual runtime path and export it
GDK_PIXBUF_LOADERS_DIR="\$DIR/Contents/Resources/lib/gdk-pixbuf-2.0/$pixbuf_version/loaders"
GDK_PIXBUF_CACHE_TEMPLATE="\$DIR/Contents/Resources/lib/gdk-pixbuf-2.0/$pixbuf_version/loaders.cache"
if [ -f "\$GDK_PIXBUF_CACHE_TEMPLATE" ]; then
  GDK_PIXBUF_CACHE="\$(mktemp -t dvdisaster-pixbuf)"
  sed "s|@GDK_PIXBUF_MODULEDIR@|\$GDK_PIXBUF_LOADERS_DIR|g" "\$GDK_PIXBUF_CACHE_TEMPLATE" > "\$GDK_PIXBUF_CACHE"
  [ "_\$GDK_PIXBUF_MODULE_FILE" != _ ] && export GDK_PIXBUF_MODULE_FILE_ORIGINAL="\$GDK_PIXBUF_MODULE_FILE"
  export GDK_PIXBUF_MODULE_FILE="\$GDK_PIXBUF_CACHE"
fi

# Point GTK to our bundled icon themes and other data files
[ "_\$XDG_DATA_DIRS" != _ ] && export XDG_DATA_DIRS_ORIGINAL="\$XDG_DATA_DIRS"
export XDG_DATA_DIRS="\$DIR/Contents/Resources/share:\${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

# Point GSettings to our bundled compiled schemas
[ "_\$GSETTINGS_SCHEMA_DIR" != _ ] && export GSETTINGS_SCHEMA_DIR_ORIGINAL="\$GSETTINGS_SCHEMA_DIR"
export GSETTINGS_SCHEMA_DIR="\$DIR/Contents/Resources/share/glib-2.0/schemas"

# If host has GTK_MODULES set, empty it to prevent loading modules from the host
[ "_\$GTK_MODULES" != _ ] && export GTK_MODULES_ORIGINAL="\$GTK_MODULES"
export GTK_MODULES=''

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
