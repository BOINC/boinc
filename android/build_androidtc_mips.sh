#/bin/sh -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

if [ "x$TRAVIS_BUILD_DIR" != "x" ]; then
    export BUILD_DIR="$TRAVIS_BUILD_DIR"
else
    export BUILD_DIR="$HOME"
fi

export NDK_ROOT="${NDK_ROOT:-$BUILD_DIR/NVPACK/android-ndk-r10e}"
export ANDROID_TC="${ANDROID_TC:-$BUILD_DIR/android-tc}"
export ANDROID_TC_MIPS="${ANDROID_TC_MIPS:-$ANDROID_TC/mips}"

cache_dir=""
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --cache_dir)
        cache_dir="$2"
        shift
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

if [ ! -d "$ANDROID_TC_MIPS/mipsel-linux-android" ]; then
    "$NDK_ROOT/build/tools/make-standalone-toolchain.sh" --verbose --platform=android-16 --arch=mips --install-dir="$ANDROID_TC_MIPS" --stl=libc++ "$@"
    if [ $? -ne 0 ]; then exit 1; fi
fi
