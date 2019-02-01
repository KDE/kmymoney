#!/bin/bash

# Halt on errors and be verbose about what we are doing
set -e
set -x

# Read in our parameters
export BUILD_PREFIX=$1
export KMYMONEY_SOURCES=$2

# Save some frequently referenced locations in variables for ease of use / updating
export APPDIR=$BUILD_PREFIX/kmymoney.appdir
export PLUGINS=$APPDIR/usr/lib/plugins/
export APPIMAGEPLUGINS=$APPDIR/usr/plugins/

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment.
# That's not always the case, so make sure it is
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

# We want to use $prefix/deps/usr/ for all our dependencies
export DEPS_INSTALL_PREFIX=$BUILD_PREFIX/deps/usr/
export DOWNLOADS_DIR=$BUILD_PREFIX/downloads/

# Setup variables needed to help everything find what we built
export LD_LIBRARY_PATH=$DEPS_INSTALL_PREFIX/lib/:$DEPS_INSTALL_PREFIX/lib/x86_64-linux-gnu/:$APPDIR/usr/lib/:$LD_LIBRARY_PATH
export PATH=$DEPS_INSTALL_PREFIX/bin/:$PATH
export PKG_CONFIG_PATH=$DEPS_INSTALL_PREFIX/share/pkgconfig/:$DEPS_INSTALL_PREFIX/lib/pkgconfig/:/usr/lib/pkgconfig/:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$DEPS_INSTALL_PREFIX:$CMAKE_PREFIX_PATH

# Switch over to our build prefix
cd $BUILD_PREFIX

#
# Now we can get the process started!
#

# Step 0: place the translations where ki18n and Qt look for them
if [ -d $APPDIR/usr/share/locale ] ; then
    mv $APPDIR/usr/share/locale $APPDIR/usr/share/kmymoney
fi

# Step 1: Copy over all the resources provided by dependencies that we need
cp -r $DEPS_INSTALL_PREFIX/share/locale $APPDIR/usr/share/kmymoney
cp -r $DEPS_INSTALL_PREFIX/share/kf5 $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/share/mime $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/translations $APPDIR/usr/
cp -r $DEPS_INSTALL_PREFIX/openssl/lib/*  $APPDIR/usr/lib

# Step 2: Relocate x64 binaries from the architecture specific directory as required for Appimages
mv $APPDIR/usr/lib/x86_64-linux-gnu/*  $APPDIR/usr/lib
rm -rf $APPDIR/usr/lib/x86_64-linux-gnu/

# Step 3: Update the rpath in the various plugins we have to make sure they'll be loadable in an Appimage context
for lib in $PLUGINS/kmymoney/*.so*; do
  patchelf --set-rpath '$ORIGIN/../../lib' $lib;
done

# Step 4: Move plugins to loadable location in AppImage

# Make sure our plugin directory already exists
if [ ! -d $APPIMAGEPLUGINS ] ; then
    mkdir -p $APPIMAGEPLUGINS
fi

mv $PLUGINS/* $APPIMAGEPLUGINS

# Step 5: Determine the version of KMyMoney we have just built
# This is needed for linuxdeployqt/appimagetool to do the right thing
cd $KMYMONEY_SOURCES
KMYMONEY_VERSION=$(grep "KMyMoney VERSION" CMakeLists.txt | cut -d '"' -f 2)

# Also find out the revision of Git we built
# Then use that to generate a combined name we'll distribute
if [[ -d .git ]]; then
        GIT_REVISION=$(git rev-parse --short HEAD)
        export VERSION=$KMYMONEY_VERSION-$GIT_REVISION
else
        export VERSION=$KMYMONEY_VERSION
fi

# Finally transition back to the build directory so we can build the appimage
cd $BUILD_PREFIX

# Step 6: Build the image!!!
linuxdeployqt $APPDIR/usr/share/applications/org.kde.kmymoney.desktop \
  -executable=$APPDIR/usr/bin/kmymoney \
  -qmldir=$DEPS_INSTALL_PREFIX/qml \
  -verbose=2 \
  -bundle-non-qt-libs \
  -appimage \
  -exclude-libs=libnss3.so,libnssutil3.so
