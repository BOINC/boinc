# Microsoft Developer Studio Project File - Name="boinc_cli" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=boinc_cli - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "boinc_cli.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "boinc_cli.mak" CFG="boinc_cli - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "boinc_cli - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "boinc_cli - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "boinc_cli - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "../../lib/" /I "../../api/" /I "../../RSAEuro/source/" /I "../../client/win/" /I "../../client" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D MAJOR_VERSION=1 /D MINOR_VERSION=00 /D "WIN_CLI" /YX /FD /c /Tp
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib wininet.lib winmm.lib nafxcwd.lib libcmtd.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"nafxcwd" /nodefaultlib:"libcmtd" /nodefaultlib:"libcmt" /nodefaultlib:"libcd"

!ELSEIF  "$(CFG)" == "boinc_cli - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MT /W3 /GX /ZI /Od /I "../../lib/" /I "../../api/" /I "../../RSAEuro/source/" /I "../../client/win/" /I "../../client" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D MAJOR_VERSION=1 /D MINOR_VERSION=00 /D "WIN_CLI" /FR /YX /FD /GZ /c /Tp
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib wininet.lib winmm.lib nafxcwd.lib libcmtd.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"nafxcwd" /nodefaultlib:"libcmtd" /nodefaultlib:"libcmt" /nodefaultlib:"libcd" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "boinc_cli - Win32 Release"
# Name "boinc_cli - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\client\account.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\app.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\app_ipc.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\client_state.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\client_types.C
# End Source File
# Begin Source File

SOURCE=..\..\Lib\Crypt.c
# End Source File
# Begin Source File

SOURCE=..\..\Client\cs_apps.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\cs_files.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\cs_scheduler.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\file_names.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\file_xfer.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\filesys.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\Hostinfo.c
# End Source File
# Begin Source File

SOURCE=..\..\Client\win\hostinfo_win.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Client\http.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\log_flags.C
# End Source File
# Begin Source File

SOURCE=..\..\client\main.C
# End Source File
# Begin Source File

SOURCE=..\..\Lib\Md5.c
# End Source File
# Begin Source File

SOURCE=..\..\Lib\Md5_file.c
# End Source File
# Begin Source File

SOURCE=..\..\Client\net_stats.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\net_xfer.C
# End Source File
# Begin Source File

SOURCE=..\..\Lib\parse.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\pers_file_xfer.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\Prefs.c
# End Source File
# Begin Source File

SOURCE=..\..\Client\scheduler_op.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\shmem.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\speed_stats.C
# End Source File
# Begin Source File

SOURCE=..\..\client\ss_logic.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\time_stats.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\util.C
# End Source File
# Begin Source File

SOURCE=..\..\Client\Version.c
# End Source File
# Begin Source File

SOURCE=..\..\Client\win\Win_main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Client\win\Win_net.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\client\app.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\app_ipc.h
# End Source File
# Begin Source File

SOURCE=..\..\client\client_state.h
# End Source File
# Begin Source File

SOURCE=..\..\client\client_types.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\error_numbers.h
# End Source File
# Begin Source File

SOURCE=..\..\client\file_names.h
# End Source File
# Begin Source File

SOURCE=..\..\client\file_xfer.h
# End Source File
# Begin Source File

SOURCE=..\..\client\filesys.h
# End Source File
# Begin Source File

SOURCE=..\..\client\hostinfo.h
# End Source File
# Begin Source File

SOURCE=..\..\client\http.h
# End Source File
# Begin Source File

SOURCE=..\..\client\log_flags.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\md5_file.h
# End Source File
# Begin Source File

SOURCE=..\..\client\message.h
# End Source File
# Begin Source File

SOURCE=..\..\client\net_stats.h
# End Source File
# Begin Source File

SOURCE=..\..\client\net_xfer.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\parse.h
# End Source File
# Begin Source File

SOURCE=..\..\Client\pers_file_xfer.h
# End Source File
# Begin Source File

SOURCE=..\..\client\prefs.h
# End Source File
# Begin Source File

SOURCE=..\..\client\scheduler_op.h
# End Source File
# Begin Source File

SOURCE=..\..\client\speed_stats.h
# End Source File
# Begin Source File

SOURCE=..\..\client\ss_logic.h
# End Source File
# Begin Source File

SOURCE=..\..\client\time_stats.h
# End Source File
# Begin Source File

SOURCE=..\..\client\util.h
# End Source File
# Begin Source File

SOURCE=..\..\client\version.h
# End Source File
# Begin Source File

SOURCE=..\..\Client\win\Win_net.h
# End Source File
# Begin Source File

SOURCE=..\..\Client\win\windows_cpp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\RSAEuro\Release\RSAEuro.lib
# End Source File
# End Group
# End Target
# End Project
