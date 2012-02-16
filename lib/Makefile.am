## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS =

# Stuff needed for server builds goes here.
if ENABLE_SERVER

bin_PROGRAMS += crypt_prog parse_test

endif
# end of "if ENABLE_SERVER"


if ENABLE_LIBRARIES

libfcgi_sources = \
    boinc_fcgi.cpp \
    coproc.cpp \
    filesys.cpp \
    hostinfo.cpp \
    md5.c \
    md5_file.cpp \
    miofile.cpp \
    msg_log.cpp \
    parse.cpp \
    shmem.cpp \
    str_util.cpp \
    synch.cpp \
	url.cpp \
    util.cpp

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
    md5.c \
    md5_file.cpp \
    mem_usage.cpp \
    mfile.cpp \
    miofile.cpp \
    msg_log.cpp \
    network.cpp \
	notice.cpp \
    parse.cpp \
    prefs.cpp \
	procinfo.cpp \
	proc_control.cpp \
    proxy_info.cpp \
    shmem.cpp \
    str_util.cpp \
    synch.cpp \
    unix_util.cpp \
	url.cpp \
    util.cpp

if OS_DARWIN
mac_sources = \
    procinfo_mac.cpp \
    mac/mac_backtrace.cpp \
    mac/QBacktrace.c \
    mac/QCrashReport.c \
    mac/QMachOImage.c \
    mac/QMachOImageList.c \
    mac/QSymbols.c \
    mac/QTaskMemory.c
mac_headers = \
    mac/dyld_gdb.h \
    mac/mac_backtrace.h \
    mac/QBacktrace.h \
    mac/QCrashReport.h \
    mac/QMachOImage.h \
    mac/QMachOImageList.h \
    mac/QSymbols.h \
    mac/QTaskMemory.h
else
    generic_sources += procinfo_unix.cpp

mac_sources = 
mac_headers =
endif


if INSTALL_HEADERS
pkginclude_HEADERS = \
    app_ipc.h \
    average.h \
    base64.h \
    boinc_fcgi.h \
    boinc_win.h \
    cal_boinc.h \
	cc_config.h \
    cert_sig.h \
    cl_boinc.h \
    common_defs.h \
    coproc.h \
    crypt.h \
    diagnostics.h \
    diagnostics_win.h \
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
    parse.h \
    prefs.h \
	procinfo.h \
    proxy_info.h \
    stackwalker_imports.h \
    stackwalker_win.h \
    str_util.h \
	url.h \
    util.h \
    win_util.h \
    $(mac_headers)
endif

# These are used internally and are not meant to be installed
noinst_HEADERS = \
    msg_queue.h \
    proc_control.h \
    procinfo.h \
    shmem.h \
    str_replace.h \
    synch.h \
    unix_util.h

lib_LTLIBRARIES = libboinc.la libboinc_crypt.la
libboinc_la_SOURCES = $(generic_sources) $(mac_sources)
libboinc_la_CFLAGS = $(AM_CFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_la_CXXFLAGS = $(AM_CXXFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_la_LDFLAGS = -L$(libdir) -rpath $(libdir) -static -version-number $(LIBBOINC_VERSION)
libboinc_la_LIBADD =

libboinc_crypt_la_SOURCES = crypt.cpp
libboinc_crypt_la_CFLAGS = $(AM_CFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_crypt_la_CXXFLAGS = $(AM_CXXFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_crypt_la_LDFLAGS = -L$(libdir) -rpath $(libdir) -static -version-number $(LIBBOINC_VERSION)
libboinc_crypt_la_LIBADD = 


if ENABLE_FCGI
lib_LTLIBRARIES += libboinc_fcgi.la
libboinc_fcgi_la_SOURCES = $(libfcgi_sources) $(mac_sources)
libboinc_fcgi_la_CFLAGS = -D_USING_FCGI_ $(AM_CFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_fcgi_la_CXXFLAGS = -D_USING_FCGI_ $(AM_CXXFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS)
libboinc_fcgi_la_LDFLAGS = -L$(libdir) -rpath $(libdir) -version-number $(LIBBOINC_VERSION)
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
all_local = $(LIBBOINC_STATIC) $(LIBBOINC_CRTYPT_STATIC)
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
crypt_prog_CXXFLAGS = $(PTHREAD_CFLAGS)
crypt_prog_LDADD = $(LIBBOINC_CRYPT_STATIC) $(LIBBOINC) $(SSL_LIBS) 
parse_test_SOURCES = parse_test.cpp 
parse_test_CXXFLAGS = $(PTHREAD_CFLAGS)
parse_test_LDADD = $(LIBBOINC)
