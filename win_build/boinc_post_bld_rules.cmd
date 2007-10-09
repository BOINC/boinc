@echo off
rem Berkeley Open Infrastructure for Network Computing
rem http://boinc.berkeley.edu
rem Copyright (C) 2005 University of California
rem 
rem This is free software; you can redistribute it and/or
rem modify it under the terms of the GNU Lesser General Public
rem License as published by the Free Software Foundation;
rem either version 2.1 of the License, or (at your option) any later version.
rem 
rem This software is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
rem See the GNU Lesser General Public License for more details.
rem 
rem To view the GNU Lesser General Public License visit
rem http://www.gnu.org/copyleft/lesser.html
rem or write to the Free Software Foundation, Inc.,
rem 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
rem

FOR /F "usebackq delims==" %%I IN ('%1') DO set PROJECTROOTDIR=%%~I\..
FOR /F "usebackq delims==" %%J IN ('%2') DO set OUTPUTDIR=%%~J
FOR /F "usebackq delims==" %%K IN ('%3') DO set PLATFORMNAME=%%~K


if not exist %OUTPUTDIR%\dbghelp.dll (
    echo Coping dbghelp to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\dbghelp.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\dbghelp95.dll (
    echo Coping dbghelp95 to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\dbghelp95.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\symsrv.dll (
    echo Coping symsrv to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\symsrv.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\symsrv.yes (
    echo Coping symsrv.yes to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\symsrv.yes" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\srcsrv.dll (
    echo Coping srcsrv.dll to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PLATFORMNAME%\srcsrv.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\libcurl.dll (
    echo Coping libcurl to the output directory...
    copy "%PROJECTROOTDIR%\curl\mswin\%PLATFORMNAME%\bin\libcurl.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\ca-bundle.crt (
    echo Coping ca-bundle.crt to the output directory...
    copy "%PROJECTROOTDIR%\curl\ca-bundle.crt" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\libeay32.dll (
    echo Coping libeay32 to the output directory...
    copy "%PROJECTROOTDIR%\openssl\mswin\%PLATFORMNAME%\bin\libeay32.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\ssleay32.dll (
    echo Coping ssleay32 to the output directory...
    copy "%PROJECTROOTDIR%\openssl\mswin\%PLATFORMNAME%\bin\ssleay32.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\zlib1.dll (
    echo Coping zlib1 to the output directory...
    copy "%PROJECTROOTDIR%\zlib\mswin\%PLATFORMNAME%\bin\zlib1.dll" "%OUTPUTDIR%"
)

