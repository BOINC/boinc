## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

#if HAVE_NSL
nslprogs=gui_test
#endif

bin_PROGRAMS = crypt_prog $(nslprogs)

EXTRA_PROGRAMS = md5_test shmem_test msg_test


##LIBS = -lnsl -lsocket
gui_test_SOURCES = gui_test.C gui_rpc_client.C gui_rpc_client.h
gui_test_LDADD = $(lib_LIBRARIES)

lib_LIBRARIES = libboinc.a 

#libboinc_graphics.a
EXTRA_DIST = *.h *.C

# libboinc.a is for use by applications (not core client or server)
libboinc_a_SOURCES =                            \
    ../api/boinc_api.C                          \
    app_ipc.C                                   \
	base64.C                                    \
    diagnostics.C                               \
    exception.C                                 \
    filesys.C                                   \
    hostinfo.C                                  \
    language.C                                  \
    md5.c                                       \
    md5_file.C                                  \
    mfile.C                                     \
    miofile.C                                   \
    parse.C                                     \
    prefs.C                                     \
    proxy_info.C                                \
	shmem.C                                     \
    util.C

## install header-files with prefix-subdir BOINC/ to avoid name-conflicts
includedir = ${prefix}/include/BOINC/

## FIXME: currently we install ALL header-files: please remove those not needed!
include_HEADERS = \
	app_ipc.h \
	base64.h  \
	boinc_win.h  \
	crypt.h  \
	diagnostics.h  \
	error_numbers.h  \
	exception.h  \
	filesys.h  \
	gui_rpc_client.h  \
	hostinfo.h  \
	language.h  \
	md5_file.h  \
	md5.h  \
	mfile.h  \
	miofile.h  \
	msg_log.h  \
	msg_queue.h  \
	parse.h  \
	prefs.h  \
	proxy_info.h  \
	result_state.h  \
	shmem.h  \
	stackwalker_win.h  \
	std_fixes.h  \
	synch.h  \
	util.h 

# libboinc_graphics_a_SOURCES =			\
#         ../api/graphics_api.C			\
#         ../api/graphics_data.C

md5_test_SOURCES = md5_test.C md5.c md5_file.C
shmem_test_SOURCES = shmem_test.C shmem.C
msg_test_SOURCES = msg_test.C msg_queue.C
crypt_prog_SOURCES = crypt_prog.C crypt.C md5.c md5_file.C
crypt_prog_DEPENDENCIES = $(LIBRSA)
crypt_prog_LDADD = $(RSA_LIBS)
