#!/bin/bash
set -x

wget https://sourceforge.net/projects/wxwindows/files/3.0.2/wxWidgets-3.0.2.tar.bz2
tar -xf wxWidgets-3.0.2.tar.bz2
cd wxWidgets-3.0.2
mkdir buildgtk
cd buildgtk
../configure --with-gtk --disable-shared --enable-webview --disable-gtktest --disable-sdltest
make
