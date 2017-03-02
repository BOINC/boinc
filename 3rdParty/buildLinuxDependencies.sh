#!/bin/bash
set -x

## support script to build a specific wxWidgets version for linux
## This script checks if a cached version is available and builts one if not.
## The build is done in 3rdParty/linux and usable files are installed to install/linux
## in order to keep the cache as small as possible
## The instal directory is cached by Travis-CI for all subsequent runs of the VM

# check working directory because the script needs to be called like: ./3rdParty/buildLinuxDependencies.sh
if [ ! -d "3rdParty" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

PREFIX=`pwd`/install/linux
FLAGFILE="${PREFIX}/wxWidgets-3.0.2_done"

if [ -e "${FLAGFILE}" ]; then
    echo "wxWidgets-3.0.2 seems already to be present in ./install/linux"
    exit 0
fi

mkdir -p 3rdParty/linux
mkdir -p install/linux
cd ./3rdParty/linux

if [ ! -d "wxWidgets-3.0.2" ]; then
    if [ ! -e "wxWidgets-3.0.2.tar.bz2" ]; then
        wget https://sourceforge.net/projects/wxwindows/files/3.0.2/wxWidgets-3.0.2.tar.bz2
    fi
    tar -xf wxWidgets-3.0.2.tar.bz2
fi
mkdir wxWidgets-3.0.2/buildgtk
if [  $? -ne 0 ]; then exit 1; fi
cd wxWidgets-3.0.2/buildgtk

../configure --prefix="${PREFIX}" --with-gtk --disable-shared --enable-webview --disable-gtktest --disable-sdltest --disable-debug
if [  $? -ne 0 ]; then exit 1; fi
make
if [  $? -ne 0 ]; then exit 1; fi
make install
if [  $? -ne 0 ]; then exit 1; fi
cd ../..
touch ${FLAGFILE}
# change back to root directory
cd ../..
