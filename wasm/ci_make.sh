#!/bin/bash
set -e

if [ ! -d "wasm" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/wasm"
EMSDK_ROOT="$BUILD_DIR/emsdk"

source $EMSDK_ROOT/emsdk_env.sh
if [ "debug" == "$1" ]; then
    MAKE_FLAGS="AM_DEFAULT_VERBOSITY=1"
fi
emmake make $MAKE_FLAGS
