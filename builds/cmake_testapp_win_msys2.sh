#!/bin/bash

set -e

## make sure pacman has all required packages ##
#pacman -S --needed base-devel mingw-w64-x86_64-toolchain \
#                    git unzip \
#                    mingw-w64-x86_64-cmake

wget -nc https://github.com/akheron/jansson/archive/v2.11.zip -O ../deps/jansson_v2.11.zip
unzip ../deps/jansson_v2.11.zip -d ../deps/

mkdir -p win

(cd win; cmake -G "MSYS Makefiles" ../../test)
