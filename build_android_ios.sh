#!/bin/sh

set -e

# get command line params
build_type=$1
sim=$2
static=$3

# get machine type (linix or mac generally)
unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac
echo $machine

if [ "$build_type" == "android" ]; then
    # do android stuff

    # set android sdk paths on mac
    if [ "$machine" == "Mac" ]; then
        export ANDROID_SDK=~/Library/Android/sdk
        export ANDROID_NDK=~/Library/Android/sdk/ndk-bundle
        export PATH="$PATH:$ANDROID_SDK/tools:$ANDROID_SDK/platform-tools:$ANDROID_NDK"
    fi

    echo :: build libzap.so *all ABIs*
    find . -name ".DS_Store" -delete
    if [ -n "$(ls -A builds/ndk_build 2>/dev/null)" ]; then
        rm -r builds/ndk_build/*
    fi
    (cd builds; ./build_android_abis.sh)
elif [ "$build_type" == "ios" ]; then
    # do ios stuff
    echo :: build libzap.a/dynlib *all ABIs*
    find . -name ".DS_Store" -delete
    if [ -n "$(ls -A builds/xcode_build 2>/dev/null)" ]; then
        rm -r builds/xcode_build/*
    fi
    (cd builds; ./cmake_lib_ios.sh $sim $static; cd ios; make;)

    if [ "$static" == "static" ]; then
        echo :: combine libs
        (cd builds; ./combine_ios_libs.sh $sim;)
    fi
else
    echo no build type specified! - 'android' or 'ios'?
fi
