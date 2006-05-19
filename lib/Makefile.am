## -*- mode: make; tab-width: 4 -*-
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
    boinc_cmd.C \
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
EXTRA_DIST = *.h *.C

if OS_DARWIN
mac_sources = 			\
	mac_backtrace.C 	\
	mac_backtrace.h  	\
	MoreAddrToSym.c  	\
	MoreAddrToSym.h  	\
	MoreBacktrace.c  	\
	MoreBacktrace.h
else
mac_sources = 
endif

libboinc_a_SOURCES = \
    app_ipc.C \
    base64.C \
    crypt.C \
    diagnostics.C \
    filesys.C \
    gui_rpc_client.C \
    gui_rpc_client_ops.C \
    gui_rpc_client_print.C \
    hostinfo.C \
    md5.c \
    md5_file.C \
    mem_usage.C \
    mfile.C \
    miofile.C \
    msg_log.C \
    network.C \
    parse.C \
    prefs.C \
    proxy_info.C \
    shmem.C \
    util.C	\
	$(mac_sources)



include_HEADERS = 	\
    app_ipc.h 		\
    boinc_win.h 	\
    diagnostics.h 	\
    filesys.h 		\
    hostinfo.h		\
    mfile.h 		\
    miofile.h 		\
    parse.h 		\
    prefs.h 		\
    proxy_info.h 	\
    util.h 			\
	msg_log.h 		\
	crypt.h

## install header-files with prefix-subdir BOINC/ to avoid name-conflicts
includedir = ${prefix}/include/BOINC/

md5_test_SOURCES = md5_test.C md5.c md5_file.C
shmem_test_SOURCES = shmem_test.C shmem.C
msg_test_SOURCES = msg_test.C msg_queue.C
crypt_prog_SOURCES = crypt_prog.C crypt.C md5.c md5_file.C
crypt_prog_DEPENDENCIES = $(LIBRSA)
crypt_prog_LDADD = $(RSA_LIBS) $(PTHREAD_LIBS)
