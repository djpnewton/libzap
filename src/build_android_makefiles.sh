#!/bin/sh

set -e

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
ANDROID_ABI=armeabi-v7a
ANDROID_API=18
DEPS=../deps
OPENSSL_ROOT_DIR=$DEPS/openssl/$ANDROID_ABI
LIBCURL_ROOT_DIR=$DEPS/curl-android-ios-2aead71c1921d87cf7330d2acd581b1307adb1e1/prebuilt-with-ssl/android
JANSSON_ROOT_DIR=$DEPS/jansson-2.11

if [ ! -d $OPENSSL_ROOT_DIR ]; then
    # from https://www.teskalabs.com/blog/openssl-binary-distribution-for-developers-static-library
    wget -nc https://getseacatiostoracc.blob.core.windows.net/getseacatio/openssl/openssl-dev-1.0.2o-android.tar.gz
    mkdir -p $DEPS
    tar xvf openssl-dev-1.0.2o-android.tar.gz -C $DEPS
fi

if [ ! -d $LIBCURL_ROOT_DIR ]; then
    # from https://github.com/gcesarmza/curl-android-ios
    wget -nc https://github.com/gcesarmza/curl-android-ios/archive/2aead71c1921d87cf7330d2acd581b1307adb1e1.zip -O libcurl-android-ios.zip
    mkdir -p $DEPS
    unzip libcurl-android-ios.zip -d $DEPS
fi

if [ ! -d $JANSSON_ROOT_DIR ]; then
    wget -nc https://github.com/akheron/jansson/archive/v2.11.zip -O ../jansson_v2.11.zip
    mkdir -p $DEPS
    unzip ../jansson_v2.11.zip -d $DEPS
fi

cmake "-GUnix Makefiles" \
    -DJANSSON_BUILD_DOCS=OFF \
    -DCMAKE_C_FLAGS=-std=c11 \
    -DOPENSSL_ROOT_DIR=$OPENSSL_ROOT_DIR -DOPENSSL_INCLUDE_DIR=$OPENSSL_ROOT_DIR/include \
    -DOPENSSL_CRYPTO_LIBRARY=$OPENSSL_ROOT_DIR/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=$OPENSSL_ROOT_DIR/lib/libssl.a \
    -DCURL_LIBRARY=$LIBCURL_ROOT_DIR/$ANDROID_ABI/libcurl.a -DCURL_INCLUDE_DIR=$LIBCURL_ROOT_DIR/include \
    -DANDROID_ABI=$ANDROID_ABI -DANDROID_NDK=$NDK -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=ndk_build/$ANDROID_ABI \
    -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=$MAKE -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE \
    -DANDROID_NATIVE_API_LEVEL=$ANDROID_API -DANDROID_TOOLCHAIN=gcc
