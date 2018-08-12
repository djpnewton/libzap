#!/bin/sh

set -e

# get command line params
sim=$1

MAKE=make
CMAKE_TOOLCHAIN_FILE=ios.toolchain.cmake
if [ "$sim" == "sim" ]; then
    IOS_PLATFORM=SIMULATOR64
else
    IOS_PLATFORM=OS
fi
DEPS=../deps
OPENSSL_ROOT_DIR=$DEPS/openssl
LIBCURL_ROOT_DIR=$DEPS/curl-android-ios-2aead71c1921d87cf7330d2acd581b1307adb1e1/prebuilt-with-ssl/iOS
JANSSON_ROOT_DIR=$DEPS/jansson-2.11

if [ ! -d $OPENSSL_ROOT_DIR ]; then
    mkdir -p $DEPS
    # from https://www.teskalabs.com/blog/openssl-binary-distribution-for-developers-static-library
    wget -nc https://getseacatiostoracc.blob.core.windows.net/getseacatio/openssl/openssl-dev-1.0.2o-ios.tar.gz -O $DEPS/openssl-ios.tar.gz
    tar xvf $DEPS/openssl-ios.tar.gz -C $DEPS
fi

if [ ! -d $LIBCURL_ROOT_DIR ]; then
    mkdir -p $DEPS
    # from https://github.com/gcesarmza/curl-android-ios
    wget -nc https://github.com/gcesarmza/curl-android-ios/archive/2aead71c1921d87cf7330d2acd581b1307adb1e1.zip -O $DEPS/libcurl-android-ios.zip
    unzip $DEPS/libcurl-android-ios.zip -d $DEPS
fi

if [ ! -d $JANSSON_ROOT_DIR ]; then
    mkdir -p $DEPS
    wget -nc https://github.com/akheron/jansson/archive/v2.11.zip -O $DEPS/jansson_v2.11.zip
    unzip $DEPS/jansson_v2.11.zip -d $DEPS
fi

cmake \
    -DJANSSON_BUILD_DOCS=OFF \
    -DCMAKE_C_FLAGS=-std=c11 \
    -DOPENSSL_ROOT_DIR=$OPENSSL_ROOT_DIR -DOPENSSL_INCLUDE_DIR=$OPENSSL_ROOT_DIR/include \
    -DOPENSSL_CRYPTO_LIBRARY=$OPENSSL_ROOT_DIR/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=$OPENSSL_ROOT_DIR/lib/libssl.a \
    -DCURL_LIBRARY=$LIBCURL_ROOT_DIR/libcurl.a -DCURL_INCLUDE_DIR=$LIBCURL_ROOT_DIR/include \
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=xcode_build \
    -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE \
    -DIOS_PLATFORM=$IOS_PLATFORM
