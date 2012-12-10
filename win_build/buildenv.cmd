@IF "%BUILDDBG%"=="TRUE" ( ECHO ON ) ELSE ( ECHO OFF )
rem Berkeley Open Infrastructure for Network Computing
rem http://boinc.berkeley.edu
rem Copyright (C) 2009 University of California
rem 
rem This is free software; you can redistribute it and/or
rem modify it under the terms of the GNU Lesser General Public
rem License as published by the Free Software Foundation;
rem either version 3 of the License, or (at your option) any later version.
rem 
rem This software is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
rem See the GNU Lesser General Public License for more details.
rem 
rem You should have received a copy of the GNU Lesser General Public License
rem along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
rem

rem Provide the groundwork for an automated build environment.

ECHO Initializing BOINC Build Environment for Windows

rem ***** Construct Build Environment Root Paths *****
rem
SET BUILDDRIVE=%~d0
SET BUILDROOT=%~dps0
SET BUILDROOT=%BUILDROOT:\win_bu~1\=%

rem Detect Branch Name
PUSHD %~dp0\..\..
FOR /F %%I IN ("%~dp0\..") DO SET _ArgBUILDROOTLFNSUPPORT=%%~fI
FOR /D %%I IN (*.*) DO (
    IF /I "%CD%\%%I" == "%_ArgBUILDROOTLFNSUPPORT%" SET BUILDBRANCHNAME=%%I
)
POPD


rem ***** Verify Parameters *****
rem
:VERIFYNEXTPARAM
IF /I  "%1"=="/?"                   GOTO :USAGE
IF /I  "%1"=="-?"                   GOTO :USAGE
IF /I  "%1"=="/HELP"                GOTO :USAGE
IF /I  "%1"=="-HELP"                GOTO :USAGE
IF /I  "%1"=="DEVENVDIR"            GOTO :PARSEPARAM
IF /I  "%1"=="TYPE"                 GOTO :PARSEPARAM
IF /I  "%1"=="PLATFORM"             GOTO :PARSEPARAM
IF /I  "%1"=="EXEC"                 GOTO :PARSEPARAM
IF NOT "%1"==""                     GOTO :USAGE
IF     "%1"==""                     GOTO :VALIDATEPARAMS

rem ***** Parse Parameters *****
rem
:PARSEPARAM
IF /I "%1"=="DEVENVDIR"             SET _ArgBuildDevEnvDir=%2
IF /I "%1"=="TYPE"                  SET _ArgBuildType=%2
IF /I "%1"=="PLATFORM"              SET _ArgBuildPlatform=%2
IF /I "%1"=="EXEC" (
    rem The exec command has to be the last of the build arguments
    rem   everything after it should be parameters for the command.
    SET _ArgExec=%2
    SET _ArgExecParam=%~3
    GOTO :VALIDATEPARAMS
)
SHIFT
SHIFT
GOTO :VERIFYNEXTPARAM

rem ***** Validate Parameters *****
rem
:VALIDATEPARAMS
rem A little bit of batchfile magic to remove double quotes
rem   which would be sent from the automated build tools.
FOR /F "usebackq delims=" %%I IN ('%_ArgBuildDevEnvDir%') DO (
    SET _ArgBuildDevEnvDir=%%~I
)
FOR /F "usebackq delims=" %%I IN ('%_ArgBuildType%')      DO (
    SET _ArgBuildType=%%~I
)
FOR /F "usebackq delims=" %%I IN ('%_ArgBuildPlatform%')  DO (
    SET _ArgBuildPlatform=%%~I
)

IF /I "%_ArgBuildType%"==""            GOTO :USAGE
IF /I "%_ArgBuildPlatform%"==""        GOTO :USAGE
IF /I "%_ArgBuildPlatform%" == "x64"   SET _ArgBuildPlatform=amd64
IF /I "%_ArgBuildPlatform%" == "Win32" SET _ArgBuildPlatform=x86

SET BUILDTYPE=%_ArgBuildType%
SET BUILDPLATFORM=%_ArgBuildPlatform%

rem ***** Visual Studio Hint Detection *****
rem
SET _ArgBuildDevEnvDir=%_ArgBuildDevEnvDir:IDE\=%
SET _ArgVS80COMNTOOLS=%VS80COMNTOOLS:Tools\=%
SET _ArgVS90COMNTOOLS=%VS90COMNTOOLS:Tools\=%
SET _ArgVS100COMNTOOLS=%VS100COMNTOOLS:Tools\=%

IF /I "%_ArgVS80COMNTOOLS%" == "%_ArgBuildDevEnvDir%"  GOTO :DETECTVS2005
IF /I "%_ArgVS90COMNTOOLS%" == "%_ArgBuildDevEnvDir%"  GOTO :DETECTVS2008
IF /I "%_ArgVS100COMNTOOLS%" == "%_ArgBuildDevEnvDir%" GOTO :DETECTVS2010

rem ***** Software Detection *****
rem
:DETECTVS2005
IF EXIST "%VS80COMNTOOLS%\vsvars32.bat" (
    ECHO Software Platform Detected: Visual Studio 2005
    CALL "%VS80COMNTOOLS%\vsvars32.bat" > NUL: 2> NUL:
    SET BUILDCOMPILERDETECTED=vs2005
	GOTO :SOFTDETECTIONCOMPLETE
)

:DETECTVS2008
IF EXIST "%VS90COMNTOOLS%\vsvars32.bat" (
    ECHO Software Platform Detected: Visual Studio 2008
    CALL "%VS90COMNTOOLS%\vsvars32.bat" > NUL: 2> NUL:
    SET BUILDCOMPILERDETECTED=vs2008
	GOTO :SOFTDETECTIONCOMPLETE
)

:DETECTVS2010
IF EXIST "%VS100COMNTOOLS%\vsvars32.bat" (
    ECHO Software Platform Detected: Visual Studio 2010
    CALL "%VS100COMNTOOLS%\vsvars32.bat" > NUL: 2> NUL:
    SET BUILDCOMPILERDETECTED=vs2010
	GOTO :SOFTDETECTIONCOMPLETE
)

:SOFTDETECTIONCOMPLETE
IF "%VCINSTALLDIR%"=="" (
	ECHO Software Platform NOT Detected: Microsoft Visual Studio 2005/2008/2010...
    EXIT /B 1
)


rem ***** Build Tools Detection *****
rem
SET BUILDTOOLSNAME=boinc_depends_win_%BUILDCOMPILERDETECTED%
SET BUILDTOOLSROOT=%BUILDROOT%\..\%BUILDTOOLSNAME%
IF EXIST %BUILDTOOLSROOT%\win_build\subversion GOTO :BUILDTOOLSROOTVALIDATED

rem We must be in a branch and we can't currently use subversion commands
rem to determine where we are to find our build tools.  Use some batch
rem file magic to parse the branch name from the name of the directory
rem hosting the build tree. This can be different than just the version
rem number, we have had to rev the branch name for certain releases due 
rem to late changing features. 6.6 vs 6.6a
FOR /F "usebackq tokens=3,* delims=_" %%I IN ('%BUILDBRANCHNAME%') DO (
    SET BUILDTOOLSNAME=boinc_depends_win_%BUILDCOMPILERDETECTED%_%%J
)
SET BUILDTOOLSROOT=%BUILDROOT%\..\%BUILDTOOLSNAME%

IF NOT EXIST %BUILDTOOLSROOT%\win_build\build.cmd (
    ECHO Software NOT Detected: Build Tools...
    EXIT /B 2
)

:BUILDTOOLSROOTVALIDATED
FOR /F %%I IN ("%BUILDTOOLSROOT%") DO SET BUILDTOOLSROOT=%%~fI


rem ***** Reset Variables to start out with a clean environment *****
rem 
SET PATH=%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\System32\Wbem
SET INCLUDE=
SET LIB=
SET LIBPATH=

rem Which build environment should we use?
IF /I NOT "%PROCESSOR_ARCHITECTURE%"=="%BUILDPLATFORM%" GOTO :BUILDENVCROSS

rem Building a native binary for the current platform.
:BUILDENVNATIVE

IF /I NOT "%BUILDPLATFORM%"=="x86" (
    CALL "%VCINSTALLDIR%\bin\%PROCESSOR_ARCHITECTURE%\vcvars%PROCESSOR_ARCHITECTURE%.bat" > NUL: 2> NUL:
    IF ERRORLEVEL 1 GOTO :BUILDENVFAILURE
) ELSE (
    CALL "%VCINSTALLDIR%\bin\vcvars32.bat" > NUL: 2> NUL:
    IF ERRORLEVEL 1 GOTO :BUILDENVFAILURE
)

GOTO :BUILDENVDONE

rem Building a binary for another platform.
:BUILDENVCROSS

IF /I NOT "%BUILDPLATFORM%"=="x86" (
    CALL "%VCINSTALLDIR%\bin\%PROCESSOR_ARCHITECTURE%_%BUILDPLATFORM%\vcvars%PROCESSOR_ARCHITECTURE%_%BUILDPLATFORM%.bat" > NUL: 2> NUL:
    IF ERRORLEVEL 1 GOTO :BUILDENVFAILURE
) ELSE (
    CALL "%VCINSTALLDIR%\bin\vcvars32.bat" > NUL: 2> NUL:
    IF ERRORLEVEL 1 GOTO :BUILDENVFAILURE
)

:BUILDENVDONE

rem Convert the OS Platform into a Visual Studio compatible platform
IF /I "%BUILDPLATFORM%"=="x86"   SET VSPLATFORM=Win32
IF /I "%BUILDPLATFORM%"=="amd64" SET VSPLATFORM=x64
IF /I "%BUILDPLATFORM%"=="ia64"  SET VSPLATFORM=Itanium


rem ***** Standard Developer Environment *****
rem

rem Add standard shell scripts executables to the path
SET PATH=%BUILDROOT%\win_build;%BUILDROOT%\win_build\%PROCESSOR_ARCHITECTURE%;%PATH%
SET PATH=%BUILDTOOLSROOT%\win_build;%BUILDTOOLSROOT%\win_build\%PROCESSOR_ARCHITECTURE%;%PATH%

rem Add Perl to the path
SET PATH=%BUILDTOOLSROOT%\win_build\perl;%BUILDTOOLSROOT%\win_build\perl\%PROCESSOR_ARCHITECTURE%\bin;%PATH%

rem Add subversion source control to the path
SET SVN_EDITOR=notepad.exe
SET PATH=%BUILDTOOLSROOT%\win_build\subversion;%BUILDTOOLSROOT%\win_build\subversion\%PROCESSOR_ARCHITECTURE%\bin;%PATH%

rem Specify build output directory
SET BUILDOUTPUT=%BUILDROOT%\win_build\build\%VSPLATFORM%\%BUILDTYPE%

rem Add source server environment variables
SET SRCSRV_SYSTEM=svn
SET SRCSRV_INI=%BUILDTOOLSROOT%\win_build\srcsrv.ini
SET SRCSRV_SYMBOLS=%BUILDROOT%\win_build\build\%VSPLATFORM%\%BUILDTYPE%
SET SRCSRV_SOURCE=%BUILDROOT%

rem Add deployable symbol store environment variables
SET BUILDSYMBOLSTORE=%BUILDTOOLSROOT%\Developr\%USERNAME%\symstore

rem Add _NT_SYMBOL_PATH to the environment
SET _NT_SYMBOL_PATH=%BUILDSYMBOLSTORE%
SET _NT_SYMBOL_PATH=%_NT_SYMBOL_PATH%;srv*%BUILDTOOLSROOT%\Developr\%USERNAME%\symbols*http://msdl.microsoft.com/download/symbols
SET _NT_SYMBOL_PATH=%_NT_SYMBOL_PATH%;srv*%BUILDTOOLSROOT%\Developr\%USERNAME%\symbols*http://boinc.berkeley.edu/symstore/

rem Add standard macros to the environment
DOSKEY /MACROFILE="%BUILDTOOLSROOT%\developr\generic.mac"

rem Add directory traversal macros to the environment
DOSKEY /MACROFILE="%BUILDTOOLSROOT%\developr\tree.mac"

rem Populate environmental version information
FOR /F "usebackq tokens=1,2,3 delims=." %%I IN (`TYPE "%BUILDROOT%\version.log"`) DO (
    SET BUILDVERSIONMAJOR=%%I
    SET BUILDVERSIONMINOR=%%J
    SET BUILDVERSIONREVISION=%%K
)

rem Adjust command processor window title
TITLE BOINC Build Environment for Windows %BUILDVERSIONMAJOR%.%BUILDVERSIONMINOR% (%BUILDTYPE%/%BUILDPLATFORM%)

rem ***** Customized Developer Environment *****
rem
SET BUILDHOMEDIR=%BUILDTOOLSROOT%\Developr\%USERNAME%
IF NOT EXIST %BUILDHOMEDIR% MKDIR %BUILDHOMEDIR%

rem Create a default directory layout
IF NOT EXIST %BUILDHOMEDIR%\bin mkdir %BUILDHOMEDIR%\bin
IF NOT EXIST %BUILDHOMEDIR%\bin\%PROCESSOR_ARCHITECTURE% mkdir %BUILDHOMEDIR%\bin\%PROCESSOR_ARCHITECTURE%
IF NOT EXIST %BUILDHOMEDIR%\temp mkdir %BUILDHOMEDIR%\temp
IF NOT EXIST %BUILDHOMEDIR%\symbols mkdir %BUILDHOMEDIR%\symbols
IF NOT EXIST %BUILDHOMEDIR%\symstore mkdir %BUILDHOMEDIR%\symstore

rem Add custom shell scripts to the path
SET PATH=%BUILDHOMEDIR%\bin;%BUILDHOMEDIR%\bin\%PROCESSOR_ARCHITECTURE%;%PATH%

rem Add custom macros to the environment
IF EXIST %BUILDHOMEDIR%\custom.mac (
	DOSKEY /MACROFILE=%BUILDHOMEDIR%\custom.mac
)

rem Execute a custom shell script as defined by the developer
IF EXIST %BUILDHOMEDIR%\customenv.cmd (
	CALL %BUILDHOMEDIR%\customenv.cmd
)


rem ***** Execute Command Line Script *****
rem
IF /I NOT "%_ArgExec%"=="" (
    ECHO Executing: %_ArgExec% %_ArgExecParam%
    ECHO.
    CALL %_ArgExec% %_ArgExecParam%
    IF ERRORLEVEL 1 EXIT /B 10
)


rem ***** Cleanup all the temporary variables *****
rem 
FOR /F "delims==" %%I IN ('SET _Arg') DO SET %%I=


rem ***** All Done *****
rem

GOTO :EOF

:BUILDENVFAILURE

ECHO Failed to setup the requested build environment.
ECHO   Commandline: %*
ECHO.

:USAGE

ECHO Usage: buildenv.cmd TYPE ^<type^> PLATFORM ^<platform^> [Optional Commands]
ECHO.
ECHO   Commands:
ECHO     DEVENVDIR:  Which build environment executed this batch file.
ECHO     TYPE:  Which build environment are you building executables for.
ECHO       Current Values: Release/Debug
ECHO     PLATFORM: Which platform are you building for.
ECHO       Current Values: x86/amd64/ia64
ECHO.

GOTO :EOF
