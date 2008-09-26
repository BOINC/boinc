## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

#if HAVE_NSL
nslprogs=boinc_cmd$(EXEEXT)
#endif

bin_PROGRAMS = crypt_prog$(EXEEXT) $(nslprogs)

EXTRA_PROGRAMS = md5_test$(EXEEXT) shmem_test$(EXEEXT) msg_test$(EXEEXT)

all-local: client-bin

client-bin: @CLIENT_CMD_BIN_FILENAME@

boinc_cmd_SOURCES = \
    boinc_cmd.cpp \
    gui_rpc_client.h

boinc_cmd_LDADD = $(lib_LIBRARIES) $(PTHREAD_LIBS)

# make a hard link to the client name.
@CLIENT_CMD_BIN_FILENAME@: boinc_cmd$(EXEEXT)
	rm -f $@
	rm -f $@.unmodified
	cp $? $@.unmodified
	@LN@ $? $@
	@STRIP@ $@

lib_LIBRARIES = libboinc.a 
EXTRA_DIST = *.h *.cpp

if OS_DARWIN
mac_sources = \
	mac/dyld_gdb.h \
	mac/mac_backtrace.cpp \
	mac/mac_backtrace.h \
	mac/QBacktrace.c \
	mac/QBacktrace.h \
	mac/QCrashReport.c \
	mac/QCrashReport.h \
	mac/QMachOImage.c \
	mac/QMachOImage.h \
	mac/QMachOImageList.c \
	mac/QMachOImageList.h \
	mac/QSymbols.c \
	mac/QSymbols.h \
	mac/QTaskMemory.c \
	mac/QTaskMemory.h
else
mac_sources = 
endif

libboinc_a_SOURCES = \
    app_ipc.cpp \
    base64.cpp \
	cert_sig.cpp \
    coproc.cpp \
    crypt.cpp \
    diagnostics.cpp \
    filesys.cpp \
    gui_rpc_client.cpp \
    gui_rpc_client_ops.cpp \
    gui_rpc_client_print.cpp \
    hostinfo.cpp \
    md5.c \
    md5_file.cpp \
    mem_usage.cpp \
    mfile.cpp \
    miofile.cpp \
    msg_log.cpp \
    network.cpp \
    parse.cpp \
    prefs.cpp \
    procinfo_unix.cpp \
    proxy_info.cpp \
    shmem.cpp \
    str_util.cpp \
    util.cpp	\
    unix_util.cpp \
	$(mac_sources)

pkginclude_HEADERS = \
    app_ipc.h \
    boinc_win.h \
    boinc_fcgi.h \
    common_defs.h	\
    coproc.h \
    crypt.h \
    diagnostics.h \
    error_numbers.h \
    filesys.h \
    gui_rpc_client.h \
    hostinfo.h \
    mfile.h \
    miofile.h \
    msg_log.h \
    parse.h \
    prefs.h \
    proxy_info.h \
    std_fixes.h \
    str_util.h \
    util.h

md5_test_SOURCES = md5_test.cpp md5.c md5_file.cpp
shmem_test_SOURCES = shmem_test.cpp shmem.cpp
msg_test_SOURCES = msg_test.cpp msg_queue.cpp
crypt_prog_SOURCES = crypt_prog.cpp crypt.cpp md5.c md5_file.cpp filesys.cpp
crypt_prog_DEPENDENCIES =
crypt_prog_LDADD = $(lib_LIBRARIES) $(RSA_LIBS) $(PTHREAD_LIBS)

clean-local:
	rm -f @CLIENT_CMD_BIN_FILENAME@
	rm -f @CLIENT_CMD_BIN_FILENAME@.unmodified
