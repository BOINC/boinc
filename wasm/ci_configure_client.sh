#!/bin/bash
set -e

if [ ! -d "wasm" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

CACHE_DIR="$PWD/3rdParty/buildCache/wasm"
BUILD_DIR="$PWD/3rdParty/wasm"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
EMSDK_ROOT="$BUILD_DIR/emsdk"
export VCPKG_DIR="$VCPKG_ROOT/installed/wasm32-emscripten"

wasm/update_emsdk.sh
source $EMSDK_ROOT/emsdk_env.sh
wasm/update_emsdk_vcpkg.sh

debug_flags=""

if [ "debug" == "$1" ]; then
    export CPPFLAGS="-g3 -fdebug-prefix-map=$PWD=.."
    debug_flags="--enable-debug"
    echo -e "\e[33mDebug flags: ./configure $debug_flags, CPPFLAGS=$CPPFLAGS\e[0m"
fi

export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
emconfigure ./configure $debug_flags --enable-wasm --enable-vcpkg --with-libcurl=$VCPKG_DIR --with-ssl=$VCPKG_DIR --disable-server --enable-client --disable-manager
