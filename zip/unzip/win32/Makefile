# NMAKE Makefile for Windows NT/Windows 95
#   D. Feinleib 7/92 <t-davefe@microsoft.com>
#   H. Gessau 9/93 <henryg@kullmar.kullmar.se>
#   J. Lee 8/95 (johnnyl@microsoft.com)
#   C. Spieler 03/99
#
# Last revised:  10 Feb 2001
#
# Tested with VC++ 2.0 for NT for MIPS and Alpha, Visual C++ 2.2 for Intel CPUs
# Revision tested with VC++ 5.0 and 6.0 for Intel CPUs
#

# Nmake macros for building Windows NT applications
# To build with debug info use 'nmake debug=1'
!IFNDEF debug
NODEBUG=1
!ENDIF

!ifdef NOASM
APPLY_ASMCRC=0
!else
!ifdef USEASM
APPLY_ASMCRC=1
!endif
!endif

!ifndef APPLY_ASMCRC
#default to generic C code for now...
APPLY_ASMCRC=0
!endif

!ifndef USE_IZTIMEZONE
#default: do not use the IZ timezone replacement
USE_IZTIMEZONE=0
!endif

!IF "$(CPU)" == ""  &&  "$(PROCESSOR_ARCHITECTURE)" == "x86"
CPU = i386
!ENDIF
!IF "$(CPU)" == ""  &&  "$(PROCESSOR_ARCHITECTURE)" == "MIPS"
CPU = $(PROCESSOR_ARCHITECTURE)
!ENDIF
!IF "$(CPU)" == ""  &&  "$(PROCESSOR_ARCHITECTURE)" == "ALPHA"
CPU = $(PROCESSOR_ARCHITECTURE)
!ENDIF
!IF "$(CPU)" == ""  &&  "$(PROCESSOR_ARCHITECTURE)" == "PPC"
CPU = $(PROCESSOR_ARCHITECTURE)
!ENDIF
!IF "$(CPU)" == ""
CPU = i386
!ENDIF

!include <ntwin32.mak>

# special subdirectory for the static library binaries, to separate them
# from other binaries (including dll import library file)
OBDIR = ow32lib
OB = $(OBDIR)/

# object files
OBJU1 = unzip.obj $(CRC32).obj crctab.obj crypt.obj envargs.obj explode.obj
OBJU2 = extract.obj fileio.obj globals.obj inflate.obj list.obj match.obj
OBJU3 = process.obj ttyio.obj unreduce.obj unshrink.obj zipinfo.obj
OBJUS = win32.obj nt.obj $(TIMEZONE_OBJU)
OBJU  = $(OBJU1) $(OBJU2) $(OBJU3) $(OBJUS)
OBJX1 = unzipsfx.obj $(CRC32X).obj crctab_.obj crypt_.obj
OBJX2 = extract_.obj fileio_.obj globals_.obj inflate_.obj
OBJX3 = match_.obj process_.obj ttyio_.obj
OBJXS = win32_.obj nt_.obj $(TIMEZONE_OBJX)
OBJX  = $(OBJX1) $(OBJX2) $(OBJX3) $(OBJXS)
OBJF1 = funzip.obj $(CRC32F).obj cryptf.obj globalsf.obj
OBJF2 = inflatef.obj ttyiof.obj
OBJFS = win32f.obj
OBJF  = $(OBJF1) $(OBJF2) $(OBJFS)
OBJD1 = api.obj $(CRC32L).obj crctabl.obj cryptl.obj explodel.obj
OBJD2 = extractl.obj fileiol.obj globalsl.obj inflatel.obj
OBJD3 = listl.obj matchl.obj processl.obj
OBJD4 = unreducl.obj unshrnkl.obj zipinfol.obj
OBJDS = win32l.obj ntl.obj windll.obj $(TIMEZONE_OBJD)
OBJD  = $(OBJD1) $(OBJD2) $(OBJD3) $(OBJD4) $(OBJDS)
OBLX1 = apig.obj $(CRC32G).obj crctabg.obj cryptg.obj
OBLX2 = extractg.obj fileiog.obj globalsg.obj inflateg.obj
OBLX3 = matchg.obj processg.obj
OBLXS = win32g.obj ntg.obj windllg.obj $(TIMEZONE_OBLX)
OBLX  = $(OBLX1) $(OBLX2) $(OBLX3) $(OBLXS)
OBGX  = sfxwiz.obj
OBJB1 = $(OB)apib.obj $(OB)$(CRC32B).obj $(OB)crctabb.obj $(OB)cryptb.obj
OBJB2 = $(OB)explodeb.obj $(OB)extractb.obj $(OB)fileiob.obj $(OB)globalsb.obj
OBJB3 = $(OB)inflateb.obj $(OB)listb.obj $(OB)matchb.obj $(OB)processb.obj
OBJB4 = $(OB)unreducb.obj $(OB)unshrnkb.obj $(OB)zipinfob.obj
OBJBS = $(OB)win32b.obj $(OB)ntb.obj $(OB)windllb.obj $(TIMEZONE_OBJB)
OBJB  = $(OBJB1) $(OBJB2) $(OBJB3) $(OBJB4) $(OBJBS)

UNZIP_H = unzip.h unzpriv.h globals.h win32/w32cfg.h
WINDLL_H = windll/windll.h windll/decs.h windll/structs.h
WINDLL_DEF = windll/windll32.def
WINDLL_IMP_H = windll/decs.h windll/structs.h

!if $(APPLY_ASMCRC) != 0
CFVARS_ASM = -DASM_CRC
!else
CFVARS_ASM = -DNO_ASM
!endif

!if $(USE_IZTIMEZONE) != 0
LOCAL_UNZIP = -DW32_USE_IZ_TIMEZONE $(LOCAL_UNZIP)
!endif

#CF_LOC = -nologo -J -D_MBCS $(CFVARS_ASM) -DREENTRANT
CF_LOC = -nologo -J -D_MBCS $(CFVARS_ASM) $(LOCAL_UNZIP)

cvars = $(cvars) $(CF_LOC)

!if $(APPLY_ASMCRC) != 0
CRC32 = crc_i386
CRC32X = $(CRC32)
CRC32F = $(CRC32)
CRC32L = crci386l
CRC32G = crci386g
CRC32B = crci386b
!else
CRC32 = crc32
CRC32X = crc32_
CRC32F = $(CRC32)
CRC32L = crc32l
CRC32G = crc32g
CRC32B = crc32b
!endif

!if $(USE_IZTIMEZONE) != 0
TIMEZONE_OBJU = timezone.obj
TIMEZONE_OBJX = timezon_.obj
TIMEZONE_OBJD = timezonl.obj
TIMEZONE_OBLX = timezolx.obj
TIMEZONE_OBJB = $(OB)timezonb.obj
!else
TIMEZONE_OBJU =
TIMEZONE_OBJX =
TIMEZONE_OBJD =
TIMEZONE_OBLX =
TIMEZONE_OBJB =
!endif

# USER32.LIB is needed for OemToChar and related routines
conlibs=$(conlibs) user32.lib
conlibsmt=$(conlibsmt) user32.lib
conlibsdll=$(conlibsdll) user32.lib

# Some optimization (if not debugging)
!IFDEF NODEBUG
cdebug = -O2
cdbgsz = -O1
!ELSE
cdbgsz = $(cdebug)
!ENDIF

!IFDEF PROFILEINFO
ldebug = $(ldebug) /PROFILE
!endif

!IFDEF DLLSTANDALONE
# the standalone variant of unzip32.dll is linked against the
# multithread-safe static C runtime library
cvars_izdll = $(cvarsmt)
conlibs_izdll = $(conlibsmt)
!ELSE
# the (normal) variant of unzip32.dll is linked against the import library
# of multithread-safe C runtime dll
# -> smaller dll size; efficient use of shared resources,
#    but requires the C RTL DLL msvcrt.dll to be installed at runtime
cvars_izdll = $(cvarsdll)
conlibs_izdll = $(conlibsdll)
!ENDIF

CFLAGS_UN = $(cdebug) $(cflags) $(cvars)
CFLAGS_SX = $(cdbgsz) $(cflags) $(cvars) -DSFX
CFLAGS_FU = $(cdebug) $(cflags) $(cvars) -DFUNZIP
CFLAGS_DL = $(cdebug) $(cflags) $(cvars_izdll) -DWINDLL -DDLL
CFLAGS_SL = $(cdbgsz) $(cflags) $(cvars) -DWINDLL -DDLL -DUNZIPLIB -DSFX
CFLAGS_GX = $(cdbgsz) $(cflags) $(cvars) -DSFX
CFLAGS_LB = $(cdebug) $(cflags) $(cvars) -DWINDLL -DDLL -DUNZIPLIB

# How to compile sources
.c.obj:
	$(cc) $(CFLAGS_UN) -o $@ $<

# How to link
.obj.exe:
   $(link) $(ldebug) $(conflags) $(conlibs) $** -out:$@
.obj.dll:
   $(link) $(ldebug) $(dlllflags) $(conlibs_izdll) $** -out:$@
.obj.lib:
   $(link) -lib $** -out:$@


# Default target is all the executables
unzips : unzip.exe funzip.exe unzipsfx.exe
guisfx: SFXWiz32.exe
dll : unzip32.dll
dllsample : uzexampl.exe
lib : $(OBDIR) $(OB)unzip32.lib
all : unzips guisfx dll dllsample lib


unzip.exe:	$(OBJU)
unzipsfx.exe:	$(OBJX)
funzip.exe:	$(OBJF)
unzip32.dll:	$(OBJD) windll.res $(WINDLL_DEF)
	$(link) $(ldebug) $(dlllflags) $(conlibs_izdll) -def:$(WINDLL_DEF) \
		$(OBJD) windll.res -out:$@
unzsfx32.lib:	$(OBLX)
	$(link) -lib $(OBLX) -out:$@
SFXWiz32.exe:	$(OBGX) sfxwiz.res unzsfx32.lib
	$(link) $(ldebug) $(guilflags) $(guilibs) \
		$(OBGX) sfxwiz.res unzsfx32.lib -out:$@
uzexampl.exe:	uzexampl.obj
	$(link) $(ldebug) $(conflags) $(conlibs) version.lib $** -out:$@
$(OB)unzip32.lib:	$(OBJB)
	$(link) -lib $(OBJB) -out:$@

crc32.obj:	crc32.c $(UNZIP_H) zip.h
crctab.obj:	crctab.c $(UNZIP_H) zip.h
crypt.obj:	crypt.c $(UNZIP_H) zip.h crypt.h ttyio.h
envargs.obj:	envargs.c $(UNZIP_H)
explode.obj:	explode.c $(UNZIP_H)
extract.obj:	extract.c $(UNZIP_H) crypt.h
fileio.obj:	fileio.c $(UNZIP_H) crypt.h ttyio.h ebcdic.h
funzip.obj:	funzip.c $(UNZIP_H) crypt.h ttyio.h tables.h
globals.obj:	globals.c $(UNZIP_H)
inflate.obj:	inflate.c inflate.h $(UNZIP_H)
list.obj:	list.c $(UNZIP_H)
match.obj:	match.c $(UNZIP_H)
process.obj:	process.c $(UNZIP_H)
timezone.obj:	timezone.c $(UNZIP_H) zip.h timezone.h
ttyio.obj:	ttyio.c $(UNZIP_H) zip.h crypt.h ttyio.h
unreduce.obj:	unreduce.c $(UNZIP_H)
unshrink.obj:	unshrink.c $(UNZIP_H)
unzip.obj:	unzip.c $(UNZIP_H) crypt.h unzvers.h consts.h
zipinfo.obj:	zipinfo.c $(UNZIP_H)

win32.obj:	win32/win32.c $(UNZIP_H) win32/nt.h
	$(cc) $(CFLAGS_UN) -I. win32/win32.c -Fo$@

nt.obj:		win32/nt.c $(UNZIP_H) win32/nt.h
	$(cc) $(CFLAGS_UN) -I. win32/nt.c -Fo$@

!ifdef USEMASM
crc_i386.obj:	win32/crc_i386.asm
	masm -ml win32/crc_i386.asm,$@;
!else
!ifdef USEML
crc_i386.obj:	win32/crc_i386.asm
	ml -c -Cp -coff -Fo$@ win32/crc_i386.asm
!else
crc_i386.obj:	win32/crc_i386.c
	$(cc) $(CFLAGS_UN) -I. win32/crc_i386.c -Fo$@
!endif
!endif

win32_.obj:	win32/win32.c $(UNZIP_H) win32/nt.h		# unzipsfx only
	$(cc) $(CFLAGS_SX) -I. win32/win32.c -Fo$@

win32f.obj:	win32/win32.c $(UNZIP_H) win32/nt.h		# funzip only
	$(cc) $(CFLAGS_FU) -I. win32/win32.c -Fo$@

nt_.obj:	win32/nt.c $(UNZIP_H) win32/nt.h		# unzipsfx only
	$(cc) $(CFLAGS_SX) -I. win32/nt.c -Fo$@

crc32_.obj:	crc32.c $(UNZIP_H) zip.h			# unzipsfx only
	$(cc) $(CFLAGS_SX) crc32.c -Fo$@

crc32f.obj:	crc32.c $(UNZIP_H) zip.h			# funzip only
	$(cc) $(CFLAGS_FU) crc32.c -Fo$@

crctab_.obj:	crctab.c $(UNZIP_H) zip.h			# unzipsfx only
	$(cc) $(CFLAGS_SX) crctab.c -Fo$@

crypt_.obj:	crypt.c $(UNZIP_H) zip.h crypt.h ttyio.h	# unzipsfx only
	$(cc) $(CFLAGS_SX) crypt.c -Fo$@

cryptf.obj:	crypt.c $(UNZIP_H) zip.h crypt.h ttyio.h	# funzip only
	$(cc) $(CFLAGS_FU) crypt.c -Fo$@

extract_.obj:	extract.c $(UNZIP_H) crypt.h			# unzipsfx only
	$(cc) $(CFLAGS_SX) extract.c -Fo$@

fileio_.obj:	fileio.c $(UNZIP_H) crypt.h ttyio.h ebcdic.h	# unzipsfx only
	$(cc) $(CFLAGS_SX) fileio.c -Fo$@

globals_.obj:	globals.c $(UNZIP_H)				# unzipsfx only
	$(cc) $(CFLAGS_SX) globals.c -Fo$@

globalsf.obj:	globals.c $(UNZIP_H)				# funzip only
	$(cc) $(CFLAGS_FU) globals.c -Fo$@

inflate_.obj:	inflate.c inflate.h $(UNZIP_H) crypt.h		# unzipsfx only
	$(cc) $(CFLAGS_SX) inflate.c -Fo$@

inflatef.obj:	inflate.c inflate.h $(UNZIP_H) crypt.h		# funzip only
	$(cc) $(CFLAGS_FU) inflate.c -Fo$@

match_.obj:	match.c $(UNZIP_H)				# unzipsfx only
	$(cc) $(CFLAGS_SX) match.c -Fo$@

process_.obj:	process.c $(UNZIP_H)				# unzipsfx only
	$(cc) $(CFLAGS_SX) process.c -Fo$@

timezon_.obj:	timezone.c $(UNZIP_H) zip.h timezone.h		# unzipsfx only
	$(cc) $(CFLAGS_SX) timezone.c -Fo$@

ttyio_.obj:	ttyio.c $(UNZIP_H) zip.h crypt.h ttyio.h	# unzipsfx only
	$(cc) $(CFLAGS_SX) ttyio.c -Fo$@

ttyiof.obj:	ttyio.c $(UNZIP_H) zip.h crypt.h ttyio.h	# funzip only
	$(cc) $(CFLAGS_FU) ttyio.c -Fo$@

unzipsfx.obj:	unzip.c $(UNZIP_H) crypt.h unzvers.h consts.h	# unzipsfx only
	$(cc) $(CFLAGS_SX) unzip.c -Fo$@

# DLL compilation section
api.obj:	api.c $(UNZIP_H) $(WINDLL_H) unzvers.h
	$(cc) $(CFLAGS_DL) api.c -Fo$@
crc32l.obj:	crc32.c $(UNZIP_H) zip.h
	$(cc) $(CFLAGS_DL) crc32.c -Fo$@
crctabl.obj:	crctab.c $(UNZIP_H) zip.h
	$(cc) $(CFLAGS_DL) crctab.c -Fo$@
cryptl.obj:	crypt.c $(UNZIP_H) zip.h crypt.h ttyio.h
	$(cc) $(CFLAGS_DL) crypt.c -Fo$@
explodel.obj:	explode.c $(UNZIP_H)
	$(cc) $(CFLAGS_DL) explode.c -Fo$@
extractl.obj:	extract.c $(UNZIP_H) $(WINDLL_H) crypt.h
	$(cc) $(CFLAGS_DL) extract.c -Fo$@
fileiol.obj:	fileio.c $(UNZIP_H) $(WINDLL_H) crypt.h ttyio.h ebcdic.h
	$(cc) $(CFLAGS_DL) fileio.c -Fo$@
globalsl.obj:	globals.c $(UNZIP_H)
	$(cc) $(CFLAGS_DL) globals.c -Fo$@
inflatel.obj:	inflate.c inflate.h $(UNZIP_H)
	$(cc) $(CFLAGS_DL) inflate.c -Fo$@
listl.obj:	list.c $(UNZIP_H) $(WINDLL_H)
	$(cc) $(CFLAGS_DL) list.c -Fo$@
matchl.obj:	match.c $(UNZIP_H)
	$(cc) $(CFLAGS_DL) match.c -Fo$@
processl.obj:	process.c $(UNZIP_H) $(WINDLL_H)
	$(cc) $(CFLAGS_DL) process.c -Fo$@
timezonl.obj:	timezone.c $(UNZIP_H) zip.h timezone.h
	$(cc) $(CFLAGS_DL) timezone.c -Fo$@
unreducl.obj:	unreduce.c $(UNZIP_H)
	$(cc) $(CFLAGS_DL) unreduce.c -Fo$@
unshrnkl.obj:	unshrink.c $(UNZIP_H)
	$(cc) $(CFLAGS_DL) unshrink.c -Fo$@
zipinfol.obj:	zipinfo.c $(UNZIP_H)
	$(cc) $(CFLAGS_DL) zipinfo.c -Fo$@

win32l.obj:	win32/win32.c $(UNZIP_H) win32/nt.h
	$(cc) $(CFLAGS_DL) -I. win32/win32.c -Fo$@

ntl.obj:	win32/nt.c $(UNZIP_H) win32/nt.h
	$(cc) $(CFLAGS_DL) -I. win32/nt.c -Fo$@

windll.obj:  windll/windll.c $(UNZIP_H) $(WINDLL_H) crypt.h unzvers.h consts.h
	$(cc) $(CFLAGS_DL) -I. windll/windll.c -Fo$@

!ifdef USEMASM
crci386l.obj:	win32/crc_i386.asm
	masm -ml win32/crc_i386.asm,$@;
!else
!ifdef USEML
crci386l.obj:	win32/crc_i386.asm
	ml -c -Cp -coff -Fo$@ win32/crc_i386.asm
!else
crci386l.obj:	win32/crc_i386.c
	$(cc) $(CFLAGS_DL) -I. win32/crc_i386.c -Fo$@
!endif
!endif

windll.res:	windll/windll.rc unzvers.h
	$(rc) /l 0x409 /fo$@ /i windll /d WIN32 windll/windll.rc

uzexampl.obj:	windll/uzexampl.c windll/uzexampl.h
	$(cc) $(CFLAGS_UN) -I. windll/uzexampl.c -Fo$@

# SFX Lib compilation section
apig.obj:	api.c $(UNZIP_H) $(WINDLL_H) unzvers.h
	$(cc) $(CFLAGS_SL) api.c -Fo$@
crc32g.obj:	crc32.c $(UNZIP_H) zip.h
	$(cc) $(CFLAGS_SL) crc32.c -Fo$@
crctabg.obj:	crctab.c $(UNZIP_H) zip.h
	$(cc) $(CFLAGS_SL) crctab.c -Fo$@
cryptg.obj:	crypt.c $(UNZIP_H) zip.h crypt.h ttyio.h
	$(cc) $(CFLAGS_SL) crypt.c -Fo$@
extractg.obj:	extract.c $(UNZIP_H) $(WINDLL_H) crypt.h
	$(cc) $(CFLAGS_SL) extract.c -Fo$@
fileiog.obj:	fileio.c $(UNZIP_H) $(WINDLL_H) crypt.h ttyio.h ebcdic.h
	$(cc) $(CFLAGS_SL) fileio.c -Fo$@
globalsg.obj:	globals.c $(UNZIP_H)
	$(cc) $(CFLAGS_SL) globals.c -Fo$@
inflateg.obj:	inflate.c inflate.h $(UNZIP_H)
	$(cc) $(CFLAGS_SL) inflate.c -Fo$@
matchg.obj:	match.c $(UNZIP_H)
	$(cc) $(CFLAGS_SL) match.c -Fo$@
processg.obj:	process.c $(UNZIP_H) $(WINDLL_H)
	$(cc) $(CFLAGS_SL) process.c -Fo$@
timezong.obj:	timezone.c $(UNZIP_H) zip.h timezone.h
	$(cc) $(CFLAGS_SL) timezone.c -Fo$@

win32g.obj:	win32/win32.c $(UNZIP_H) win32/nt.h
	$(cc) $(CFLAGS_SL) -I. win32/win32.c -Fo$@

ntg.obj:	win32/nt.c $(UNZIP_H) win32/nt.h
	$(cc) $(CFLAGS_SL) -I. win32/nt.c -Fo$@

windllg.obj:  windll/windll.c $(UNZIP_H) $(WINDLL_H) crypt.h unzvers.h consts.h
	$(cc) $(CFLAGS_SL) -I. windll/windll.c -Fo$@

!ifdef USEMASM
crci386g.obj:	win32/crc_i386.asm
	masm -ml win32/crc_i386.asm,$@;
!else
!ifdef USEML
crci386g.obj:	win32/crc_i386.asm
	ml -c -Cp -coff -Fo$@ win32/crc_i386.asm
!else
crci386g.obj:	win32/crc_i386.c
	$(cc) $(CFLAGS_SL) -I. win32/crc_i386.c -Fo$@
!endif
!endif

sfxwiz.obj:	windll/guisfx/sfxwiz.c windll/guisfx/dialog.h $(WINDLL_IMP_H)
	$(cc) $(CFLAGS_GX) windll/guisfx/sfxwiz.c -Fo$@

sfxwiz.res:	windll/guisfx/sfxwiz.rc
	$(rc) /l 0x409 /fo$@ /i windll/guisfx /d WIN32 windll/guisfx/sfxwiz.rc

# static LIB compilation section
$(OB)apib.obj:	api.c $(UNZIP_H) $(WINDLL_H) unzvers.h
	$(cc) $(CFLAGS_LB) api.c -Fo$@
$(OB)crc32b.obj:	crc32.c $(UNZIP_H) zip.h
	$(cc) $(CFLAGS_LB) crc32.c -Fo$@
$(OB)crctabb.obj:	crctab.c $(UNZIP_H) zip.h
	$(cc) $(CFLAGS_LB) crctab.c -Fo$@
$(OB)cryptb.obj:	crypt.c $(UNZIP_H) zip.h crypt.h ttyio.h
	$(cc) $(CFLAGS_LB) crypt.c -Fo$@
$(OB)explodeb.obj:	explode.c $(UNZIP_H)
	$(cc) $(CFLAGS_LB) explode.c -Fo$@
$(OB)extractb.obj:	extract.c $(UNZIP_H) $(WINDLL_H) crypt.h
	$(cc) $(CFLAGS_LB) extract.c -Fo$@
$(OB)fileiob.obj:	fileio.c $(UNZIP_H) $(WINDLL_H) crypt.h ttyio.h ebcdic.h
	$(cc) $(CFLAGS_LB) fileio.c -Fo$@
$(OB)globalsb.obj:	globals.c $(UNZIP_H)
	$(cc) $(CFLAGS_LB) globals.c -Fo$@
$(OB)inflateb.obj:	inflate.c inflate.h $(UNZIP_H)
	$(cc) $(CFLAGS_LB) inflate.c -Fo$@
$(OB)listb.obj:	list.c $(UNZIP_H) $(WINDLL_H)
	$(cc) $(CFLAGS_LB) list.c -Fo$@
$(OB)matchb.obj:	match.c $(UNZIP_H)
	$(cc) $(CFLAGS_LB) match.c -Fo$@
$(OB)processb.obj:	process.c $(UNZIP_H) $(WINDLL_H)
	$(cc) $(CFLAGS_LB) process.c -Fo$@
$(OB)timezonb.obj:	timezone.c $(UNZIP_H) zip.h timezone.h
	$(cc) $(CFLAGS_LB) timezone.c -Fo$@
$(OB)unreducb.obj:	unreduce.c $(UNZIP_H)
	$(cc) $(CFLAGS_LB) unreduce.c -Fo$@
$(OB)unshrnkb.obj:	unshrink.c $(UNZIP_H)
	$(cc) $(CFLAGS_LB) unshrink.c -Fo$@
$(OB)zipinfob.obj:	zipinfo.c $(UNZIP_H)
	$(cc) $(CFLAGS_LB) zipinfo.c -Fo$@

$(OB)win32b.obj:	win32/win32.c $(UNZIP_H) win32/nt.h
	$(cc) $(CFLAGS_LB) -I. win32/win32.c -Fo$@

$(OB)ntb.obj:	win32/nt.c $(UNZIP_H) win32/nt.h
	$(cc) $(CFLAGS_LB) -I. win32/nt.c -Fo$@

$(OB)windllb.obj:  windll/windll.c $(UNZIP_H) $(WINDLL_H) crypt.h unzvers.h consts.h
	$(cc) $(CFLAGS_LB) -I. windll/windll.c -Fo$@

!ifdef USEMASM
$(OB)crci386b.obj:	win32/crc_i386.asm
	masm -ml win32/crc_i386.asm,$@;
!else
!ifdef USEML
$(OB)crci386b.obj:	win32/crc_i386.asm
	ml -c -Cp -coff -Fo$@ win32/crc_i386.asm
!else
$(OB)crci386b.obj:	win32/crc_i386.c
	$(cc) $(CFLAGS_LB) -I. win32/crc_i386.c -Fo$@
!endif
!endif

$(OBDIR):
	-mkdir $@

clean:
	del $(OBJU) unzip.exe
	del $(OBJF) funzip.exe
	del $(OBJX) unzipsfx.exe
	del $(OBJD) windll.res unzip32.dll unzip32.lib unzip32.exp
        del $(OBJB) $(OB)unzip32.lib
	del $(OBLX) unzsfx32.lib
