#!/bin/sh

set -e

sim=$1

DEPS=../deps
OPENSSL_ROOT_DIR=$DEPS/ios/openssl
LIBCURL_ROOT_DIR=$DEPS/curl-android-ios-2aead71c1921d87cf7330d2acd581b1307adb1e1/prebuilt-with-ssl/iOS

INPATH="xcode_build/in"
INTPATH="xcode_build/int"
OUTPATH="xcode_build"

# copy all libs to xcode_build
mkdir -p $INPATH
cp $OPENSSL_ROOT_DIR/lib/* $INPATH
cp $LIBCURL_ROOT_DIR/libcurl.a $INPATH
cp ios/build/jansson/lib/* $INPATH
cp ios/build/waves-c/lib/* $INPATH

# combine static libs

LIBPREFIX="lib"
LIBS="crypto curl jansson ssl waves_c z zap"
LIBEXT=".a"

OUT="zap_combined"

if [ "$sim" == "sim" ]; then
  ARCHS="x86_64"
else
  ARCHS="armv7 armv7s arm64"
fi

mkdir -p $INTPATH
for arch in $ARCHS
do
  for lib in $LIBS
  do
    if lipo -info $INPATH/$LIBPREFIX$lib$LIBEXT | grep -q "Non-fat"; then
      cp $INPATH/$LIBPREFIX$lib$LIBEXT $INTPATH/$LIBPREFIX$lib-$arch$LIBEXT
    else
      lipo -extract $arch $INPATH/$LIBPREFIX$lib$LIBEXT -o $INTPATH/$LIBPREFIX$lib-$arch$LIBEXT
    fi
  done
  INLIBS=`eval echo $INTPATH/$LIBPREFIX\{${LIBS// /,}\}-$arch$LIBEXT`
  libtool -static -o $INTPATH/$LIBPREFIX$OUT-$arch$LIBEXT $INLIBS
  rm $INLIBS
done

if [ "$sim" == "sim" ]; then
  OUTLIBS=$INTPATH/$LIBPREFIX$OUT-${ARCHS}$LIBEXT
else
  OUTLIBS=`eval echo $INTPATH/$LIBPREFIX$OUT-\{${ARCHS// /,}\}$LIBEXT`
fi

lipo -create $OUTLIBS -o $OUTPATH/$LIBPREFIX$OUT$LIBEXT
rm $OUTLIBS
