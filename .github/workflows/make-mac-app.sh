#!/bin/bash
set -e
set -x

github_ref="$1"

if ./dvdisaster --version | grep -q NOGUI; then
  suffix="-cli-only"
else
  suffix=""
fi

archive="dvdisaster-$(echo "$github_ref" | grep -Eo '[^/]+$')$suffix.dmg"
echo "Archive name is $archive"
echo "::set-output name=archive::$archive"

mkdir -p dist

# Create directory structure for the macOS application bundle
mkdir -p dvdisaster.app/Contents/{MacOS,Resources}

# Copy the main executable to the appropriate location
cp dvdisaster dvdisaster.app/Contents/MacOS

# Use dylibbundler to bundle dynamic libraries into the application bundle
dylibbundler -od -cd -b -x dvdisaster.app/Contents/MacOS/dvdisaster -d 'dvdisaster.app/Contents/libs/'

# Copy the Info.plist file to define application metadata
cp macinst/Info.plist dvdisaster.app/Contents/

# Create a directory for documentation resources
mkdir dvdisaster.app/Contents/Resources/documentation

# Copy documentation files to the documentation directory
cp CHANGELOG TODO COPYING CREDITS.* documentation/dvdisaster.*.1 documentation/user-manual/manual.pdf dvdisaster.app/Contents/Resources/documentation

# Copy localization files to the locale directory
find locale/* -maxdepth 0 -type d -exec cp -r {} dvdisaster.app/Contents/Resources/locale/ \;

# Copy the application icon to the resources directory
cp macinst/dvdisaster.icns dvdisaster.app/Contents/Resources/

# Make the main executable executable
chmod +x dvdisaster.app/Contents/MacOS/dvdisaster

# Move the application bundle to the 'dist' directory
mv dvdisaster.app dist

create-dmg "$archive" dist

echo "dist done ($archive)"
