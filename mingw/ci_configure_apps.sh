#!/bin/sh
set -e

if [ ! -d "mingw" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

CACHE_DIR="$PWD/3rdParty/buildCache/mingw"
BUILD_DIR="$PWD/3rdParty/mingw"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
export VCPKG_DIR="$VCPKG_ROOT/installed/x64-mingw-static"

mingw/update_vcpkg.sh

export ZIP_VCPROJ_FLAGS="-DWIN32 -D_LIB -DDLL -D_CRT_SECURE_NO_WARNINGS -DNO_MKTEMP -DUSE_ZIPMAIN -DNO_CRYPT -DIZ_PWLEN=80 -DNO_ASM -DNO_UNICODE_SUPPORT -DNO_MBCS"
export ZIP_BOINC_RENAMES="-Dinflate=inflate_boinc -Ddeflate=deflate_boinc -Dget_crc_table=get_crc_table_boinc -Dlongest_match=longest_match_boinc -Dinflate_codes=inflate_codes_boinc"
export ZIP_MINGW="-DUSE_MINGW_GLOBBING -DUSE_STRM_INPUT"
export CXXFLAGS="$ZIP_VCPROJ_FLAGS $ZIP_BOINC_RENAMES $ZIP_MINGW -I$VCPKG_DIR/include -L$VCPKG_DIR/lib"
export CFLAGS="$CXXFLAGS"

export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
./configure --host=x86_64-w64-mingw32 --with-libcurl=$VCPKG_DIR --with-ssl=$VCPKG_DIR --enable-apps --enable-apps-mingw --enable-apps-vcpkg --enable-apps-gui --disable-server --disable-client --disable-manager
