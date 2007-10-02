@ECHO OFF
REM ----------------------------------------------------------
REM - Autobuild Batch file for DC-API client side on Windows -
REM ----------------------------------------------------------

set dir=C:\Cygwin\home\kadam\dc

call "C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"

cd %dir%\dcapi\win_build

devenv dcapi_2003.sln /build debug
devenv dcapi_2003.sln /build release

cd %dir%\dcapi\win_build\Build\Release\obj

lib /OUT:../libdc-client-boinc.lib client.obj util.obj logger.obj cfg-client.obj "%dir%\boinc_samples\win_build\Release\libboinc.lib" "%dir%\boinc_samples\win_build\Release\libboincapi.lib"

cd %dir%\dcapi\win_build\Build\Debug\obj

lib /OUT:../libdc-client-boinc-debug.lib client.obj util.obj logger.obj cfg-client.obj "%dir%\boinc_samples\win_build\Debug\libboinc.lib" "%dir%\boinc_samples\win_build\Debug\libboincapi.lib"

cd ../../..