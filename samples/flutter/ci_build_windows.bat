

REM # Script to compile everything BOINC needs for Windows flutter

REM # check working directory because the script needs to be called like: ./samples/flutter/ci_build_manager.sh
if not exist "samples\flutter\" (
    echo "start this script in the source root directory"
    exit 1
)


echo '===== BOINC Flutuer Windows build start ====='
vcpkg.exe integrate remove
msbuild win_build\boinc_vs2019.sln -p:Configuration=Release -p:Platform=x64 -p:VcpkgTripletConfig=ci -m

copy "win_build\Build\x64\Release\boinc.exe" "samples\flutter\boinc\assets\windows\boinc.exe"
type "samples\flutter\boinc\windows.yaml" >> "samples\flutter\boinc\pubspec.yaml"
echo '===== BOINC Flutuer Windows build done ====='
