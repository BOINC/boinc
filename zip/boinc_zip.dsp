# Microsoft Developer Studio Project File - Name="boinc_zip" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=boinc_zip - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "boinc_zip.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "boinc_zip.mak" CFG="boinc_zip - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "boinc_zip - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "boinc_zip - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
F90=df.exe
RSC=rc.exe

!IF  "$(CFG)" == "boinc_zip - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE F90 /compile_only /nologo /warn:nofileopt
# ADD F90 /compile_only /nologo /threads /warn:nofileopt
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\lib" /I "..\\" /I ".\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"boinc_zip.lib"

!ELSEIF  "$(CFG)" == "boinc_zip - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE F90 /check:bounds /compile_only /dbglibs /debug:full /nologo /traceback /warn:argument_checking /warn:nofileopt
# ADD F90 /check:bounds /compile_only /dbglibs /debug:full /nologo /threads /traceback /warn:argument_checking /warn:nofileopt
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\lib" /I "..\\" /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"boinc_zipd.lib"

!ENDIF 

# Begin Target

# Name "boinc_zip - Win32 Release"
# Name "boinc_zip - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;f90;for;f;fpp"
# Begin Source File

SOURCE=.\boinc_zip.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\boinc_zip.h
# End Source File
# End Group
# Begin Group "Unzip Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\unzip\consts.h
# End Source File
# Begin Source File

SOURCE=.\unzip\ebcdic.h
# End Source File
# Begin Source File

SOURCE=.\unzip\globals.h
# End Source File
# Begin Source File

SOURCE=.\unzip\inflate.h
# End Source File
# Begin Source File

SOURCE=.\unzip\tables.h
# End Source File
# Begin Source File

SOURCE=.\unzip\ttyio.h
# End Source File
# Begin Source File

SOURCE=.\unzip\unzip.h
# End Source File
# Begin Source File

SOURCE=.\unzip\unzpriv.h
# End Source File
# Begin Source File

SOURCE=.\unzip\unzvers.h
# End Source File
# Begin Source File

SOURCE=.\unzip\win32\w32cfg.h
# End Source File
# End Group
# Begin Group "Unzip Source Files"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\unzip\apihelp.c
# End Source File
# Begin Source File

SOURCE=.\unzip\crc32.c
# End Source File
# Begin Source File

SOURCE=.\unzip\crctab.c
# End Source File
# Begin Source File

SOURCE=.\unzip\envargs.c
# End Source File
# Begin Source File

SOURCE=.\unzip\explode.c
# End Source File
# Begin Source File

SOURCE=.\unzip\extract.c
# End Source File
# Begin Source File

SOURCE=.\unzip\fileio.c
# End Source File
# Begin Source File

SOURCE=.\unzip\globals.c
# End Source File
# Begin Source File

SOURCE=.\unzip\inflate.c
# End Source File
# Begin Source File

SOURCE=.\unzip\list.c
# End Source File
# Begin Source File

SOURCE=.\unzip\match.c
# End Source File
# Begin Source File

SOURCE=.\unzip\process.c
# End Source File
# Begin Source File

SOURCE=.\unzip\ttyio.c
# End Source File
# Begin Source File

SOURCE=.\unzip\unreduce.c
# End Source File
# Begin Source File

SOURCE=.\unzip\unshrink.c
# End Source File
# Begin Source File

SOURCE=.\unzip\unzip.c
# End Source File
# Begin Source File

SOURCE=.\unzip\win32\win32.c
# End Source File
# Begin Source File

SOURCE=.\unzip\zipinfo.c
# End Source File
# End Group
# Begin Group "Zip Header Files"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\zip\win32\osdep.h
# End Source File
# Begin Source File

SOURCE=.\zip\revision.h
# End Source File
# Begin Source File

SOURCE=.\zip\tailor.h
# End Source File
# Begin Source File

SOURCE=.\zip\z_ttyio.h
# End Source File
# Begin Source File

SOURCE=.\zip\zip.h
# End Source File
# Begin Source File

SOURCE=.\zip\ziperr.h
# End Source File
# End Group
# Begin Group "Zip Source Files"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\zip\deflate.c
# End Source File
# Begin Source File

SOURCE=.\zip\mktime.c
# End Source File
# Begin Source File

SOURCE=.\zip\trees.c
# End Source File
# Begin Source File

SOURCE=.\zip\util.c
# End Source File
# Begin Source File

SOURCE=.\zip\win32\win32zip.c
# End Source File
# Begin Source File

SOURCE=.\zip\z_fileio.c
# End Source File
# Begin Source File

SOURCE=.\zip\z_globals.c
# End Source File
# Begin Source File

SOURCE=.\zip\win32\z_win32.c
# End Source File
# Begin Source File

SOURCE=.\zip\zip.c
# End Source File
# Begin Source File

SOURCE=.\zip\zipfile.c
# End Source File
# Begin Source File

SOURCE=.\zip\zipup.c
# End Source File
# End Group
# End Target
# End Project
