#!/bin/sh

set -e

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

ARCHS="armv7 armv7s arm64"

for arch in $ARCHS
do
  for lib in $LIBS
  do
    lipo -extract $arch $INPATH/$LIBPREFIX$lib$LIBEXT -o $LIBPREFIX$lib-$arch$LIBEXT
  done
  INLIBS=`eval echo $LIBPREFIX\{${LIBS// /,}\}-$arch$LIBEXT`
  libtool -static -o $LIBPREFIX$OUT-$arch$LIBEXT $INLIBS
  rm $INLIBS
done

OUTLIBS=`eval echo $LIBPREFIX$OUT-\{${ARCHS// /,}\}$LIBEXT`
lipo -create $OUTLIBS -o $LIBPREFIX$OUT$LIBEXT
rm $OUTLIBS
