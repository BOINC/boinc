#!/bin/sh

#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

IXWEBSOCKET_VERSION="11.4.6"
BUILD_DIR="$PWD/3rdParty/linux"
IXWEBSOCKET_ROOT="$BUILD_DIR/IXWebSocket"
LINK="https://github.com/machinezone/IXWebSocket -b v$IXWEBSOCKET_VERSION"

if [ ! -d $IXWEBSOCKET_ROOT ]; then
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone $LINK
fi

mkdir $IXWEBSOCKET_ROOT/build
cd $IXWEBSOCKET_ROOT/build
cmake .. -DUSE_TLS=ON
make

if [ "$1" = "no_sudo" ]; then
    make install
else
    sudo make install
fi
