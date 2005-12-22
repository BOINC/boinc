#!/bin/sh

# Script to build Macintosh Universal Binary library of curl-7.15.1 for
# use in building BOINC.
#
# by Charlie Fenton 12/19/05
#
## cd [path]/curl-7.15.1/
## source buildcurl [ -clean ]
#
# the -clean argument will force a full rebuild.
#


if [ "$1" != "-clean" ]; then
  if [ -f lib/.libs/libcurl_ppc.a ] && [ -f lib/.libs/libcurl_i386.a ] && [ -f lib/.libs/libcurl.a ]; then
    
    echo "curl-7.15.1 already built"
    return 0
  fi
fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-3.3;export CXX=/usr/bin/g++-3.3
./configure --enable-shared=NO --host=ppc --build=ppc CPPFLAGS="-I/Developer/SDKs/MacOSX10.3.9.sdk/Developer/Headers/FlatCarbon -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include -isystem /Developer/SDKs/MacOSX10.3.9.sdk/usr/include/gcc/darwin/3.3 -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include/gcc/darwin/3.3/c++ -I/Developer/SDKs/MacOSX10.3.9.sdk/usr/include/gcc/darwin/3.3/c++/ppc-darwin -isystem /Developer/SDKs/MacOSX10.3.9.sdk/usr/include"
if [  $? -ne 0 ]; then exit 1; fi

make clean

rm -f lib/.libs/libcurl.a
rm -f lib/.libs/libcurl_ppc.a
rm -f lib/.libs/libcurl_i386.a

export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.3.9.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.3.9.sdk"

make
if [  $? -ne 0 ]; then exit 1; fi
mv -f lib/.libs/libcurl.a lib/libcurl_ppc.a

make clean
if [  $? -ne 0 ]; then exit 1; fi

export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS=""
export CPPFLAGS=""
./configure --enable-shared=NO --host=i386 --build=i386
if [  $? -ne 0 ]; then exit 1; fi

## export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"

make -e
if [  $? -ne 0 ]; then exit 1; fi
mv -f lib/.libs/libcurl.a lib/.libs/libcurl_i386.a
mv -f lib/libcurl_ppc.a lib/.libs/
lipo -create lib/.libs/libcurl_i386.a lib/.libs/libcurl_ppc.a -output lib/.libs/libcurl.a
if [  $? -ne 0 ]; then exit 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""

return 0
