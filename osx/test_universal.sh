#!/bin/sh
set -e

folders_list="build-universal-osx"
libs_list=$(find $folders_list -type f -name "*.a")

for lib in $libs_list; do
    echo "Test lib: $lib"
    if ! lipo $lib -verify_arch x86_64 arm64; then
        echo "lib: $(basename $lib) is not universal"
        lipo -info $lib
    fi
done
