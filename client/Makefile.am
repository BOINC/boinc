## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

if ENABLE_CLIENT_RELEASE
  AM_LDFLAGS += -static-libtool-libs
## for an entirely statically linked library, you may want to try
## -all-static instead.  There's a good chance it won't work properly,
## so we'll use the safer "-static-libtool-libs" by default.
else
if DYNAMIC_CLIENT
## if libtool starts to need flags for dynamic linking, add them here
else
  AM_LDFLAGS += -static
endif
endif ## ENABLE_CLIENT_RELEASE

LIBS += $(CLIENTLIBS)

if OS_DARWIN
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Cocoa -framework CoreServices
endif

bin_PROGRAMS = boinc_client switcher boinccmd

boinccmd_SOURCES = boinc_cmd.cpp
boinccmd_DEPENDENCIES = $(LIBBOINC)
boinccmd_CPPFLAGS = $(AM_CPPFLAGS)
boinccmd_LDFLAGS = $(AM_LDFLAGS) -L../lib
boinccmd_LDADD = $(LIBBOINC) $(BOINC_EXTRA_LIBS) $(PTHREAD_LIBS)

boinc_client_SOURCES = \
    acct_mgr.cpp \
    acct_setup.cpp \
    app.cpp \
    app_config.cpp \
    app_control.cpp \
    app_start.cpp \
	async_file.cpp \
    check_state.cpp \
    client_msgs.cpp \
    client_state.cpp \
    client_types.cpp \
    cpu_sched.cpp \
    cs_account.cpp \
    cs_apps.cpp \
    cs_benchmark.cpp \
    cs_cmdline.cpp \
    cs_files.cpp \
    cs_notice.cpp \
    cs_platforms.cpp \
    cs_prefs.cpp \
    cs_proxy.cpp \
    cs_scheduler.cpp \
    cs_statefile.cpp \
    cs_trickle.cpp \
	current_version.cpp \
    dhrystone.cpp \
    dhrystone2.cpp \
    file_names.cpp \
    file_xfer.cpp \
	gpu_amd.cpp \
	gpu_detect.cpp \
	gpu_intel.cpp \
	gpu_nvidia.cpp \
	gpu_opencl.cpp \
    gui_http.cpp \
    gui_rpc_server.cpp \
    gui_rpc_server_ops.cpp \
    hostinfo_network.cpp \
    hostinfo_unix.cpp \
    http_curl.cpp \
    log_flags.cpp \
    main.cpp \
    net_stats.cpp \
    pers_file_xfer.cpp \
	project.cpp \
	result.cpp \
    rr_sim.cpp \
    sandbox.cpp \
    scheduler_op.cpp \
    time_stats.cpp \
    whetstone.cpp \
    work_fetch.cpp

boinc_client_DEPENDENCIES = $(LIBBOINC)
boinc_client_CPPFLAGS = $(AM_CPPFLAGS)
boinc_client_LDFLAGS = $(AM_LDFLAGS) -L../lib
if OS_DARWIN
boinc_client_LDFLAGS += -Wl,-flat_namespace,-undefined,dynamic_lookup
endif
boinc_client_LDADD = $(LIBBOINC) $(LIBBOINC_CRYPT) $(BOINC_EXTRA_LIBS) $(PTHREAD_LIBS)

boinc_clientdir = $(bindir)

switcher_SOURCES = switcher.cpp
switcher_LDFLAGS = $(AM_LDFLAGS) -L../lib
switcher_LDADD = $(LIBBOINC)

## since we are using libtool we need some magic to get boinc and boinc_client
## to both be installed properly.  The next two rules do that...
all-local: boinc

boinc: boinc_client
	rm -f boinc .libs/boinc
	$(LN) boinc_client boinc
	if test -f .libs/boinc_client ; then $(LN) .libs/boinc_client .libs/boinc ; fi

install-exec-hook:
	rm -f $(DESTDIR)$(exec_prefix)/bin/boinc
	$(LN) $(DESTDIR)$(exec_prefix)/bin/boinc_client $(DESTDIR)$(exec_prefix)/bin/boinc

## these source files need to be specified because no rule uses them.

EXTRA_DIST = *.h \
    mac \
    translation \
    win
