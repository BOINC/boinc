#!/bin/bash

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2023 University of California
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
#
# Script to build Macintosh 64-bit Intel openssl libraries
# libcrypto.a and libssl.a for use in building BOINC.
#
# by Charlie Fenton 6/25/12
# Updated 7/10/12 for Xcode 4.3 and later which are not at a fixed address
# Updated 7/30/13 for openssl-1.0.1e
# Updated 2/12/14 for openssl-1.0.1f
# Updated 4/14/14 for openssl-1.0.1g
# Updated 6/6/14 for openssl-1.0.1h
# Updated 9/2/14 for bulding openssl as 64-bit binary
# Updated 6/6/14 for openssl-1.0.1j
# Updated 12/11/15 for openssl-1.0.2e
# Updated 3/2/16 for openssl-1.0.2g
# Updated 9/10/16 for openssl-1.1.0
# Updated 1/25/18 for bulding openssl 1.1.0g (updated comemnts only)
# Updated 1/23/19 use libc++ instead of libstdc++ for Xcode 10 compatibility
# Updated 10/20/20 To build Apple Silicon / arm64 and x86_64 Universal binary
# Updated 12/24/20 for openssl-1.1.0l
# Updated 5/18/21 for compatibility with zsh
# Updated 10/18/21 for building OpenSSL 3.0.0
# Updated 2/6/23 changed MAC_OS_X_VERSION_MAX_ALLOWED to 101300 and MAC_OS_X_VERSION_MIN_REQUIRED to 101300 and MACOSX_DEPLOYMENT_TARGET to 10.13
# Updated 4/5/23 for args now accepted by patch utility; set mmacosx-version-min=10.13
#
## Building OpenSSL 3.0 requires Xcode 10.2 or later
#
## After first installing Xcode, you must have opened Xcode and
## clicked the Install button on the dialog which appears to
## complete the Xcode installation before running this script.
#
## Where x.xx.xy is the openssl version number:
## In Terminal, CD to the openssl-x.xx.xy directory.
##     cd [path]/openssl-x.xx.xy/
## then run this script:
##     source [path]/buildopenssl.sh [ -clean ] [--prefix PATH]
##
## the -clean argument will force a full rebuild.
## if --prefix is given as absolute path the library is installed into there
## use -q or --quiet to redirect build output to /dev/null instead of /dev/stdout
##

doclean=""
stdout_target="/dev/stdout"
lprefix=""
libPath="."
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="yes"
        ;;
        -prefix|--prefix)
        lprefix="$2"
        libPath="${lprefix}/lib"
        shift
        ;;
        -q|--quiet)
        stdout_target="/dev/null"
        ;;
    esac
    shift # past argument or value
done

export PATH=/usr/local/bin:$PATH

GCCPATH=`xcrun -find gcc`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find gcc compiler"
    return 1
fi

GCC_can_build_x86_64="no"
GCC_can_build_arm64="no"
GCC_archs=`lipo -info "${GCCPATH}"`
if [[ "${GCC_archs}" = *"x86_64"* ]]; then GCC_can_build_x86_64="yes"; fi
if [[ "${GCC_archs}" = *"arm64"* ]]; then GCC_can_build_arm64="yes"; fi

if [ "${doclean}" != "yes" ]; then
    if [ -f ${libPath}/libssl.a ] && [ -f ${libPath}/libcrypto.a ]; then
        alreadyBuilt=1

        if [ $GCC_can_build_x86_64 = "yes" ]; then
            lipo "${libPath}/libssl.a" -verify_arch x86_64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
            lipo "${libPath}/libcrypto.a" -verify_arch x86_64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
        fi

        if [ $alreadyBuilt -eq 1 ] && [ $GCC_can_build_arm64 = "yes" ]; then
            lipo "${libPath}/libssl.a" -verify_arch arm64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
            lipo "${libPath}/libcrypto.a" -verify_arch arm64
            if [ $? -ne 0 ]; then alreadyBuilt=0; doclean="yes"; fi
        fi

        if [ $alreadyBuilt -eq 1 ]; then
            cwd=$(pwd)
            dirname=${cwd##*/}
            echo "${dirname} already built"
            return 0
        fi
    fi
fi
echo ""

GPPPATH=`xcrun -find g++`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find g++ compiler"
    return 1
fi

MAKEPATH=`xcrun -find make`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find make tool"
    return 1
fi

TOOLSPATH1=${MAKEPATH%/make}

ARPATH=`xcrun -find ar`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find ar tool"
    return 1
fi

TOOLSPATH2=${ARPATH%/ar}

SDKPATH=`xcodebuild -version -sdk macosx Path`

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":/usr/local/bin:$PATH

if [ -d "${libPath}" ]; then
    rm -f ${libPath}/libssl.a
    rm -f ${libPath}/libcrypto.a
fi

echo ""

# Build for x86_64 architecture

## avx-512 cpu extensions are first supported in Xcode 10.2, but there is a bug in
## crypto/bn/asm/rsaz-avx512.pl which causes OpenSSL to try to build with avx-512
## instructions on earlier versions of Xcode, causing many build errors. In those
## cases, we patch rsaz-avx512.pl to prevent that.
##
## This code works for versions of both forms major.minor and major.minor.revision
## Get Xcode version and remove "Xcode " from resulting string
fullversion=`xcodebuild -version | cut -d' ' -f2`
## Remove all after the actual version number x.y or x.y.z under bash
fullversion=`echo $fullversion | cut -d' ' -f1`
## The next line is needed under zsh to finish removing all after x.y or x.y.z
fullversion=`echo $fullversion | sed '/version/d'`
major=`echo $fullversion | cut -d. -f1`
minor=`echo $fullversion | cut -d. -f2`
## $revision will be empty string if no revision number (only x.y not x.y.z)
##revision=`echo $fullversion | cut -d. -f3` # We don't need the revision number
if [[ $major -lt 10  || ($major -eq 10 && $minor -lt 2 ) ]]; then
# Disable avx-512 support because not available in this Xcode verson
    rm -f crypto/bn/asm/rsaz-avx512.pl.orig
    rm -f /tmp/rsaz-avx512_pl_diff
    # We must escape all the $ as \$ in the diff for the shell to treat them as literals
    cat >> /tmp/rsaz-avx512_pl_diff << ENDOFFILE
--- /Volumes/Dev/BOINC_GIT/openssl-3.0.0-patched/crypto/bn/asm/rsaz-avx512-orig.pl    2021-09-07 04:46:32.000000000 -0700
+++ /Volumes/Dev/BOINC_GIT/openssl-3.0.0-patched/crypto/bn/asm/rsaz-avx512.pl    2021-10-14 01:16:23.000000000 -0700
@@ -52,6 +52,9 @@
     \$avx512ifma = (\$2>=7.0);
 }

+# Disable avx-512 support because not available in this Xcode verson
+\$avx512ifma = 0;
+
 open OUT,"| \"\$^X\" \"\$xlate\" \$flavour \"\$output\""
     or die "can't call \$xlate: \$!";
 *STDOUT=*OUT;
ENDOFFILE

    patch -b -f -i /tmp/rsaz-avx512_pl_diff crypto/bn/asm/rsaz-avx512.pl
    rm -f /tmp/rsaz-avx512_pl_diff
    rm -f crypto/bn/asm/rsaz-avx512.pl.rej
else
    echo "crypto/bn/asm/rsaz-avx512.pl is OK for this Xcode version\n"
fi
echo ""

## The "-Werror=unguarded-availability" compiler flag generates an error if
## there is an unguarded API not available in our Deployment Target. This
## helps ensure openssl won't try to use unavailable APIs on older Mac
## systems supported by BOINC.
##
export CC="${GCCPATH}";export CXX="${GPPPATH}"
export CPPFLAGS=""
export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,x86_64"
export CXXFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -arch x86_64 -mmacosx-version-min=10.13 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300"
export CFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -arch x86_64 -mmacosx-version-min=10.13 -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300"
export SDKROOT="${SDKPATH}"
export MACOSX_DEPLOYMENT_TARGET=10.13
export LIBRARY_PATH="${SDKPATH}/usr/lib"

if [ "x${lprefix}" != "x" ]; then
    ./configure --prefix=${lprefix} no-shared darwin64-x86_64-cc
    if [ $? -ne 0 ]; then return 1; fi
else
    ./configure no-shared darwin64-x86_64-cc
    if [ $? -ne 0 ]; then return 1; fi
fi

if [ "${doclean}" = "yes" ]; then
    make clean 1>$stdout_target
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

# Now see if we can build for arm64
# Note: Some versions of Xcode 12 don't support building for arm64
if [ $GCC_can_build_arm64 = "yes" ]; then

    export CC="${GCCPATH}";export CXX="${GPPPATH}"
    export LDFLAGS="-Wl,-syslibroot,${SDKPATH},-arch,arm64"
    export CPPFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -target arm64-apple-macos -mmacosx-version-min=10.13 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300"
    export CXXFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -target arm64-apple-macos -mmacosx-version-min=10.13 -stdlib=libc++ -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300"
    export CFLAGS="-isysroot ${SDKPATH} -Werror=unguarded-availability -mmacosx-version-min=10.13 -target arm64-apple-macos -DMAC_OS_X_VERSION_MAX_ALLOWED=101300 -DMAC_OS_X_VERSION_MIN_REQUIRED=101300"
    export SDKROOT="${SDKPATH}"
    export MACOSX_DEPLOYMENT_TARGET=10.13
    export LIBRARY_PATH="${SDKPATH}/usr/lib"

    if [ "x${lprefix}" != "x" ]; then
        ./configure --prefix=${lprefix} no-shared darwin64-arm64-cc
        if [ $? -ne 0 ]; then return 1; fi
    else
        ./configure no-shared darwin64-arm64-cc
        if [ $? -ne 0 ]; then return 1; fi
    fi

    # save x86_64 lib for later use
    mv -f libcrypto.a libcrypto_x86_64.a
    mv -f libssl.a libssl_x86_64.a

    make clean 1>$stdout_target

    make 1>$stdout_target
    if [ $? -ne 0 ]; then
        echo "              ******"
        echo "OpenSSL: x86_64 build succeeded but could not build for arm64."
        echo "              ******"
        rm -f libcrypto_x86_64.a
        rm -f libssl_x86_64.a
    fi

    mv -f libcrypto.a libcrypto_arm64.a
    mv -f libssl.a libssl_arm64.a

    # combine x86_64 and arm libraries
    lipo -create libcrypto_x86_64.a libcrypto_arm64.a -output libcrypto.a
    if [ $? -eq 0 ]; then
        lipo -create libssl_x86_64.a libssl_arm64.a -output libssl.a
        if [ $? -ne 0 ]; then
            rm -f libcrypto_x86_64.a libcrypto_arm64.a
            rm -f libssl_x86_64.a libssl_arm64.a
            return 1;
        fi
    fi

    rm -f libcrypto_x86_64.a libcrypto_arm64.a
    rm -f libssl_x86_64.a libssl_arm64.a

    ## openssl 1.1.0g does not have a configure option for darwin arm64, so we
    ## patched Configurations/10-main.conf to add it.
    ## NOTE: At the time of writing, I do not have an arm64 Mac to test with.
    # Revisit this if a newer version of openssl becomes available.
    #
    # Get the names of the current versions of and openssl from the
    # dependencyNames.sh file in the same directory as this script.
    myScriptPath="${BASH_SOURCE[0]}"
    if [ -z ${myScriptPath} ]; then
        myScriptPath="$0"   # for zsh
    fi
    myScriptDir="${myScriptPath%/*}"
    source "${myScriptDir}/dependencyNames.sh"
fi

if [ "x${lprefix}" != "x" ]; then
    make install 1>$stdout_target
    if [ $? -ne 0 ]; then return 1; fi
fi

lprefix=""
export CC="";export CXX=""
export LDFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export SDKROOT=""

return 0
