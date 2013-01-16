#/bin/sh
#script to setup Android toolchain

export ANDROIDTC="~/android-tc"

if [ ! -d "$ANDROIDTC" ]; then

    cd ~/NVPACK/android-ndk-r8
    build/tools/make-standalone-toolchain.sh --platform=android-4 --install-dir=$ANDROIDTC
    cd ~

fi
