#!/bin/sh

set -e

ABIS="x86 x86_64 armeabi-v7a arm64-v8a"
for abi in $ABIS
do
    rm -r ./android
    export ANDROID_ABI=$abi
    ./cmake_lib_android.sh
    (cd android/; make)
done
