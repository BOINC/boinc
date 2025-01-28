@echo off

if not exist "windows" (
    echo start this script in the source root directory
    exit 1
)

if [%1] == [] (
    set "PLATFORM=x64"
) else (
    set "PLATFORM=%1"
)

if [%2] == [] (
    set "CONFIGURATION=Release"
) else (
    set "CONFIGURATION=%2"
)

echo PLATFORM: %PLATFORM%
echo CONFIGURATION: %CONFIGURATION%

set "BUILD_DIR=%CD%\3rdParty\Windows"
set "VCPKG_PORTS=%CD%\3rdParty\vcpkg_ports"
set "VCPKG_ROOT=%BUILD_DIR%\vcpkg"


call windows\bootstrap_vcpkg_cmake.bat

cmake lib -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DVCPKG_MANIFEST_DIR=3rdParty\vcpkg_ports\configs\libs\windows -DVCPKG_INSTALLED_DIR=%VCPKG_ROOT%\installed -DVCPKG_OVERLAY_PORTS=%VCPKG_PORTS%\ports -DVCPKG_OVERLAY_TRIPLETS=%VCPKG_PORTS%\triplets\ci -DVCPKG_TARGET_TRIPLET=%PLATFORM%-windows-static -DVCPKG_INSTALL_OPTIONS=--clean-after-build
cmake --build build --config %CONFIGURATION%
