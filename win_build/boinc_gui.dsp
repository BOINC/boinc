# Microsoft Developer Studio Project File - Name="boinc_gui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=boinc_gui - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "boinc_gui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "boinc_gui.mak" CFG="boinc_gui - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "boinc_gui - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "boinc_gui - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "boinc_gui - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "../lib/" /I "../api/" /I "../RSAEuro/source/" /I "../client/win/" /I "../client" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c /Tp
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib wininet.lib winmm.lib nafxcwd.lib libcmtd.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"nafxcwd" /nodefaultlib:"libcmtd"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "boinc_gui - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug\"
# PROP Intermediate_Dir "Build\Debug\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../lib/" /I "../api/" /I "../RSAEuro/source/" /I "../client/win/" /I "../client" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c /Tp
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib wininet.lib winmm.lib nafxcwd.lib libcmtd.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /nodefaultlib:"nafxcwd" /nodefaultlib:"libcmtd" /pdbtype:sept /libpath:"..\boinc_dll\Debug"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "boinc_gui - Win32 Release"
# Name "boinc_gui - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Client\app.C
# End Source File
# Begin Source File

SOURCE=..\lib\app_ipc.C
# End Source File
# Begin Source File

SOURCE=..\client\client_messages.C
# End Source File
# Begin Source File

SOURCE=..\Client\client_state.C
# End Source File
# Begin Source File

SOURCE=..\Client\client_types.C
# End Source File
# Begin Source File

SOURCE=..\client\cpu_benchmark.C
# End Source File
# Begin Source File

SOURCE=..\Lib\Crypt.c
# End Source File
# Begin Source File

SOURCE=..\client\cs_account.C
# End Source File
# Begin Source File

SOURCE=..\Client\cs_apps.C
# End Source File
# Begin Source File

SOURCE=..\client\cs_benchmark.C
# End Source File
# Begin Source File

SOURCE=..\client\cs_cmdline.C
# End Source File
# Begin Source File

SOURCE=..\Client\cs_files.C
# End Source File
# Begin Source File

SOURCE=..\client\cs_prefs.C
# End Source File
# Begin Source File

SOURCE=..\Client\cs_scheduler.C
# End Source File
# Begin Source File

SOURCE=..\client\cs_statefile.C
# End Source File
# Begin Source File

SOURCE=..\Client\file_names.C
# End Source File
# Begin Source File

SOURCE=..\Client\file_xfer.C
# End Source File
# Begin Source File

SOURCE=..\lib\filesys.C
# End Source File
# Begin Source File

SOURCE=..\client\gui_titles.C
# End Source File
# Begin Source File

SOURCE=..\Client\Hostinfo.c
# End Source File
# Begin Source File

SOURCE=..\Client\win\hostinfo_win.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\http.C
# End Source File
# Begin Source File

SOURCE=..\lib\language.C
# End Source File
# Begin Source File

SOURCE=..\Client\log_flags.C
# End Source File
# Begin Source File

SOURCE=..\Lib\Md5.c
# End Source File
# Begin Source File

SOURCE=..\Lib\Md5_file.c
# End Source File
# Begin Source File

SOURCE=..\client\message.C
# End Source File
# Begin Source File

SOURCE=..\lib\messages.C
# End Source File
# Begin Source File

SOURCE=..\Client\net_stats.C
# End Source File
# Begin Source File

SOURCE=..\Client\net_xfer.C
# End Source File
# Begin Source File

SOURCE=..\Lib\parse.C
# End Source File
# Begin Source File

SOURCE=..\Client\pers_file_xfer.C
# End Source File
# Begin Source File

SOURCE=..\Client\Prefs.c
# End Source File
# Begin Source File

SOURCE=..\client\win\resource.rc
# End Source File
# Begin Source File

SOURCE=..\Client\scheduler_op.C
# End Source File
# Begin Source File

SOURCE=..\lib\shmem.C
# End Source File
# Begin Source File

SOURCE=..\client\ss_logic.C
# End Source File
# Begin Source File

SOURCE=..\client\win\Stackwalker.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\time_stats.C
# End Source File
# Begin Source File

SOURCE=..\lib\util.C
# End Source File
# Begin Source File

SOURCE=..\Client\Version.c
# End Source File
# Begin Source File

SOURCE=..\Client\win\Win_main.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\win\Win_net.cpp
# End Source File
# Begin Source File

SOURCE=..\client\win\win_util.cpp
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui.cpp
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_dialog.cpp
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_listctrl.cpp
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_mainwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_piectrl.cpp
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_sswindow.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\client\app.h
# End Source File
# Begin Source File

SOURCE=..\lib\app_ipc.h
# End Source File
# Begin Source File

SOURCE=..\client\client_state.h
# End Source File
# Begin Source File

SOURCE=..\client\client_types.h
# End Source File
# Begin Source File

SOURCE=..\client\cpp.h
# End Source File
# Begin Source File

SOURCE=..\client\cpu_benchmark.h
# End Source File
# Begin Source File

SOURCE=..\lib\error_numbers.h
# End Source File
# Begin Source File

SOURCE=..\client\file_names.h
# End Source File
# Begin Source File

SOURCE=..\client\file_xfer.h
# End Source File
# Begin Source File

SOURCE=..\lib\filesys.h
# End Source File
# Begin Source File

SOURCE=..\client\gui_titles.h
# End Source File
# Begin Source File

SOURCE=..\client\hostinfo.h
# End Source File
# Begin Source File

SOURCE=..\client\http.h
# End Source File
# Begin Source File

SOURCE=..\lib\language.h
# End Source File
# Begin Source File

SOURCE=..\client\log_flags.h
# End Source File
# Begin Source File

SOURCE=..\lib\md5.h
# End Source File
# Begin Source File

SOURCE=..\lib\md5_file.h
# End Source File
# Begin Source File

SOURCE=..\client\message.h
# End Source File
# Begin Source File

SOURCE=..\client\net_stats.h
# End Source File
# Begin Source File

SOURCE=..\client\net_xfer.h
# End Source File
# Begin Source File

SOURCE=..\lib\parse.h
# End Source File
# Begin Source File

SOURCE=..\Client\pers_file_xfer.h
# End Source File
# Begin Source File

SOURCE=..\client\prefs.h
# End Source File
# Begin Source File

SOURCE=..\client\win\Resource.h
# End Source File
# Begin Source File

SOURCE=..\client\scheduler_op.h
# End Source File
# Begin Source File

SOURCE=..\lib\shmem.h
# End Source File
# Begin Source File

SOURCE=..\client\ss_logic.h
# End Source File
# Begin Source File

SOURCE=..\client\win\Stackwalker.h
# End Source File
# Begin Source File

SOURCE=..\client\time_stats.h
# End Source File
# Begin Source File

SOURCE=..\lib\util.h
# End Source File
# Begin Source File

SOURCE=..\client\version.h
# End Source File
# Begin Source File

SOURCE=.\win_dialog.h
# End Source File
# Begin Source File

SOURCE=..\Client\win\Win_net.h
# End Source File
# Begin Source File

SOURCE=..\client\win\win_util.h
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui.h
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_dialog.h
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_listctrl.h
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_mainwindow.h
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_piectrl.h
# End Source File
# Begin Source File

SOURCE=..\client\win\wingui_sswindow.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\client\win\boinc.bmp
# End Source File
# Begin Source File

SOURCE=..\client\win\res\boinc.bmp
# End Source File
# Begin Source File

SOURCE=..\client\win\boincsm.bmp
# End Source File
# Begin Source File

SOURCE=..\client\win\res\boincsm.bmp
# End Source File
# Begin Source File

SOURCE=..\res\CoreClient.ico
# End Source File
# Begin Source File

SOURCE=..\client\win\res\icon.ico
# End Source File
# Begin Source File

SOURCE=..\client\win\res\iconhigh.ico
# End Source File
# Begin Source File

SOURCE=..\client\win\res\iconsmall.ico
# End Source File
# Begin Source File

SOURCE=..\client\win\res\mess.bmp
# End Source File
# Begin Source File

SOURCE=..\client\win\res\proj.bmp
# End Source File
# Begin Source File

SOURCE=..\client\win\res\result.bmp
# End Source File
# Begin Source File

SOURCE=..\client\win\res\small.ico
# End Source File
# Begin Source File

SOURCE=..\client\win\res\smallico.ico
# End Source File
# Begin Source File

SOURCE=..\client\win\res\xfer.bmp
# End Source File
# End Group
# End Target
# End Project
