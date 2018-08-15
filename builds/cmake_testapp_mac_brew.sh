#!/bin/bash

set -e

mkdir -p mac

(cd mac; cmake -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl ../../test)
