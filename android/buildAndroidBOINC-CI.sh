#!/bin/sh
set -e

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

# When you want to invalidate openssl and curl without change their versions.

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

doclean=""
cache_dir=""
arch=""
component=""
silent=""
verbose="${VERBOSE:-no}"
ci=""

while [ $# -gt 0 ]; do
    key="$1"
    case $key in
        --cache_dir)
        cache_dir="$2"
        shift
        ;;
        --build_dir)
        build_dir="$2"
        shift
        ;;
        --clean)
        doclean="yes"
        ;;
        --arch)
        arch="$2"
        shift
        ;;
        --component)
        component="$2"
        shift
        ;;
        --silent)
        silent="yes"
        ;;
        --verbose)
        verbose="yes"
        ;;
        --ci)
        ci="yes"
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

cd ../
vcpkg_ports_dir="$(pwd)/3rdParty/vcpkg_ports"
cd android/

if [ "x$cache_dir" != "x" ]; then
    if  ! ( isPathCanonical "$cache_dir" && [ "$cache_dir" != "/" ] ); then
        echo "cache_dir must be an absolute path without ./ or ../ in it"
        exit 1
    fi
else
    cd ../
    cache_dir="$(pwd)/3rdParty/buildCache/android"
    cd android/
fi

PREFIX="$cache_dir/android-tc"

if [ "x$build_dir" != "x" ]; then
    if isPathCanonical "$build_dir" && [ "$build_dir" != "/" ]; then
         BUILD_DIR="$build_dir"
     else
         echo "build_dir must be an absolute path without ./ or ../ in it"
         exit 1
     fi
else
    cd ../
    BUILD_DIR="$(pwd)/3rdParty/android"
    THIRD_PARTY="$(pwd)/3rdParty"
    cd android/
fi

mkdir -p "${PREFIX}"
mkdir -p "${BUILD_DIR}"

if [ "${doclean}" = "yes" ]; then
    echo "cleaning cache dir"
    rm -rf "${cache_dir}"
    mkdir -p "${cache_dir}"
    echo "cleaning build dir"
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
    echo "cleaning downloaded cache files"
    rm -f /tmp/ndk_${NDK_VERSION}.zip
    rm -f /tmp/ndk_armv6_${NDK_ARMV6_VERSION}.zip
fi

if [ "${silent}" = "yes" ]; then
    export STDOUT_TARGET="/dev/null"
fi

. $(pwd)/ndk_common.sh
. $(pwd)/../3rdParty/vcpkg_ports/vcpkg_link.sh
export NDK_FLAGFILE="$PREFIX/NDK-${NDK_VERSION}_done"
export NDK_ARMV6_FLAGFILE="$PREFIX/NDK-${NDK_ARMV6_VERSION}-armv6_done"
export VCPKG_ROOT="$BUILD_DIR/vcpkg"
export ANDROID_TC=$PREFIX
export VERBOSE=$verbose
export CI=$ci

if [ "$arch" = armv6 ]; then
    export ANDROID_TC_FLAGFILE="$PREFIX/ANDROID_TC_WITH_NDK-${NDK_ARMV6_VERSION}-${arch}_done"
else
    export ANDROID_TC_FLAGFILE="$PREFIX/ANDROID_TC_WITH_NDK-${NDK_VERSION}-${arch}_done"
fi

if [ ! -e "${NDK_FLAGFILE}" ]; then
    createNDKFolder
    touch "${NDK_FLAGFILE}"
fi

if [ ! -e "${NDK_ARMV6_FLAGFILE}" ]; then
    createNDKARMV6Folder
    touch "${NDK_ARMV6_FLAGFILE}"
fi

if [ ! -d $NDK_ROOT ]; then
    createNDKFolder
fi

if [ ! -d $NDK_ARMV6_ROOT ]; then
    createNDKARMV6Folder
fi

if [ ! -e "${ANDROID_TC_FLAGFILE}" ]; then
    rm -rf "${PREFIX}/${arch}"
    echo delete "${PREFIX}/${arch}"
    touch ${ANDROID_TC_FLAGFILE}
fi

if [ $arch != "armv6" -a ! -d ${PREFIX}/${arch} ]; then
    mkdir -p ${PREFIX}/${arch}
fi

getTripletName()
{
    arch=$1
    triplet_name=""
    if [ $arch = "armv6" ]; then
        triplet_name="armv6-android"
    fi
    if [ $arch = "arm" ]; then
        triplet_name="arm-android"
    fi
    if [ $arch = "neon" ]; then
        triplet_name="arm-neon-android"
    fi
    if [ $arch = "arm64" ]; then
        triplet_name="arm64-android"
    fi
    if [ $arch = "x86" ]; then
        triplet_name="x86-android"
    fi
    if [ $arch = "x86_64" ]; then
        triplet_name="x64-android"
    fi

    echo $triplet_name
}

if [ $ci = "yes" ]; then
    triplets_setup="ci"
else
    triplets_setup="default"
fi
manifest_dir=$THIRD_PARTY/vcpkg_ports/configs/$component
if [ $component = "apps" ]; then
    manifest_dir=$manifest_dir/android
fi
manifests="--x-manifest-root=$manifest_dir --x-install-root=$VCPKG_ROOT/installed/"
vcpkg_overlay="--overlay-ports=$vcpkg_ports_dir/ports --overlay-triplets=$vcpkg_ports_dir/triplets/$triplets_setup"
vcpkg_flags="$vcpkg_overlay  --feature-flags=versions --clean-after-build"
if [ ! -d "$VCPKG_ROOT" ]; then
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone $VCPKG_LINK
fi
if [ ! -e /tmp/vcpkg_updated ]; then
    git -C $VCPKG_ROOT pull
    $VCPKG_ROOT/bootstrap-vcpkg.sh
    touch /tmp/vcpkg_updated
fi
if [ $arch = "armv6" ]; then
    export ANDROID_NDK_HOME=$NDK_ARMV6_ROOT

    $VCPKG_ROOT/vcpkg install $manifests $vcpkg_flags --triplet=$(getTripletName $arch)
elif [ $arch = "arm" ]; then
    export ANDROID_NDK_HOME=$NDK_ROOT

    $VCPKG_ROOT/vcpkg install $manifests $vcpkg_flags --triplet=$(getTripletName neon)
    $VCPKG_ROOT/vcpkg install $manifests $vcpkg_flags --triplet=$(getTripletName $arch)
else
    export ANDROID_NDK_HOME=$NDK_ROOT

    $VCPKG_ROOT/vcpkg install $manifests $vcpkg_flags --triplet=$(getTripletName $arch)
fi

vcpkgDir()
{
    arch=$1
    shift
    vcpkg_dir="$VCPKG_ROOT/installed"

    if [ $arch = "armv6" ]; then
        vcpkg_dir="$vcpkg_dir/armv6-android"
    elif [ $arch = "arm" ]; then
        vcpkg_dir="$vcpkg_dir/arm-android"
    elif [ $arch = "arm64" ]; then
        vcpkg_dir="$vcpkg_dir/arm64-android"
    elif [ $arch = "x86" ]; then
        vcpkg_dir="$vcpkg_dir/x86-android"
    elif [ $arch = "x86_64" ]; then
        vcpkg_dir="$vcpkg_dir/x64-android"
    fi

    echo $vcpkg_dir
}

list_apps_name="boinc_gahp uc2 ucn multi_thread sleeper sporadic worker wrapper wrappture_example fermi"

NeonTest()
{
    while [ $# -gt 0 ]; do
        echo "NeonTest: readelf -A" "$1"
        vcpkg_dir_search=$(vcpkgDir $arch)
        if [ $(readelf -A $(find $ANDROID_TC/${arch} "../samples" $vcpkg_dir_search -type f -name "$1") | grep -i neon | head -c1 | wc -c) -ne 0 ]; then
            echo $(readelf -A $(find $ANDROID_TC/${arch} "../samples" $vcpkg_dir_search -type f -name "$1") | grep -i neon)
            echo [ERROR] "$1" contains neon optimization
            exit 1
        fi
        shift
    done
}

NeonTestClient()
{
    NeonTest libcrypto.a libssl.a libcurl.a
}

NeonTestLibs()
{
    NeonTest libboinc.a libboinc_api.a libboinc_opencl.a libboinc_zip.a
}

Armv6Test()
{
    while [ $# -gt 0 ]; do
        echo "Armv6Test: readelf -A" "$1"
        vcpkg_dir_search=$(vcpkgDir $arch)
        if [ $(readelf -A $(find $ANDROID_TC/armv6 "BOINC/app/src/main/assets/armeabi" "../samples" $vcpkg_dir_search -type f -name "$1") | grep -i -E "Tag_CPU_arch: (v6|v5TE)" | head -c1 | wc -c) -eq 0 ]; then
            echo $(readelf -A $(find $ANDROID_TC/armv6 "BOINC/app/src/main/assets/armeabi" "../samples" $vcpkg_dir_search -type f -name "$1") | grep -i "Tag_CPU_arch:")
            echo [ERROR] "$1" is not armv6 cpu arch
            exit 1
        fi
        shift
    done
}

Armv6TestClient()
{
    Armv6Test libcrypto.a libssl.a libcurl.a boinc
}

Armv6TestLibs()
{
    Armv6Test libboinc.a libboinc_api.a libboinc_opencl.a libboinc_zip.a
}

Armv6TestApps()
{
    Armv6Test $list_apps_name
}

NeonTestApps()
{
    NeonTest  $list_apps_name
}

RenameAllApps()
{
    list_apps="../samples/condor/ boinc_gahp
                ../samples/example_app/ uc2
                ../samples/example_app/ ucn
                ../samples/multi_thread/ multi_thread
                ../samples/sleeper/ sleeper
                ../samples/sporadic/ sporadic
                ../samples/worker/ worker
                ../samples/wrapper/ wrapper
                ../samples/wrappture/ wrappture_example
                ../samples/wrappture/ fermi
                "

    RenameApp $1 $list_apps
}

RenameApp()
{
    arch=$1
    shift
    while [ $# -gt 0 ]; do
        mv "$1$2" "$1android_${arch}_$2"
        shift
        shift
    done
}

case "$arch" in
    "armv6")
        ./build_androidtc_armv6.sh
        case "$component" in
            "client")
                ./build_boinc_armv6.sh
                NeonTestClient
                Armv6TestClient
                exit 0
            ;;
            "libs")
                ./build_libraries_armv6.sh
                NeonTestLibs
                Armv6TestLibs
                exit 0
            ;;
            "apps")
                ./build_libraries_armv6.sh
                ./build_example_armv6.sh
                NeonTestLibs
                NeonTestApps
                Armv6TestLibs
                Armv6TestApps
                if [ "$ci" = "yes" ]; then
                    RenameAllApps armv6
                fi
                exit 0
            ;;
            *)
                echo "unknown component: $component"
                exit 1
            ;;
        esac
    ;;
    "arm")
        case "$component" in
            "client")
                ./build_boinc_arm.sh
                NeonTestClient
                exit 0
            ;;
            "libs")
                ./build_libraries_arm.sh
                NeonTestLibs
                exit 0
            ;;
            "apps")
                ./build_libraries_arm.sh
                ./build_example_arm.sh
                NeonTestLibs
                NeonTestApps
                if [ "$ci" = "yes" ]; then
                    RenameAllApps arm
                fi
                exit 0
            ;;
            *)
                echo "unknown component: $component"
                exit 1
            ;;
        esac
    ;;
    "arm64")
        case "$component" in
            "client")
                ./build_boinc_arm64.sh
                exit 0
            ;;
            "libs")
                ./build_libraries_arm64.sh
                exit 0
            ;;
            "apps")
                ./build_libraries_arm64.sh
                ./build_example_arm64.sh
                if [ "$ci" = "yes" ]; then
                    RenameAllApps arm64
                fi
                exit 0
            ;;
            *)
                echo "unknown component: $component"
                exit 1
            ;;
        esac
    ;;
    "x86")
        case "$component" in
            "client")
                ./build_boinc_x86.sh
                exit 0
            ;;
            "libs")
                ./build_libraries_x86.sh
                exit 0
            ;;
            "apps")
                ./build_libraries_x86.sh
                ./build_example_x86.sh
                if [ "$ci" = "yes" ]; then
                    RenameAllApps x86
                fi
                exit 0
            ;;
            *)
                echo "unknown component: $component"
                exit 1
            ;;
        esac
    ;;
    "x86_64")
        case "$component" in
            "client")
                ./build_boinc_x86_64.sh
                exit 0
            ;;
            "libs")
                ./build_libraries_x86_64.sh
                exit 0
            ;;
            "apps")
                ./build_libraries_x86_64.sh
                ./build_example_x86_64.sh
                if [ "$ci" = "yes" ]; then
                    RenameAllApps x86_64
                fi
                exit 0
            ;;
            *)
                echo "unknown component: $component"
                exit 1
            ;;
        esac
    ;;
esac

echo "unknown architecture: $arch"
exit 1
