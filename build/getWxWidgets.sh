#!/bin/bash
set -x

## Travis-CI support script to built a specific wxWidgets version
## This script checks if a cached version is available and builts one if not.
## The directory is cached by Travis-CI for all subsequent runs of the VM

# change to correct working directory because the script is called like: ./build/getWxWidgets.sh
cd ./build

# if the versioning changes try to delete the old directory and change .travis.yml to cache the new directory
if [ ! -d "wxWidgets-3.0.2" ]; then
  wget https://sourceforge.net/projects/wxwindows/files/3.0.2/wxWidgets-3.0.2.tar.bz2
  tar -xf wxWidgets-3.0.2.tar.bz2
  cd wxWidgets-3.0.2
  mkdir buildgtk
  cd buildgtk
  ../configure --with-gtk --disable-shared --enable-webview --disable-gtktest --disable-sdltest
  make
  cd ../..
fi

# change back to root directory
cd ..
