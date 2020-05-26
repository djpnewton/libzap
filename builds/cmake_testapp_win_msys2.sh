#!/bin/bash

set -e

mkdir -p win

(cd win; cmake -G "MSYS Makefiles" ../../test)
