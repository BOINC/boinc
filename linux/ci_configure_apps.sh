#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

CACHE_DIR="$PWD/3rdParty/buildCache"
export XDG_CACHE_HOME=$CACHE_DIR/.cache

if [ ! -d "$CACHE_DIR/vcpkg" ]; then
    mkdir -p $CACHE_DIR
    git -C $CACHE_DIR clone https://github.com/microsoft/vcpkg
fi

git -C $CACHE_DIR/vcpkg pull
$CACHE_DIR/vcpkg/bootstrap-vcpkg.sh
$CACHE_DIR/vcpkg/vcpkg install rappture
$CACHE_DIR/vcpkg/vcpkg upgrade --no-dry-run

./configure --enable-apps --enable-vcpkg --disable-server --disable-client --disable-manager
