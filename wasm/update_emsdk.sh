#!/bin/sh
set -e

if [ ! -d "wasm" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

CACHE_DIR="$PWD/3rdParty/buildCache/wasm"
BUILD_DIR="$PWD/3rdParty/wasm"
EMSDK_ROOT="$BUILD_DIR/emsdk"


if [ ! -d $EMSDK_ROOT ]; then
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone https://github.com/emscripten-core/emsdk
fi

git -C $EMSDK_ROOT pull
$EMSDK_ROOT/emsdk install latest
$EMSDK_ROOT/emsdk activate latest
