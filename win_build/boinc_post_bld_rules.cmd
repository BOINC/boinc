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
rem 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
rem

set PROJECTROOTDIR=%1\..
set OUTPUTDIR=%2

if not exist %OUTPUTDIR%\libcurl.dll (
    echo Coping libcurl to the output directory...
    copy "%PROJECTROOTDIR%\curl\mswin\%PROCESSOR_ARCHITECTURE%\bin\libcurl.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\libeay32.dll (
    echo Coping libeay32 to the output directory...
    copy "%PROJECTROOTDIR%\openssl\mswin\%PROCESSOR_ARCHITECTURE%\bin\libeay32.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\ssleay32.dll (
    echo Coping ssleay32 to the output directory...
    copy "%PROJECTROOTDIR%\openssl\mswin\%PROCESSOR_ARCHITECTURE%\bin\ssleay32.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\zlib1.dll (
    echo Coping zlib1 to the output directory...
    copy "%PROJECTROOTDIR%\zlib\mswin\%PROCESSOR_ARCHITECTURE%\bin\zlib1.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\dbghelp.dll (
    echo Coping dbghelp to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PROCESSOR_ARCHITECTURE%\dbghelp.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\dbghelp95.dll (
    echo Coping dbghelp95 to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PROCESSOR_ARCHITECTURE%\dbghelp95.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\symsrv.dll (
    echo Coping symsrv to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PROCESSOR_ARCHITECTURE%\symsrv.dll" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\symsrv.yes (
    echo Coping symsrv.yes to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PROCESSOR_ARCHITECTURE%\symsrv.yes" "%OUTPUTDIR%"
)

if not exist %OUTPUTDIR%\srcsrv.dll (
    echo Coping srcsrv.dll to the output directory...
    copy "%PROJECTROOTDIR%\win_build\installerv2\redist\Windows\%PROCESSOR_ARCHITECTURE%\srcsrv.dll" "%OUTPUTDIR%"
)
