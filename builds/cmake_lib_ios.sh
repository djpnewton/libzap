#!/bin/bash

set -e

# get command line params
sim=$1
static=$2

MAKE=make
CMAKE_TOOLCHAIN_FILE=ios.toolchain.cmake
if [ "$sim" == "sim" ]; then
    PLATFORM=SIMULATOR64
else
    PLATFORM=OS
fi
if [ "$static" == "static" ]; then
    BUILD_STATIC=true
else
    BUILD_STATIC=
fi
DEPS=`realpath ../deps`
OPENSSL_ROOT_DIR=$DEPS/ios/openssl
LIBCURL_ROOT_DIR=$DEPS/curl-android-ios-2aead71c1921d87cf7330d2acd581b1307adb1e1/prebuilt-with-ssl/iOS
JANSSON_ROOT_DIR=$DEPS/jansson-2.12

if [ ! -d $OPENSSL_ROOT_DIR ]; then
    mkdir -p $DEPS/ios
    # from https://www.teskalabs.com/blog/openssl-binary-distribution-for-developers-static-library
    wget -nc https://getseacatiostoracc.blob.core.windows.net/getseacatio/openssl/openssl-dev-1.0.2o-ios.tar.gz -O $DEPS/openssl-ios.tar.gz
    tar xvf $DEPS/openssl-ios.tar.gz -C $DEPS/ios
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

mkdir -p ios

cmake \
    -DJANSSON_BUILD_DOCS=OFF \
    -DCMAKE_C_FLAGS=-std=c11 \
    -DOPENSSL_ROOT_DIR=$OPENSSL_ROOT_DIR -DOPENSSL_INCLUDE_DIR=$OPENSSL_ROOT_DIR/include \
    -DOPENSSL_CRYPTO_LIBRARY=$OPENSSL_ROOT_DIR/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=$OPENSSL_ROOT_DIR/lib/libssl.a \
    -DCURL_LIBRARY=$LIBCURL_ROOT_DIR/libcurl.a -DCURL_INCLUDE_DIR=$LIBCURL_ROOT_DIR/include \
    -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../$CMAKE_TOOLCHAIN_FILE \
    -DPLATFORM=$PLATFORM \
    -DENABLE_BITCODE=1 \
    -DBUILD_STATIC=$BUILD_STATIC \
    -Bios -H../src
