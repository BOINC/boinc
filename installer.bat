@echo off

rem "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

echo *********************************
echo **** BOINC INSTALLER BUILDER ****
echo *********************************
echo.

set /A BUILD_X64=1
set /A BUILD_X64_VBOX=0
set /A BUILD_ARM=0

REM Preparations and requirements check
ECHO [Requirements check: MSBuild]
msbuild --version
if %ERRORLEVEL% GEQ 1 goto fail

ECHO [Requirements check: Wix Toolkit]
heat -help
if %ERRORLEVEL% GEQ 1 goto fail

if "%1" EQU "--clean" (
    ECHO [Cleanup]
    DEL /S /Q build 2> NUL
    echo OK

    ECHO [Preparing build dir]
    MKDIR build\  2> NUL
    if %ERRORLEVEL% GEQ 1 goto fail

    echo OK
    exit /B 0
) else (
    ECHO [Fast Cleanup]
    DEL /S /Q build\installer* 2> NUL
    DEL /S /Q build\boinc_*.wxs 2> NUL
    echo OK
)

REM Parameters check
if "%1" EQU "--x64" (
    set /A BUILD_X64=1
)
if "%1" EQU "--x64_vbox" (
    set /A BUILD_X64_VBOX=1
)
if "%1" EQU "--arm" (
    set /A BUILD_ARM=1
)

if %BUILD_X64% EQU 0 (
    if %BUILD_X64_VBOX% EQU 0 (
        if %BUILD_X64_VBOX% EQU 0 (
            echo Usage: installer.bat ^< --clean|--x64|--x64_vbox|--arm ^>
            echo.
            goto fail
        )
    )
)

echo [Additional files copy]
COPY /Y api\ttf\liberation-fonts-ttf-2.00.0\LiberationMono-Regular.ttf build\LiberationMono-Regular.ttf
if %ERRORLEVEL% GEQ 1 goto fail
COPY /Y COPYING build\COPYING
if %ERRORLEVEL% GEQ 1 goto fail
COPY /Y COPYRIGHT build\COPYRIGHT
if %ERRORLEVEL% GEQ 1 goto fail
COPY /Y doc\logo\boinc_logo_black.jpg build\boinc_logo_black.jpg
if %ERRORLEVEL% GEQ 1 goto fail

echo OK

:installer
echo [Build of installer]
pushd win_build\installer_wix
msbuild installer.sln
if %ERRORLEVEL% GEQ 1 goto fail

popd
echo ********************************
echo **** RESULT: SUCCESS        ****
echo ********************************
exit /B 0

:fail
popd
echo ********************************
echo **** RESULT: FAILURE        ****
echo ********************************
exit /B 1

