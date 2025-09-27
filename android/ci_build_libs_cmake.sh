#!/bin/sh

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

set -e

PLATFORM_NAME="android"

if [ ! -d "$PLATFORM_NAME" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

BUILD_DIR="$PWD/3rdParty/$PLATFORM_NAME"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
VCPKG_PORTS="$PWD/3rdParty/vcpkg_ports"
. $PLATFORM_NAME/ndk_common.sh

if [ ! -d $NDK_ROOT ]; then
    createNDKFolder
fi

if [ ! -d $NDK_ARMV6_ROOT ]; then
    createNDKARMV6Folder
fi

ORIG_PATH=$PATH

$PLATFORM_NAME/bootstrap_vcpkg_cmake.sh
./_autosetup

TRIPLETS_LIST="armv6-android arm-android arm-neon-android arm64-android x86-android x64-android"

for TRIPLET in $TRIPLETS_LIST ; do
    echo "\e[0;35m building $TRIPLET ... \e[0m"

    if [ "$TRIPLET" = "armv6-android" ]; then
        export ANDROID_NDK_HOME=$NDK_ARMV6_ROOT
        "$ANDROID_NDK_HOME/build/tools/make-standalone-toolchain.sh" --verbose --toolchain=arm-linux-androideabi-clang --platform=android-16 --arch=arm --stl=libc++ --install-dir="${ANDROID_NDK_HOME}/armv6" "$@"
    else
        export ANDROID_NDK_HOME=$NDK_ROOT
    fi
    export PKG_CONFIG_PATH=$VCPKG_ROOT/installed/$TRIPLET/lib/pkgconfig/
    if [ $TRIPLET = "armv6-android" ]; then
        toolchain_root="${ANDROID_NDK_HOME}/armv6"
        tc_binaries="${toolchain_root}/bin"
        tc_includes="${ANDROID_NDK_HOME}/arm-linux-androideabi"
        export PATH="${tc_includes}:${tc_binaries}:${ORIG_PATH}"
        sysroot="${toolchain_root}/sysroot/"
        includes="${ANDROID_NDK_HOME}/arm-linux-androideabi/"
        CC="arm-linux-androideabi-clang"
        CXX="arm-linux-androideabi-clang++"
        LD="arm-linux-androideabi-clang"
        CFLAGS="--sysroot=${sysroot} -DANDROID -I${includes}/include -O3 -fomit-frame-pointer -fPIE -march=armv6 -mfloat-abi=softfp -mfpu=vfp -DANDROID_ABI=armeabi-v6 -D__ANDROID_API__=16"
        CXXFLAGS="${CFLAGS} -std=c++11"
        LDFLAGS="-L${includes}/usr/lib -L${includes}/lib -fPIE -pie -march=armv6 -static-libstdc++ -llog"
        HOST="armv6-linux"
    elif [ $TRIPLET = "arm-android" ]; then
        toolchain_root="${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64"
        tc_binaries="${toolchain_root}/bin"
        tc_includes="${ANDROID_NDK_HOME}/arm-linux-androideabi"
        export PATH="${tc_includes}:${tc_binaries}:${ORIG_PATH}"
        sysroot="${toolchain_root}/sysroot/"
        includes="${ANDROID_NDK_HOME}/arm-linux-androideabi/"
        CC="armv7a-linux-androideabi28-clang"
        CXX="armv7a-linux-androideabi28-clang++"
        LD="armv7a-linux-androideabi28-ld"
        CFLAGS="--sysroot=${sysroot} -DANDROID -I${includes}/include -O3 -fomit-frame-pointer -fPIE -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -DANDROID_ABI=armeabi-v7a -D__ANDROID_API__=16"
        CXXFLAGS="${CFLAGS} -std=c++11"
        LDFLAGS="-L${includes}/usr/lib -L${includes}/lib -fPIE -pie -march=armv7-a -static-libstdc++ -llog -latomic -Wl,--fix-cortex-a8"
        HOST="arm-linux"
    elif [ $TRIPLET = "arm-neon-android" ]; then
        toolchain_root="${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64"
        tc_binaries="${toolchain_root}/bin"
        tc_includes="${ANDROID_NDK_HOME}/arm-linux-androideabi"
        export PATH="${tc_includes}:${tc_binaries}:${ORIG_PATH}"
        sysroot="${toolchain_root}/sysroot/"
        includes="${ANDROID_NDK_HOME}/arm-linux-androideabi/"
        CC="armv7a-linux-androideabi28-clang"
        CXX="armv7a-linux-androideabi28-clang++"
        LD="armv7a-linux-androideabi28-ld"
        CFLAGS="--sysroot=${sysroot} -DANDROID -I${includes}/include -O3 -fomit-frame-pointer -fPIE -march=armv7-a -mfloat-abi=softfp -mfpu=neon-vfpv3 -DANDROID_ABI=armeabi-v7a -D__ANDROID_API__=16"
        CXXFLAGS="${CFLAGS} -std=c++11"
        LDFLAGS="-L${includes}/usr/lib -L${includes}/lib -fPIE -pie -march=armv7-a -static-libstdc++ -llog -latomic -Wl,--fix-cortex-a8"
        HOST="arm-linux"
    elif [ $TRIPLET = "arm64-android" ]; then
        toolchain_root="${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64"
        tc_binaries="${toolchain_root}/bin"
        tc_includes="${ANDROID_NDK_HOME}/aarch64-linux-android"
        export PATH="${tc_includes}:${tc_binaries}:${ORIG_PATH}"
        sysroot="${toolchain_root}/sysroot/"
        includes="${ANDROID_NDK_HOME}/aarch64-linux-android/"
        CC="aarch64-linux-android28-clang"
        CXX="aarch64-linux-android28-clang++"
        LD="aarch64-linux-android28-ld"
        CFLAGS="--sysroot=${sysroot} -DANDROID -I${includes}/include -O3 -fomit-frame-pointer -fPIE -DANDROID_ABI=arm64-v8a -D__ANDROID_API__=21"
        CXXFLAGS="${CFLAGS} -std=c++11"
        LDFLAGS="-L${includes}/usr/lib -L${includes}/lib -fPIE -pie -static-libstdc++ -llog -latomic"
        HOST="aarch64-linux"
    elif [ $TRIPLET = "x64-android" ]; then
        toolchain_root="${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64"
        tc_binaries="${toolchain_root}/bin"
        tc_includes="${ANDROID_NDK_HOME}/x86_64-linux-android"
        export PATH="${tc_includes}:${tc_binaries}:${ORIG_PATH}"
        sysroot="${toolchain_root}/sysroot/"
        includes="${ANDROID_NDK_HOME}/x86_64-linux-android/"
        CC="x86_64-linux-android28-clang"
        CXX="x86_64-linux-android28-clang++"
        LD="x86_64-linux-android28-ld"
        CFLAGS="--sysroot=${sysroot} -DANDROID -I${includes}/include -O3 -fomit-frame-pointer -fPIE -DANDROID_ABI=x86_64 -D__ANDROID_API__=21"
        CXXFLAGS="${CFLAGS} -std=c++11"
        LDFLAGS="-L${includes}/usr/lib -L${includes}/lib -fPIE -pie -static-libstdc++ -llog -latomic"
        HOST="x86_64-linux"
    elif [ $TRIPLET = "x86-android" ]; then
        toolchain_root="${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64"
        tc_binaries="${toolchain_root}/bin"
        tc_includes="${ANDROID_NDK_HOME}/i686-linux-android"
        export PATH="${tc_includes}:${tc_binaries}:${ORIG_PATH}"
        sysroot="${toolchain_root}/sysroot/"
        includes="${ANDROID_NDK_HOME}/i686-linux-android/"
        CC="i686-linux-android28-clang"
        CXX="i686-linux-android28-clang++"
        LD="i686-linux-android28-ld"
        CFLAGS="--sysroot=${sysroot} -DANDROID -I${includes}/include -O3 -fomit-frame-pointer -fPIE -DANDROID_ABI=x86 -D__ANDROID_API__=16"
        CXXFLAGS="${CFLAGS} -std=c++11"
        LDFLAGS="-L${includes}/usr/lib -L${includes}/lib -fPIE -pie -static-libstdc++ -llog -latomic"
        HOST="i686-linux"
    fi

    BUILD_TRIPLET=build-$TRIPLET
    cmake -E env CC="${CC}" CXX="${CXX}" LD="${LD}" CFLAGS="${CFLAGS}" CXXFLAGS="${CXXFLAGS}" LDFLAGS="${LDFLAGS}" cmake lib -B $BUILD_TRIPLET -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVCPKG_MANIFEST_DIR=3rdParty/vcpkg_ports/configs/libs/ -DVCPKG_INSTALLED_DIR=$VCPKG_ROOT/installed/ -DVCPKG_OVERLAY_PORTS=$VCPKG_PORTS/ports -DVCPKG_OVERLAY_TRIPLETS=$VCPKG_PORTS/triplets/ci -DVCPKG_TARGET_TRIPLET=$TRIPLET -DVCPKG_INSTALL_OPTIONS=--clean-after-build -DHOST=$HOST
    cmake --build $BUILD_TRIPLET

    echo "\e[1;32m $TRIPLET done \e[0m"
done

export PATH=$ORIG_PATH
