@ECHO OFF
REM ----------------------------------------------------------
REM - Autobuild Batch file for DC-API client side on Windows -
REM ----------------------------------------------------------

IF NOT EXIST C:\Cygwin\var\lib\autobuild\dcapi\win_build\build GOTO build

rd /S /Q C:\Cygwin\var\lib\autobuild\dcapi\win_build\build

:build

call "C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"

cd C:\Cygwin\var\lib\autobuild\dcapi\win_build

devenv dcapi.sln /build debug

devenv dcapi.sln /build release