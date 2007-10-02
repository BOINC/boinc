set dir=C:\Documents and Settings\Adam\My Documents\_svn\boinc_ws_kadam

call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

cd %dir%\dcapi\win_build

devenv dcapi.sln /build debug
devenv dcapi.sln /build release

cd %dir%\dcapi\win_build\Build\Release\obj

lib /OUT:../libdc-client-boinc.lib client.obj util.obj logger.obj cfg-client.obj "%dir%\boinc_samples\win_build\Build\Win32\Release\libboinc.lib" "%dir%\boinc_samples\win_build\Build\Win32\Release\libboincapi.lib"

cd %dir%\dcapi\win_build\Build\Debug\obj

lib /OUT:../libdc-client-boinc-debug.lib client.obj util.obj logger.obj cfg-client.obj "%dir%\boinc_samples\win_build\Build\Win32\Debug\libboinc.lib" "%dir%\boinc_samples\win_build\Build\Win32\Debug\libboincapi.lib"

cd ../../..