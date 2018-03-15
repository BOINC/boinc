#/bin/sh -e

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export NDK_ROOT="${NDK_ROOT:-$HOME/NVPACK/android-ndk-r10e}"
export ANDROID_TC="${ANDROID_TC:-$HOME/android-tc}"
export ANDROID_TC_MIPS64="${ANDROID_TC_MIPS64:-$ANDROID_TC/mips64}"

# checks if a given path is canonical (absolute and does not contain relative links)
# from http://unix.stackexchange.com/a/256437
isPathCanonical() {
  case "x$1" in
    (x*/..|x*/../*|x../*|x*/.|x*/./*|x./*)
        rc=1
        ;;
    (x/*)
        rc=0
        ;;
    (*)
        rc=1
        ;;
  esac
  return $rc
}

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

if [ ! -d "$ANDROID_TC_MIPS64/mips64el-linux-android" ]; then
    "$NDK_ROOT/build/tools/make-standalone-toolchain.sh" --verbose --platform=android-21 --arch=mips64 --install-dir="$ANDROID_TC_MIPS64" --stl=libc++ "$@"
    if [ $? -ne 0 ]; then exit 1; fi
fi
