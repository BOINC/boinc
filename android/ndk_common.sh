#!/bin/sh

export NDK_VERSION=21d
export NDK_ARMV6_VERSION=15c
export NDK_ROOT=$BUILD_DIR/android-ndk-r${NDK_VERSION}
export NDK_ARMV6_ROOT=$BUILD_DIR/android-ndk-r${NDK_ARMV6_VERSION}

createNDKFolder()
{
    rm -rf "$BUILD_DIR/android-ndk-r${NDK_VERSION}"
    wget -c --no-verbose -O /tmp/ndk_${NDK_VERSION}.zip https://dl.google.com/android/repository/android-ndk-r${NDK_VERSION}-linux-x86_64.zip
    unzip -qq /tmp/ndk_${NDK_VERSION}.zip -d $BUILD_DIR
}

createNDKARMV6Folder()
{
    rm -rf "$BUILD_DIR/android-ndk-r${NDK_ARMV6_VERSION}"
    wget -c --no-verbose -O /tmp/ndk_armv6_${NDK_ARMV6_VERSION}.zip https://dl.google.com/android/repository/android-ndk-r${NDK_ARMV6_VERSION}-linux-x86_64.zip
    unzip -qq /tmp/ndk_armv6_${NDK_ARMV6_VERSION}.zip -d $BUILD_DIR
}
