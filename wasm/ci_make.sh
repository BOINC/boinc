#!/bin/bash
set -e

if [ ! -d "wasm" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/wasm"
EMSDK_ROOT="$BUILD_DIR/emsdk"

source $EMSDK_ROOT/emsdk_env.sh
emmake make
