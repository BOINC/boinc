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

FOR /F "usebackq delims==" %%I IN ('%1') DO set PROJECTROOTDIR=%%~I\..
FOR /F "usebackq delims==" %%J IN ('%2') DO set OUTPUTDIR=%%~J
FOR /F "usebackq delims==" %%K IN ('%3') DO set PLATFORMNAME=%%~K


if not exist %OUTPUTDIR%\COPYING.txt (
    echo Coping COPYING to the source directory...
    copy "%PROJECTROOTDIR%\COPYING" "%OUTPUTDIR%\COPYING.txt"
)

if not exist %OUTPUTDIR%\COPYRIGHT.txt (
    echo Coping COPYRIGHT to the source directory...
    copy "%PROJECTROOTDIR%\COPYRIGHT" "%OUTPUTDIR%\COPYRIGHT.txt"
)