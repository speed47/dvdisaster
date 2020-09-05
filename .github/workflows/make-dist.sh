#!/bin/bash
set -e
set -x

github_ref="$1"

if [ -n "$GITHUB_EVENT_PATH" ] && [ -f "$GITHUB_EVENT_PATH" ]; then
    if command -v jq >/dev/null; then
        upload_url=$(jq -r '.release.upload_url' < $GITHUB_EVENT_PATH)
        echo "Upload URL is $upload_url"
        echo "::set-output name=upload_url::$upload_url"
    fi
else
    echo "This should only be run from GitHub Actions"
    exit 1
fi

case "$MSYSTEM" in
    MINGW64) os=win64; exe=.exe;;
    MINGW32) os=win32; exe=.exe;;
    *)       os=linux64; exe='';;
esac

./dvdisaster$exe --version

if ./dvdisaster$exe --version | grep -q NOGUI; then
    GUI=0
    suffix="$os-cli-only"
else
    GUI=1
    suffix="$os"
fi

[ "$os" = linux64 ] && suffix="$os-static"

archive=dvdisaster-$(echo "$github_ref" | grep -Eo '[^/]+$')-$suffix.zip
echo "Archive name is $archive"
echo "::set-output name=archive::$archive"

mkdir -p dist/locale
cp -vr locale/*/ dist/locale/
if [ "$os" != "linux64" ]; then
  if [ "$GUI" = 1 ]; then
    mkdir -p dist/share/themes
    cp -vr $MINGW_PREFIX/share/themes/MS-Windows dist/share/themes/
    cp -vr $MINGW_PREFIX/lib/gtk-2.0 dist/lib/
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
cp dvdisaster documentation/dvdisaster.*.1 documentation/user-manual/manual.pdf dist/
if command -v zip >/dev/null; then
    mv dist ${archive/.zip/}
    zip -9r $archive ${archive/.zip/}
    mv ${archive/.zip/} dist
fi
echo "dist done ($archive)"
