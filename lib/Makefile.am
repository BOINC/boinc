## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

#if HAVE_NSL
nslprogs=boinc_cmd
#endif

bin_PROGRAMS = crypt_prog $(nslprogs)

EXTRA_PROGRAMS = md5_test shmem_test msg_test
boinc_cmd_SOURCES = boinc_cmd.C gui_rpc_client.C gui_rpc_client.h
boinc_cmd_LDADD = $(lib_LIBRARIES) $(PTHREAD_LIBS)

lib_LIBRARIES = libboinc.a 

EXTRA_DIST = *.h *.C

libboinc_a_SOURCES =        \
    acct_mgr_client.C       \
    app_ipc.C               \
    base64.C                \
    crypt.C                 \
    diagnostics.C           \
    exception.C             \
    filesys.C               \
    hostinfo.C              \
    language.C              \
    md5.c                   \
    md5_file.C              \
    mem_usage.C             \
    mfile.C                 \
    miofile.C               \
    msg_log.C               \
    network.C \
    parse.C                 \
    prefs.C                 \
    proxy_info.C            \
    shmem.C                 \
    util.C

include_HEADERS =               \
                acct_mgr_client.h       \
                app_ipc.h       \
                boinc_win.h     \
                diagnostics.h   \
                exception.h     \
                filesys.h       \
                hostinfo.h      \
                mfile.h         \
                miofile.h   \
                parse.h \
                prefs.h         \
                proxy_info.h    \
                util.h      

## install header-files with prefix-subdir BOINC/ to avoid name-conflicts
includedir = ${prefix}/include/BOINC/

md5_test_SOURCES = md5_test.C md5.c md5_file.C
shmem_test_SOURCES = shmem_test.C shmem.C
msg_test_SOURCES = msg_test.C msg_queue.C
crypt_prog_SOURCES = crypt_prog.C crypt.C md5.c md5_file.C
crypt_prog_DEPENDENCIES = $(LIBRSA)
crypt_prog_LDADD = $(RSA_LIBS) $(PTHREAD_LIBS)
