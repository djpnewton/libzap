#!/bin/sh

set -e

emulator=$1

DEPS=../deps
OPENSSL_ROOT_DIR=$DEPS/openssl
LIBCURL_ROOT_DIR=$DEPS/curl-android-ios-2aead71c1921d87cf7330d2acd581b1307adb1e1/prebuilt-with-ssl/iOS

# copy all libs to xcode_build
cp $OPENSSL_ROOT_DIR/lib/* xcode_build/
cp $LIBCURL_ROOT_DIR/libcurl.a xcode_build/
cp build/jansson/lib/* xcode_build/
cp build/waves-c/lib/* xcode_build/

# combine static libs
INPATH="xcode_build"

LIBPREFIX="lib"
LIBS="crypto curl jansson ssl waves_c zap"
LIBEXT=".a"

OUT="zap_combined"

if [ "$emulator" == "emulator" ]; then
  ARCHS="x86_64"
else
  ARCHS="armv7 armv7s arm64 i386 x86_64"
fi

for arch in $ARCHS
do
  for lib in $LIBS
  do
    if lipo -info $INPATH/$LIBPREFIX$lib$LIBEXT | grep -q "Non-fat"; then
      cp $INPATH/$LIBPREFIX$lib$LIBEXT $LIBPREFIX$lib-$arch$LIBEXT
    else
      lipo -extract $arch $INPATH/$LIBPREFIX$lib$LIBEXT -o $LIBPREFIX$lib-$arch$LIBEXT
    fi
  done
  INLIBS=`eval echo $LIBPREFIX\{${LIBS// /,}\}-$arch$LIBEXT`
  libtool -static -o $LIBPREFIX$OUT-$arch$LIBEXT $INLIBS
  rm $INLIBS
done

if [ "$emulator" == "emulator" ]; then
  OUTLIBS=$LIBPREFIX$OUT-${ARCHS}$LIBEXT
else
  OUTLIBS=`eval echo $LIBPREFIX$OUT-\{${ARCHS// /,}\}$LIBEXT`
fi

lipo -create $OUTLIBS -o $LIBPREFIX$OUT$LIBEXT
rm $OUTLIBS
