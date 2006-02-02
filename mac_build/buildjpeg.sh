#!/bin/sh

# Script to build Macintosh Universal Binary library of jpeg-6b for
# use in building BOINC.
#
# by Charlie Fenton 12/19/05
#
## cd [path]/jpeg-6b/
## source buildcurl [ -clean ]
#
# the -clean argument will force a full rebuild.
#

if [ "$1" != "-clean" ]; then
  if [ -f libjpeg_ppc.a ] && [ -f libjpeg_i386.a ] && [ -f libjpeg.a ]; then
    echo "jpeg-6b already built"
    return 0
  fi
fi


## cd [path]/jpeg-6b/
export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-3.3;export CXX=/usr/bin/g++-3.3
./configure --disable-shared --host=ppc
if [  $? -ne 0 ]; then exit 1; fi

export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.3.9.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.3.9.sdk"
export CPPFLAGS="-I/Developer/SDKs/MacOSX10.3.9.sdk/Developer/Headers/FlatCarbon -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include -isystem /Developer/SDKs/MacOSX10.3.9.sdk/usr/include/gcc/darwin/3.3 -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include/gcc/darwin/3.3/c++ -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include/gcc/darwin/3.3/c++/ppc-darwin -isystem /Developer/SDKs/MacOSX10.3.9.sdk/usr/include"

rm -f libjpeg_ppc.a
rm -f libjpeg_i386.a
rm -f libjpeg.a
make clean

make
if [  $? -ne 0 ]; then exit 1; fi
mv -f libjpeg.a libjpeg_ppc.a

make clean
if [  $? -ne 0 ]; then exit 1; fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS=""
export CPPFLAGS=""
./configure --disable-shared --host=i386
if [  $? -ne 0 ]; then exit 1; fi

export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"

make -e
if [  $? -ne 0 ]; then exit 1; fi
mv libjpeg.a libjpeg_i386.a
lipo -create libjpeg_i386.a libjpeg_ppc.a -output libjpeg.a

if [  $? -ne 0 ]; then exit 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""

return 0
