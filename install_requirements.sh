#!/bin/sh

set -e

apt install cmake
apt install libssl-dev
apt install libcurl4-openssl-dev
apt install google-android-ndk-installer

wget -nc https://github.com/akheron/jansson/archive/v2.11.zip -O jansson_v2.11.zip
unzip jansson_v2.11.zip -d deps/

