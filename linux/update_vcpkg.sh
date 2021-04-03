#!/bin/sh
set -e

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

cache_dir="$PWD/3rdParty/buildCache/linux"
build_dir="$PWD/3rdParty/linux"
vcpkg_root="$build_dir/vcpkg"

export XDG_CACHE_HOME=$cache_dir/vcpkgcache

if [ ! -d $vcpkg_root ]; then
    mkdir -p $build_dir
    git -C $build_dir clone https://github.com/microsoft/vcpkg
fi

git -C $vcpkg_root pull
$vcpkg_root/bootstrap-vcpkg.sh
$vcpkg_root/vcpkg install rappture --clean-after-build
$vcpkg_root/vcpkg upgrade --no-dry-run
