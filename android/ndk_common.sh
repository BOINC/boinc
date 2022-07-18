#!/bin/sh

export NDK_VERSION=22b
export NDK_ARMV6_VERSION=15c
export NDK_ROOT=$BUILD_DIR/android-ndk-r${NDK_VERSION}
export NDK_ARMV6_ROOT=$BUILD_DIR/android-ndk-r${NDK_ARMV6_VERSION}

createNDKFolder()
{
    ndk_filename=android-ndk-r${NDK_VERSION}-linux-x86_64.zip
    rm -rf "$BUILD_DIR/android-ndk-r${NDK_VERSION}"
    echo Downloading $ndk_filename
    wget -c --no-verbose -O /tmp/$ndk_filename https://dl.google.com/android/repository/$ndk_filename
    echo Extracting $ndk_filename
    unzip -qq /tmp/$ndk_filename -d $BUILD_DIR
    echo Finished extracting of $ndk_filename
}

createNDKARMV6Folder()
{
    ndk_filename="android-ndk-r${NDK_ARMV6_VERSION}-linux-x86_64.zip"
    rm -rf "$BUILD_DIR/android-ndk-r${NDK_ARMV6_VERSION}"
    echo Downloading $ndk_filename
    wget -c --no-verbose -O /tmp/$ndk_filename https://dl.google.com/android/repository/$ndk_filename
    echo Extracting $ndk_filename
    unzip -qq /tmp/$ndk_filename -d $BUILD_DIR
    echo Finished extracting of $ndk_filename
}
