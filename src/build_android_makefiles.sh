#!/bin/sh

set -e

NDK=/usr/lib/android-ndk
MAKE=make
CMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake
ANDROID_ABI=armeabi-v7a
ANDROID_API=18
DEPS=../deps
OPENSSL_ROOT_DIR=$DEPS/openssl/$ANDROID_ABI

if [ ! -d $OPENSSL_ROOT_DIR ]; then
    # from https://www.teskalabs.com/blog/openssl-binary-distribution-for-developers-static-library
    wget -nc https://getseacatiostoracc.blob.core.windows.net/getseacatio/openssl/openssl-dev-1.0.2o-android.tar.gz
    mkdir -p $DEPS
    tar xvf openssl-dev-1.0.2o-android.tar.gz -C $DEPS
fi

cmake "-GUnix Makefiles" \
    -DCMAKE_C_FLAGS=-std=c11 \
    -DOPENSSL_ROOT_DIR=$OPENSSL_ROOT_DIR -DOPENSSL_INCLUDE_DIR=$OPENSSL_ROOT_DIR/include \
    -DOPENSSL_CRYPTO_LIBRARY=$OPENSSL_ROOT_DIR/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=$OPENSSL_ROOT_DIR/lib/libssl.a \
    -DANDROID_ABI=$ANDROID_ABI -DANDROID_NDK=$NDK -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=ndk_build/$ANDROID_ABI \
    -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=$MAKE -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE \
    -DANDROID_NATIVE_API_LEVEL=$ANDROID_API -DANDROID_TOOLCHAIN=gcc
