#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

export ANDROIDTC="$HOME/androidarm-tc"

if [ ! -d $ANDROIDTC/arm-linux-androideabi ]; then
    $NDKROOT/build/tools/make-standalone-toolchain.sh --platform=android-9 --install-dir=$ANDROIDTC
fi
