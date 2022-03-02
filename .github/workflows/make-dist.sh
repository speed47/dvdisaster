#!/bin/bash
set -e
set -x

github_ref="$1"

case "$MSYSTEM" in
    MINGW64) os=win64;   suf=$os-portable; exe=.exe;;
    MINGW32) os=win32;   suf=$os-portable; exe=.exe;;
    *)
        if [ "$(uname -s)" = Darwin ]; then
            os=darwin; suf=$os; exe=''
        else
            os=linux64; suf=$os-static; exe=''
        fi
    ;;
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
if [[ $os =~ ^win ]]; then
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
elif [ $os = darwin ]; then
  mkdir -p opt

  # copy all directly referenced libs
  for dir in $(otool -L dvdisaster | awk '/usr\/local/ {print $1}' | cut -d/ -f1-5 | sort -u); do
    cp -r $dir opt/
  done

  # copy all indirectly referenced libs (libs of libs)
  for dir in $(find opt -name "*.dylib" | awk '/usr\/local/ {print $1}' | cut -d/ -f1-5 | sort -u); do
    test -e opt/$dir || cp -r $dir opt/
  done

  # patch the binary to set relative paths for libs
  for i in $(otool -L dvdisaster | awk '/usr\/local/ {print $1}'); do
    j=$(echo $i | cut -d/ -f4-)
    install_name_tool -change "$i" "@executable_path/$j" dvdisaster
  done

  # patch the libs to set relative paths for libs of libs
  for i in $(find opt -name "*.dylib"); do
    echo
    for j in $(otool -L $i 2>/dev/null | awk '/usr\/local/ {print $1}'); do
      k=$(find opt -name $(basename $j) | head -n1)
      if [ -z "$k" ]; then
        echo "MISSING LIB: $j"
        k=opt/$(basename $j)
        cp -v "$j" "$k"
      fi
      install_name_tool -change "$j" "@executable_path/$k" "$i"
    done
  done

  mv opt dist/
fi

man -t documentation/dvdisaster.en.1 | ps2pdf - dist/dvdisaster.pdf
cp CHANGELOG TODO COPYING CREDITS.* dvdisaster documentation/dvdisaster.*.1 documentation/user-manual/manual.pdf dist/

if command -v zip >/dev/null; then
    mv dist ${archive/.zip/}
    zip -9r $archive ${archive/.zip/}
    mv ${archive/.zip/} dist
fi

echo "dist done ($archive)"
