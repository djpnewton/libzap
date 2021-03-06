#!/bin/bash

set -e

# get command line params
build_dir=$1
source_dir=$2
static=$3

if [ "$static" == "static" ]; then
    BUILD_STATIC=true
else
    BUILD_STATIC=
fi

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac
echo $machine

if [[ ! -z "${ANDROID_NDK}" ]]; then
  NDK=$ANDROID_NDK
elif [ "$machine" == "Mac" ]; then
  NDK=~/Library/Android/sdk/ndk-bundle
else
  NDK=/usr/lib/android-ndk
fi
MAKE=make
CMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake
if [[ -z "${ANDROID_ABI}" ]]; then
  ANDROID_ABI=armeabi-v7a
fi
ANDROID_API=18
DEPS=`realpath ../deps`
OPENSSL_ROOT_DIR=$DEPS/android/openssl/$ANDROID_ABI
LIBCURL_ROOT_DIR=$DEPS/curl-android-ios-2aead71c1921d87cf7330d2acd581b1307adb1e1/prebuilt-with-ssl/android
JANSSON_ROOT_DIR=$DEPS/jansson-2.12

if [ ! -d $OPENSSL_ROOT_DIR ]; then
    mkdir -p $DEPS/android
    # from https://www.teskalabs.com/blog/openssl-binary-distribution-for-developers-static-library
    wget -nc https://teskalabs.blob.core.windows.net/openssl/openssl-dev-1.0.2o-android.tar.gz -O $DEPS/openssl-android.tar.gz
    tar xvf $DEPS/openssl-android.tar.gz -C $DEPS/android
fi

if [ ! -d $LIBCURL_ROOT_DIR ]; then
    mkdir -p $DEPS
    # from https://github.com/gcesarmza/curl-android-ios
    wget -nc https://github.com/gcesarmza/curl-android-ios/archive/2aead71c1921d87cf7330d2acd581b1307adb1e1.zip -O $DEPS/libcurl-android-ios.zip
    unzip $DEPS/libcurl-android-ios.zip -d $DEPS
fi

if [ ! -d $JANSSON_ROOT_DIR ]; then
    mkdir -p $DEPS
    wget -nc https://github.com/akheron/jansson/archive/v2.12.zip -O $DEPS/jansson_v2.12.zip
    unzip $DEPS/jansson_v2.12.zip -d $DEPS
fi

mkdir -p $build_dir

cmake "-GUnix Makefiles" \
    -DJANSSON_BUILD_DOCS=OFF \
    -DCMAKE_C_FLAGS=-std=gnu11 \
    -DOPENSSL_ROOT_DIR=$OPENSSL_ROOT_DIR -DOPENSSL_INCLUDE_DIR=$OPENSSL_ROOT_DIR/include \
    -DOPENSSL_CRYPTO_LIBRARY=$OPENSSL_ROOT_DIR/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=$OPENSSL_ROOT_DIR/lib/libssl.a \
    -DCURL_LIBRARY=$LIBCURL_ROOT_DIR/$ANDROID_ABI/libcurl.a -DCURL_INCLUDE_DIR=$LIBCURL_ROOT_DIR/include \
    -DANDROID_ABI=$ANDROID_ABI -DANDROID_NDK=$NDK -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=ndk_build/$ANDROID_ABI \
    -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=$MAKE -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE \
    -DANDROID_NATIVE_API_LEVEL=$ANDROID_API \
    -DBUILD_STATIC=$BUILD_STATIC \
    -B$build_dir -H$source_dir
