# Microsoft Developer Studio Project File - Name="RSAEuro" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=RSAEuro - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RSAEuro.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RSAEuro.mak" CFG="RSAEuro - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RSAEuro - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "RSAEuro - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RSAEuro - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RSAEuro_Release"
# PROP Intermediate_Dir "RSAEuro_Release\objs"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "RSAEuro - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "RSAEuro_Debug"
# PROP Intermediate_Dir "RSAEuro_Debug\objs"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "RSAEuro - Win32 Release"
# Name "RSAEuro - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\RSAEuro\source\desc.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\md2c.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\md4c.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\md5c.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\nn.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\prime.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\r_dh.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\r_encode.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\r_enhanc.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\r_keygen.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\r_random.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\r_stdlib.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\rsa.c
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\shsc.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\RSAEuro\source\des.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\global.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\md2.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\md4.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\md5.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\nn.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\prime.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\r_random.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\rsa.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\rsaeuro.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\rsaref.h
# End Source File
# Begin Source File

SOURCE=..\RSAEuro\source\shs.h
# End Source File
# End Group
# End Target
# End Project
