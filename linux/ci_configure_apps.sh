#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

linux/update_vcpkg.sh

./configure --enable-apps --enable-vcpkg --disable-server --disable-client --disable-manager
