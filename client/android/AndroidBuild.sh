#!/bin/bash

# Script to compile BOINC for Android.
# Requires the BOINC source directory and the Android NDK.
#
# Date: December 12th, 2011
# Author: Peter Hanappe (Sony Computer Science Laboratory)

BOINCDIR=`pwd`
NDK_SYSROOT=""
NDK_PATH=""
NDK_ARCH="arm"
NDK_ROOT=""
NDK_ABI="armeabi"
NDK_VERSION="android-8"
COMPILE_BOINC="yes"
COMPILE_ZLIB="yes"
COMPILE_OPENSSL="yes"
COMPILE_CURL="yes"
CONFIGURE="yes"
MAKE_CLEAN="no"

function print_usage()
{
    echo "Options:"
    echo "  --help"
    echo "  --boinc-dir <dir>       Root directory of BOINC source tree (default: current dir)."
    echo "  --android-root <dir>    The root directory of the Android NDK."
    echo "  --android-sysroot <dir> The target sysroot directory of the Android NDK."
    echo "  --android-bin <dir>     The bin directory of the Android NDK compilers."
    echo "  --android-arch <dir>    Compile for 'arm' or 'x86' architecture (default: arm)."
    echo "  --android-version <val> Specify Android version (default: android-8)."
    echo "  --skip-boinc            Don't compile BOINC."
    echo "  --skip-zlib             Don't download and compile zlib."
    echo "  --skip-openssl          Don't download and compile OpenSSL."
    echo "  --skip-curl             Don't download and compile curl."
    echo "  --skip-configure        Don't run configure script before compiling."
    echo "  --make-clean            Run 'make clean' before compiling."
    echo "Example: ./AndroidBuild.sh \\"
    echo "           --android-root /opt/google/android-ndk-r6 \\"
    echo "           --android-bin /opt/google/android-ndk-r6/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin"
    exit 0
}

while (($#)); do
      option=$1
      shift
      case "$option" in
          --help) 
              print_usage
              shift 
              ;;
          --boinc-dir) 
              BOINCDIR=$1
              shift 
              ;;
          --android-sysroot) 
              NDK_SYSROOT=$1
              shift 
              ;;
          --android-root) 
              NDK_ROOT=$1
              shift 
              ;;
          --android-bin) 
              NDK_PATH=$1
              shift 
              ;;
          --android-arch) 
              NDK_ARCH=$1
              shift 
              ;;
          --android-version) 
              NDK_VERSION=$1
              shift 
              ;;
          --skip-boinc) 
              COMPILE_BOINC="no"
              ;;
          --skip-zlib) 
              COMPILE_ZLIB="no"
              ;;
          --skip-openssl) 
              COMPILE_OPENSSL="no"
              ;;
          --skip-curl) 
              COMPILE_CURL="no"
              ;;
          --skip-configure) 
              CONFIGURE="no"
              ;;
          --make-clean) 
              MAKE_CLEAN="yes"
              ;;
          *) 
              echo "Unknown option ${option}"
              print_usage
              ;;
      esac      
done

###################################################
# Test options and paths

if [ ! -e "${BOINCDIR}/client/gui_rpc_server.h" ]; then
   echo "Please launch this script in the boinc root directory, or use the --boinc-dir <dir> option."
   print_usage
fi

if [ "x${NDK_ROOT}" == "x" ]; then
   echo "Please specify the Android NDK root directory."
   print_usage
fi

if [ ${NDK_ARCH} == "arm" ]; then
    NDK_HOST=arm-linux-androideabi
elif [ ${NDK_ARCH} == "x86" ]; then
    NDK_HOST=i686-android-linux
else
    echo "Please specify the Android architecture."
    print_usage
fi

NDK_CC=${NDK_HOST}-gcc

if [ "x${NDK_SYSROOT}" == "x" ]; then
    NDK_SYSROOT=${NDK_ROOT}/platforms/${NDK_VERSION}/arch-${NDK_ARCH}
    if [ ! -d ${NDK_SYSROOT} ]; then
        echo "Please specify the Android NDK sysroot."
        echo "Tried, but failed: ${NDK_SYSROOT}"
        print_usage
    fi
fi

if [ "x${NDK_PATH}" == "x" ]; then
   echo "Please specify the Android binary directory."
   print_usage
fi

if [ ! -e "${NDK_SYSROOT}/usr/include/stdio.h" ]; then
    echo "Could not find ${NDK_SYSROOT}/usr/include/stdio.h header. Please verify the sysroot."
    print_usage
fi

if [ ! -e "${NDK_PATH}/${NDK_CC}" ]; then
    echo "Could not find the ${NDK_CC} compiler. Please verify the binary path."
    print_usage
fi

###################################################

echo "================================================"
echo "BOINCDIR        ${BOINCDIR}"
echo "NDK_ROOT        ${NDK_ROOT}"
echo "NDK_SYSROOT     ${NDK_SYSROOT}"
echo "NDK_PATH        ${NDK_PATH}"
echo "NDK_ARCH        ${NDK_ARCH}"
echo "NDK_HOST        ${NDK_HOST}"
echo "NDK_ABI         ${NDK_ABI}"
echo "NDK_VERSION     ${NDK_VERSION}"
echo "NDK_CC          ${NDK_CC}"
echo "COMPILE_ZLIB    $COMPILE_ZLIB"
echo "COMPILE_OPENSSL $COMPILE_OPENSSL"
echo "COMPILE_CURL    $COMPILE_CURL"
echo "CONFIGURE       $CONFIGURE"
echo "MAKE_CLEAN      $MAKE_CLEAN"
echo "================================================"

###################################################

function compile_zlib()
{
    echo "================================================"
    echo "= Starting download and compilation of zlib    ="
    echo "================================================"

    cd ${BOINCDIR}/android

    if [ ! -e zlib-1.2.5 ]; then
        wget http://zlib.net/zlib-1.2.5.tar.gz
        tar xzvf zlib-1.2.5.tar.gz 
    fi

    cd zlib-1.2.5

    if [ $CONFIGURE == "yes" ]; then
        CC="${NDK_PATH}/${NDK_CC} --sysroot=${NDK_SYSROOT}" \
            ./configure --prefix=${BOINCDIR}/android/${NDK_ARCH} --static 
    fi
    if [ $MAKE_CLEAN == "yes" ]; then
        make clean
    fi
    make
# The 'make install' script fails
    mkdir -p ${BOINCDIR}/android/${NDK_ARCH}/lib
    mkdir -p ${BOINCDIR}/android/${NDK_ARCH}/include
    cp libz.a ${BOINCDIR}/android/${NDK_ARCH}/lib
    cp zlib.h ${BOINCDIR}/android/${NDK_ARCH}/lib
    cp zconf.h ${BOINCDIR}/android/${NDK_ARCH}/lib
}

###################################################

function compile_openssl()
{
    echo "================================================"
    echo "= Starting download and compilation of openssl ="
    echo "================================================"

    cd ${BOINCDIR}/android

    if [ ! -e openssl-1.0.0e ]; then
        wget http://www.openssl.org/source/openssl-1.0.0e.tar.gz
        tar xzvf openssl-1.0.0e.tar.gz 
    fi

    cd openssl-1.0.0e

    if [ $CONFIGURE == "yes" ]; then
        CC="${NDK_PATH}/${NDK_CC} --sysroot=${NDK_SYSROOT}" \
            ./config --prefix=${BOINCDIR}/android/${NDK_ARCH} no-shared no-asm
    fi

    if [ $MAKE_CLEAN == "yes" ]; then
        make clean
    fi

    make
    make install
}

###################################################

function compile_curl()
{
    echo "================================================"
    echo "= Starting download and compilation of curl ="
    echo "================================================"

    cd ${BOINCDIR}/android

    if [ ! -e curl-7.23.1 ]; then
        wget http://curl.haxx.se/download/curl-7.23.1.tar.gz
        tar xzvf curl-7.23.1.tar.gz 
    fi

    cd curl-7.23.1

# Export binary path.
    export PATH=${PATH}:${NDK_PATH}

#
# ./configure --with-sysroot=${NDK_SYSROOT} doesn't seem to work. Pass
# the correct path using the CFLAGS environment variable.
#
# The ANDROID flag is used in curl-7.23.1/include/curl/curl.h
#
    if [ $CONFIGURE == "yes" ]; then
        CFLAGS="--sysroot=${NDK_SYSROOT} -DANDROID" \
            ./configure --host=${NDK_HOST} \
            --with-sysroot=${NDK_SYSROOT} \
            --prefix=${BOINCDIR}/android/${NDK_ARCH} --enable-debug \
            --disable-shared --enable-static \
            --enable-http --disable-ftp --disable-ldap --disable-ldaps \
            --disable-rtsp --disable-proxy --disable-dict --disable-telnet \
            --disable-tftp --disable-pop3 --disable-imap --disable-smtp \
            --disable-gopher --disable-ipv6 --disable-sspi --disable-crypto-auth \
            --disable-tls-srp --disable-ntlm-wb --disable-cookies --disable-manual \
            --disable-threaded-resolver --without-ca-bundle
    fi

    if [ $MAKE_CLEAN == "yes" ]; then
        make clean
    fi

    make
    make install
}

###################################################
# Problems: 
# - problems in configure.ac with curl: --with-curl doesn't use the correct path
# - config.sub too old to recognize --host=arm-linux-androideabi, used newer config.sub (2011-03-23)
# - changed lib/network.cpp:298: !defined(__ANDROID__)
# - changed lib/synch.cpp: #if defined(__ANDROID__) \ #define create_semaphore(_key) (-1), ...
# - changed lib/mac_address.cpp:189: #elif (defined(SIOCGIFCONF) || defined(SIOCGLIFCONF)) && !defined(__ANDROID__)
function compile_boinc()
{
    cd ${BOINCDIR}

# Export binary path.
    export PATH=${PATH}:${NDK_PATH}

    if [ $CONFIGURE == "yes" ]; then

        bash _autosetup

        OPTIONS="--host=${NDK_HOST} \
            --disable-server \
            --enable-client \
            --disable-manager \
            --enable-libraries \
            --enable-debug \
            --with-ssl=${BOINCDIR}/android/arm/lib"

        echo "================================================"
        echo "Configuring BOINC with: $OPTIONS"
        echo "================================================"

        FLAGS="--sysroot=${NDK_SYSROOT} \
            -I${NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/include \
            -I${NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/libs/${NDK_ABI}/include"
        
        CFLAGS="${FLAGS}" \
        CXXFLAGS="${FLAGS}" \
        LDFLAGS="-L${BOINCDIR}/android/arm/lib -L${NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/libs/${NDK_ABI}" \
        LIBS="-lstdc++" \
            ./configure ${OPTIONS}
    fi

    if [ $MAKE_CLEAN == "yes" ]; then
        make clean
    fi

    make
}

###################################################

if [ ! -e "${BOINCDIR}/android" ]; then
    echo "================================================"
    echo "= Creating the android direcory                ="
    echo "================================================"
    mkdir ${BOINCDIR}/android
fi
if [ $COMPILE_ZLIB == "yes" ]; then
    compile_zlib
fi
if [ $COMPILE_OPENSSL == "yes" ]; then
    compile_openssl
fi
if [ $COMPILE_CURL == "yes" ]; then
    compile_curl
fi
if [ $COMPILE_BOINC == "yes" ]; then
    compile_boinc
fi

