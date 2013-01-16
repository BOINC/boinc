#/bin/sh
#script to setup Android toolchain

export ANDROIDTC="~/android-tc"
export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$PATH:$TCBINARIES:$TCINCLUDES/bin"
export CC=arm-linux-androideabi-gcc
export CXX=arm-linux-androideabi-g++
export LD=arm-linux-androideabi-ld

if [ ! -d "$ANDROIDTC" ]; then

    cd ~/NVPACK/android-ndk-r8
    build/tools/make-standalone-toolchain.sh --platform=android-4 --install-dir=$ANDROIDTC
    cd ~

fi
