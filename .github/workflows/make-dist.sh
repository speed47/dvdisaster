#!/bin/bash
set -e
set -x

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
echo "Archive name is $archive"
echo "::set-output name=archive::$archive"

mkdir -p dist/locale
cp -vr locale/*/ dist/locale/
if [ "$os" != "linux64" ]; then
  if [ "$GUI" = 1 ]; then
    mkdir -p dist/share/themes dist/lib/gtk-2.0
    cp -vr $MINGW_PREFIX/share/themes/MS-Windows dist/share/themes/
    cp -vr $MINGW_PREFIX/lib/gtk-2.0/* dist/lib/gtk-2.0/
    rm -rf dist/lib/gtk-2.0/include
  fi
  mkdir -p dist/lib
  ntldd -R dvdisaster
  for i in $(ntldd -R dvdisaster | awk '/mingw/ {print $3}' | tr \\\\ / | grep -Eo '[^/]+$')
  do
    test -e $MINGW_PREFIX/bin/$i && cp -va $MINGW_PREFIX/bin/$i dist/
  done
  find dist -type f -name "*.a" -delete
fi
man -t documentation/dvdisaster.en.1 | ps2pdf - dist/dvdisaster.pdf
cp CHANGELOG TODO COPYING CREDITS.* dvdisaster documentation/dvdisaster.*.1 documentation/user-manual/manual.pdf dist/
if command -v zip >/dev/null; then
    mv dist ${archive/.zip/}
    zip -9r $archive ${archive/.zip/}
    mv ${archive/.zip/} dist
fi
echo "dist done ($archive)"