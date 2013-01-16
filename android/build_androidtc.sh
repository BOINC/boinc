#/bin/sh
#script to setup Android toolchain

export ANDROIDTC="$HOME/android-tc"

if [ ! -d $ANDROIDTC ]; then
    $NDKROOT/build/tools/make-standalone-toolchain.sh --platform=android-4 --install-dir=$ANDROIDTC
fi
