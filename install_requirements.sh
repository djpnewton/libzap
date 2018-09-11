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

if [ "$machine" == "Mac" ]; then
    brew install cmake
    brew install wget
elif [ "$machine" == "MinGw" ]; then
    pacman -S --needed base-devel mingw-w64-x86_64-toolchain \
                    git unzip \
                    mingw-w64-x86_64-cmake
else
    apt install cmake
    apt install libssl-dev
    apt install libcurl4-openssl-dev
    apt install google-android-ndk-installer
fi

wget -nc https://github.com/akheron/jansson/archive/v2.11.zip -O deps/jansson_v2.11.zip
unzip deps/jansson_v2.11.zip -d deps/

