#!/bin/bash

set -e

mkdir -p linux

(cd linux; cmake ../../test)
