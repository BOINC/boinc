## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS =

# Stuff needed for server builds goes here.
if ENABLE_SERVER

serverbindir = $(libexecdir)/boinc-server-maker/lib
serverbin_PROGRAMS = crypt_prog parse_test

endif
# end of "if ENABLE_SERVER"


if ENABLE_LIBRARIES

libfcgi_sources = \
    coproc.cpp \
    filesys.cpp \
    hostinfo.cpp \
	keyword.cpp \
    md5.cpp \
    md5_file.cpp \
    mfile.cpp \
    miofile.cpp \
    msg_log.cpp \
    opencl_boinc.cpp \
    parse.cpp \
    sched_msgs.cpp \
    shmem.cpp \
    str_util.cpp \
    synch.cpp \
    url.cpp \
    util.cpp \
	wslinfo.cpp

generic_sources = \
    app_ipc.cpp \
    base64.cpp \
    cc_config.cpp \
    cert_sig.cpp \
    coproc.cpp \
    diagnostics.cpp \
    filesys.cpp \
    gui_rpc_client.cpp \
    gui_rpc_client_ops.cpp \
    gui_rpc_client_print.cpp \
    hostinfo.cpp \
	keyword.cpp \
    md5.cpp \
    md5_file.cpp \
    mem_usage.cpp \
    mfile.cpp \
    miofile.cpp \
    msg_log.cpp \
    network.cpp \
    notice.cpp \
    opencl_boinc.cpp \
    parse.cpp \
    prefs.cpp \
    procinfo.cpp \
    proc_control.cpp \
    project_init.cpp \
    proxy_info.cpp \
    sched_msgs.cpp \
    shmem.cpp \
    str_util.cpp \
    url.cpp \
    util.cpp \
	wslinfo.cpp

if OS_WIN32
win_sources = \
    boinc_win.cpp \
    daemonmgt_win.cpp \
    diagnostics_win.cpp \
    idlemon_win.cpp \
    procinfo_win.cpp \
    run_app_windows.cpp \
    stackwalker_win.cpp \
    win_util.cpp \
    wslinfo.cpp
win_headers= \
    boinc_win.h \
    diagnostics_win.h \
    run_app_windows.h \
    stackwalker_win.h \
    win_util.h \
    wslinfo.h
else # !OS_WIN32
if OS_DARWIN
mac_sources = \
    procinfo_mac.cpp \
    mac/mac_backtrace.cpp \
    mac/mac_spawn.cpp \
    mac/QBacktrace.c \
    mac/QCrashReport.c \
    mac/QMachOImage.c \
    mac/QMachOImageList.c \
    mac/QSymbols.c \
    mac/QTaskMemory.c \
    mac/mac_branding.cpp
mac_headers = \
    mac/dyld_gdb.h \
    mac/mac_backtrace.h \
    mac/mac_spawn.h \
    mac/QBacktrace.h \
    mac/QCrashReport.h \
    mac/QMachOImage.h \
    mac/QMachOImageList.h \
    mac/QSymbols.h \
    mac/QTaskMemory.h \
    mac/mac_branding.h
win_sources=
win_headers=
else # !(OS_WIN32 || OS_MAC)

generic_sources += \
    procinfo_unix.cpp \
    synch.cpp \
    unix_util.cpp
win_sources=
win_headers=
mac_sources=
mac_headers=

endif
endif

if INSTALL_HEADERS
pkginclude_HEADERS = \
    app_ipc.h \
    average.h \
    base64.h \
    boinc_stdio.h \
    cal_boinc.h \
    cc_config.h \
    cert_sig.h \
    cl_boinc.h \
    common_defs.h \
    coproc.h \
    crypt.h \
    diagnostics.h \
    error_numbers.h \
    filesys.h \
    gui_rpc_client.h \
    hostinfo.h \
    md5.h \
    md5_file.h \
    mem_usage.h \
    mfile.h \
    miofile.h \
    msg_log.h \
    network.h \
    notice.h \
    opencl_boinc.h \
    parse.h \
    prefs.h \
    proc_control.h \
    procinfo.h \
    proxy_info.h \
    sched_msgs.h \
    stackwalker_imports.h \
    str_replace.h \
    str_util.h \
    url.h \
    util.h \
    $(mac_headers) \
    $(win_headers)
endif

# These are used internally and are not meant to be installed
noinst_HEADERS = \
    msg_queue.h \
    shmem.h \
    synch.h \
    unix_util.h

lib_LTLIBRARIES = libboinc.la
pkgconfig_DATA = libboinc.pc
libboinc_la_SOURCES = $(generic_sources) $(mac_sources) $(win_sources)
libboinc_la_CFLAGS = $(AM_CFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_la_CXXFLAGS = $(AM_CXXFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_la_LDFLAGS = -static -version-number $(LIBBOINC_VERSION)
if OS_FREEBSD
libboinc_la_LDFLAGS += -lexecinfo
endif
libboinc_la_LIBADD =

if ENABLE_BOINCCRYPT
lib_LTLIBRARIES += libboinc_crypt.la
pkgconfig_DATA += libboinc_crypt.pc
libboinc_crypt_la_SOURCES = crypt.cpp
libboinc_crypt_la_CFLAGS = $(AM_CFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS) $(SSL_CFLAGS)
libboinc_crypt_la_CXXFLAGS = $(AM_CXXFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS) $(SSL_CXXFLAGS)
libboinc_crypt_la_LDFLAGS = -static -version-number $(LIBBOINC_VERSION)
libboinc_crypt_la_LIBADD =
endif

if ENABLE_FCGI
lib_LTLIBRARIES += libboinc_fcgi.la
pkgconfig_DATA += libboinc_fcgi.pc
libboinc_fcgi_la_SOURCES = $(libfcgi_sources) $(mac_sources) $(win_sources)
libboinc_fcgi_la_CFLAGS = -D_USING_FCGI_ $(AM_CFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_fcgi_la_CXXFLAGS = -D_USING_FCGI_ $(AM_CXXFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_fcgi_la_LDFLAGS = -version-number $(LIBBOINC_VERSION)
libboinc_fcgi_la_LIBADD =
endif
# end of "if ENABLE_FCGI"

# Some OSs may not prefix libraries with lib.
# For example OS2
if OS_OS2
LIBBOINC_STATIC=boinc.${LIBEXT}
LIBBOINC_CRYPT_STATIC=boinc_crypt.${LIBEXT}
LIBBOINC_FCGI_STATIC=boinc_fcgi.${LIBEXT}
else
LIBBOINC_STATIC=libboinc.${LIBEXT}
LIBBOINC_CRYPT_STATIC=libboinc_crypt.${LIBEXT}
LIBBOINC_FCGI_STATIC=libboinc_fcgi.${LIBEXT}
endif


if BUILD_STATIC_LIBS
all_local = $(LIBBOINC_STATIC)
if ENABLE_BOINCCRYPT
all_local += $(LIBBOINC_CRYPT_STATIC)
endif
if ENABLE_FCGI
all_local += $(LIBBOINC_FCGI_STATIC)
endif
endif

all-local: $(all_local)

$(LIBBOINC_STATIC): libboinc.la
	rm -f $(LIBBOINC_STATIC)
	$(LN) .libs/$(LIBBOINC_STATIC) .

$(LIBBOINC_CRYPT_STATIC): libboinc_crypt.la
	rm -f $(LIBBOINC_CRYPT_STATIC)
	$(LN) .libs/$(LIBBOINC_CRYPT_STATIC) .

$(LIBBOINC_FCGI_STATIC): libboinc_fcgi.la
	rm -f $(LIBBOINC_FCGI_STATIC)
	$(LN) .libs/$(LIBBOINC_FCGI_STATIC) .

clean: clean-am
	rm -f $(LIBBOINC_STATIC) $(LIBBOINC_CRYPT_STATIC) $(LIBBOINC_FCGI_STATIC) *.gcno *.gcda *.gcov *.a *.o *.lo *.la

endif
# end of "if ENABLE_LIBRARIES"

EXTRA_PROGRAMS = md5_test shmem_test msg_test

EXTRA_DIST = *.h *.cpp

md5_test_SOURCES = md5_test.cpp
md5_test_CXXFLAGS = $(PTHREAD_CFLAGS)
md5_test_LDADD = $(LIBBOINC)
shmem_test_SOURCES = shmem_test.cpp
shmem_test_CXXFLAGS = $(PTHREAD_CFLAGS)
shmem_test_LDADD = $(LIBBOINC)
msg_test_SOURCES = msg_test.cpp
msg_test_CXXFLAGS = $(PTHREAD_CFLAGS)
msg_test_LDADD = $(LIBBOINC)
crypt_prog_SOURCES = crypt_prog.cpp
crypt_prog_CXXFLAGS = $(PTHREAD_CFLAGS) $(SSL_CXXFLAGS)
crypt_prog_LDADD = $(LIBBOINC_CRYPT_STATIC) $(LIBBOINC) $(SSL_LIBS)
parse_test_SOURCES = parse_test.cpp
parse_test_CXXFLAGS = $(PTHREAD_CFLAGS)
parse_test_LDADD = $(LIBBOINC)
