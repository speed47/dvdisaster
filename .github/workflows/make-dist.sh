#!/bin/bash
set -e

github_ref="$1"

case "$MSYSTEM" in
    MINGW64) os=win64;   suf=$os-portable; exe=.exe;;
    MINGW32) os=win32;   suf=$os-portable; exe=.exe;;
    *)       os=linux64; suf=$os-static;   exe='';;
esac

./dvdisaster$exe --version

if ./dvdisaster$exe --version | grep -q NOGUI; then
    GUI=0
    suffix=$suf-cli-only
else
    GUI=1
    suffix=$suf
fi

archive=dvdisaster-$(echo "$github_ref" | grep -Eo '[^/]+$')-$suffix.zip
[ -n "$GITHUB_OUTPUT" ] && echo "archive=$archive" >> "$GITHUB_OUTPUT"
echo "!> Appimage is <$archive>"

echo "!> Copying locales"
mkdir -p dist/locale
cp -va locale/*/ dist/locale/

# WINDOWS 32/64
if [ "$os" != "linux64" ]; then
  lookup_dependencies="dvdisaster"
  if [ "$GUI" = 1 ]; then
    echo "!> Copying default icons..."
    mkdir -p dist/share
    cp -va "$MINGW_PREFIX/share/icons" dist/share/
    loaders_cache="$(find "$MINGW_PREFIX/lib/gdk-pixbuf-2.0/" -name loaders.cache | head -n1)"
    pixbuf_dir="$(dirname "$loaders_cache")"
    echo "!> Absolute pixbuf directory is $pixbuf_dir"
    pixbuf_dir_relative=$(echo "$pixbuf_dir" | sed -re "s=$MINGW_PREFIX/==")
    echo "!> Relative pixbuf directory is $pixbuf_dir_relative"
    mkdir -p "dist/$pixbuf_dir_relative/loaders"
    cp -va "$loaders_cache" "dist/$pixbuf_dir_relative/"
    lookup_dependencies="$lookup_dependencies $(\ls -1 "$pixbuf_dir"/loaders/*.dll)"
  fi
  echo "!> Will look for dependencies of $lookup_dependencies"

  for binary in $lookup_dependencies
  do
    if [ "$binary" != "dvdisaster" ]; then
      echo "!> Copying pixbuf loader lib $binary to dist/$pixbuf_dir_relative/loaders..."
      cp -va "$binary" "dist/$pixbuf_dir_relative/loaders/"
    fi
    echo "!> Looking for dependencies of $binary..."
    # ntldd -R $binary
    for i in $(ntldd -R "$binary" | awk '/mingw/ {print $3}' | tr \\\\ / | grep -Eo '[^/]+$')
    do
      src="$MINGW_PREFIX/bin/$i"
      echo -n "!>> Inspecting $src... "
      if [ -e "$src" ]; then
        echo "found, copying"
        cp -va "$src" dist/
      else
        echo "not found, skipping"
      fi
    done
  done

  echo "!> Remove .a files if any"
  find dist -type f -name "*.a" -print -delete

  echo "!> Copy and rename text files"
  for file in CHANGELOG TODO COPYING CREDITS.* TODO
  do
    cp -va "$file" "dist/$file.txt"
  done

  echo "!> Copy other files"
  for file in dvdisaster.exe documentation/user-manual/manual.pdf
  do
    cp -va "$file" dist/
  done
# /WINDOWS
else
# Linux
  echo "!> Copying text, man and pdf files"
  cp -va CHANGELOG TODO COPYING CREDITS.* dvdisaster documentation/dvdisaster.*.1 documentation/user-manual/manual.pdf dist/
fi

echo "!> Building pdf from man"
man -t documentation/dvdisaster.en.1 | ps2pdf - dist/dvdisaster.pdf

echo "!> Building dist zip"
if command -v zip >/dev/null; then
    mv -v dist "${archive/.zip/}"
    zip -9r "$archive" "${archive/.zip/}"
    mv -v "${archive/.zip/}" dist
fi

echo "!> Dist done ($archive)"
