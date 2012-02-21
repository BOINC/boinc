rem @echo off

set PREFIX=e:\usr\local\seti
set PATH_WARPIN=e:\bin\WarpIn
set OS2_TOOLKIT=e:\dev\toolkit452

set boinc_Major=5
set boinc_Minor=2
set boinc_Rev=1
set boinc_Bld=1

set PATH_boinc=boinc-%boinc_Major%.%boinc_Minor%.%boinc_Rev%

set boinc_VER=%boinc_Major%.%boinc_Minor%.%boinc_Rev%
set boinc-VER=%boinc_Major%-%boinc_Minor%-%boinc_Rev%
set boincsVER=%boinc_Major%\%boinc_Minor%\%boinc_Rev%

set WPI=boinc-%boinc_Major%-%boinc_Minor%-%boinc_Rev%-b%boinc_Bld%.wpi
set beginlibpath=%PATH_WARPIN%;%beginlibpath%

del %PREFIX\%WPI% >nul 2>nul

REM *** create ppwizard header ***
echo #define WARPIN_VERSION "1.0.8"     > boinc.ih
echo #define boinc.Major %boinc_Major% >> boinc.ih
echo #define boinc.Minor %boinc_Minor% >> boinc.ih
echo #define boinc.Rev %boinc_Rev%     >> boinc.ih
echo #define boinc.BLD %boinc_Bld%     >> boinc.ih
echo #define boinc\VER %boinc_Major%\%boinc_Minor%\%boinc_Rev%\%boinc_Bld% >> boinc.ih
echo #define boinc.VER %boinc_Major%.%boinc_Minor%.%boinc_Rev% >> boinc.ih
echo #define boinc-VER %boinc_Major%-%boinc_Minor%-%boinc_Rev% >> boinc.ih

REM *** create scripts ***
call ppwizard.cmd boinc.wis /Pack:N /output:%PREFIX%\boinc.wis /other

REM *** add OS/2 files ****
%OS2_TOOLKIT%\bin\mapsym ..\client\boinc_client.map
%OS2_TOOLKIT%\bin\mapsym ..\lib\boinc_cmd.map
%OS2_TOOLKIT%\bin\mapsym ..\lib\crypt_prog.map
splitdbg %PREFIX%\bin\boinc_client.exe
splitdbg %PREFIX%\bin\boinc_cmd.exe
splitdbg %PREFIX%\bin\crypt_prog.exe

copy ReadMe.txt %PREFIX%
copy COPYING %PREFIX%
copy *.ico %PREFIX%
copy e:\bin\dll\libc06.dll %PREFIX%\bin
copy *.sym %PREFIX%\bin
copy boinc_attach.cmd %PREFIX%\bin
touch %PREFIX%\bin\*.exe %PREFIX%\bin\*.sym

REM *** delete temp files here ***
del *.dbg
del *.sym

REM *** start packaging ***
setlocal
cd %PREFIX%

SET INSTALL_PARM=-a 1 *.ico 1 ReadMe.txt 1 COPYING 1 bin\libc06.dll 1 bin\boinc_*.* 1 bin\crypt*.* 2 -r include\* 2 lib\* 2 src\*
echo %INSTALL_PARM% > install.lst

%PATH_WARPIN%\wic.exe %WPI% @install.lst
%PATH_WARPIN%\wic.exe %WPI% -s boinc.wis

endlocal

copy %PREFIX%\%WPI% ..\..\
del %PREFIX%\%WPI%
