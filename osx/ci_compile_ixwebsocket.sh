#!/bin/sh

#!/bin/sh
set -e

echo ARCHS: $ARCHS
echo PREFIX: $PREFIX

for ARCH in $ARCHS; do
    echo "Building ixwebsocket for architecture: $ARCH"

    if [ "$ARCH" = "x86_64" ]; then
        SYSTEM_PROCESSOR="x64"
        OSX_ARCHITECTURES="x86_64"
    fi
    if [ "$ARCH" = "arm64" ]; then
        SYSTEM_PROCESSOR="arm64"
        OSX_ARCHITECTURES="arm64"
    fi

    cmake . -B build_$ARCH \
        -DUSE_TLS=ON \
        -DUSE_OPEN_SSL=ON \
        -DCMAKE_SYSTEM_PROCESSOR=$SYSTEM_PROCESSOR \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_SYSTEM_NAME=Darwin \
        -DCMAKE_OSX_ARCHITECTURES=$OSX_ARCHITECTURES \
        -DCMAKE_INSTALL_PREFIX="$PREFIX/$ARCH" \
        -DCMAKE_PREFIX_PATH="$PREFIX"

    cmake --build build_$ARCH --config Release --target install
done

echo " "
echo "Copy includes"
mkdir -p "$PREFIX/lib"
cp -R $PREFIX/x86_64/include $PREFIX

for lib_x64 in $PREFIX/x86_64/lib/*.a; do
    lib_full_name=$(basename $lib_x64)
    lib_name=$(basename -s .a $lib_x64)
    lib_arm64=""
    lib_universal="$PREFIX/lib/$lib_full_name"
    if   [ -f "$PREFIX/x86_64/lib/$lib_full_name" ]; then
            lib_arm64="$PREFIX/arm64/lib/$lib_full_name"
    elif [ -f "$PREFIX/arm64/lib/$lib_name-Darwin.a" ]; then
            lib_arm64="$PREFIX/arm64/lib/$lib_name-Darwin.a"
    fi
    if [ ! -z $lib_arm64 ]; then
        echo exist $lib_full_name
        echo "lib_x64: ${lib_x64}"
        echo "lib_arm64: ${lib_arm64}"
        echo "lib_universal: ${lib_universal}"

        lipo -create  "$lib_x64" "$lib_arm64" -output $lib_universal
        if ! lipo "$lib_x64" -verify_arch x86_64; then
            echo "Fail verify x86_64 on $lib_x64"
            exit 1
        fi
        if ! lipo "$lib_arm64" -verify_arch arm64; then
            echo "Fail verify arm64 on $lib_arm64"
            exit 1
        fi
        if ! lipo "$lib_universal" -verify_arch x86_64 arm64; then
            echo "Fail verify x86_64 arm64 on $lib_universal"
            exit 1
        fi
    else
        echo Not exist $lib_full_name
        exit 1
    fi
done

rm -rf $PREFIX/x86_64
rm -rf $PREFIX/arm64
