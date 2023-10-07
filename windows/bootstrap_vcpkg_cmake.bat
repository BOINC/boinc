@echo off

if not exist "windows" (
    echo start this script in the source root directory
    exit 1
)

set "BUILD_DIR=%CD%\3rdParty\Windows"
set "VCPKG_PORTS=%CD%\3rdParty\vcpkg_ports"
set "VCPKG_ROOT=%BUILD_DIR%\vcpkg"

call "%CD%\3rdParty\vcpkg_ports\vcpkg_link.bat"

if not exist "%VCPKG_ROOT%" (
    mkdir -p "%VCPKG_ROOT%"
    git -C %BUILD_DIR% clone %VCPKG_LINK%
)


git -C %VCPKG_ROOT% pull
%VCPKG_ROOT%\bootstrap-vcpkg.bat
