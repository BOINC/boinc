## -*- mode: makefile; tab-width: 4 -*-
## $Id$

CXXFLAGS := $(filter-out -mfpu=vfpv3-d16,$(CXXFLAGS))

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

bin_PROGRAMS = boinc_client boinccmd boinc

if OS_DARWIN
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Cocoa -framework CoreServices
   bin_PROGRAMS += switcher
endif

boinccmd_SOURCES = boinc_cmd.cpp
boinccmd_DEPENDENCIES = $(LIBBOINC)
boinccmd_CPPFLAGS = $(AM_CPPFLAGS)
boinccmd_LDFLAGS = $(AM_LDFLAGS) -L$(top_srcdir)/lib
boinccmd_LDADD = $(LIBBOINC) $(BOINC_EXTRA_LIBS) $(PTHREAD_LIBS)

boinc_client_SOURCES = \
    acct_mgr.cpp \
    acct_setup.cpp \
    app.cpp \
    app_config.cpp \
    app_control.cpp \
    app_start.cpp \
    app_test.cpp \
    async_file.cpp \
    check_state.cpp \
    client_msgs.cpp \
    client_state.cpp \
    client_types.cpp \
	coproc_sched.cpp \
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
    cs_sporadic.cpp \
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
    hostinfo_linux.cpp \
    hostinfo_network.cpp \
    http_curl.cpp \
    log_flags.cpp \
    mac_address.cpp \
    main.cpp \
    net_stats.cpp \
    pers_file_xfer.cpp \
    project.cpp \
    project_list.cpp \
    result.cpp \
    rr_sim.cpp \
    sandbox.cpp \
    scheduler_op.cpp \
    thread.cpp \
    time_stats.cpp \
    whetstone.cpp \
    work_fetch.cpp

boinc_client_DEPENDENCIES = $(LIBBOINC)
boinc_client_CPPFLAGS = $(AM_CPPFLAGS)
boinc_client_CXXFLAGS = $(AM_CXXFLAGS) $(SSL_CXXFLAGS)
boinc_client_LDFLAGS = $(AM_LDFLAGS) $(SSL_LDFLAGS) -L$(top_srcdir)/lib
if OS_WIN32
boinc_client_CXXFLAGS += -I$(top_srcdir)/coprocs/NVIDIA/include
boinc_client_SOURCES += hostinfo_win.cpp \
	hostinfo_wsl.cpp \
	sysmon_win.cpp \
	win/boinc_cli.rc \
	win/res/boinc.ico
else
if OS_DARWIN
boinc_client_LDFLAGS += -Wl,-flat_namespace,-undefined,dynamic_lookup
else
boinc_client_SOURCES += hostinfo_unix.cpp
endif
endif
boinc_client_LDADD = $(LIBBOINC) $(LIBBOINC_CRYPT) $(BOINC_EXTRA_LIBS) $(PTHREAD_LIBS)

boinc_clientdir = $(bindir)

if OS_ARM_LINUX
boinccmd_CPPFLAGS += -mfpu=vfpv3-d16
boinc_client_CPPFLAGS += -mfpu=vfpv3-d16
boinc_client_CXXFLAGS += -mfpu=vfpv3-d16
EXTRA_boinc_client_DEPENDENCIES = libwhetneon.a libwhetvfp.a
boinc_client_LDADD += libwhetneon.a libwhetvfp.a
noinst_LIBRARIES = libwhetneon.a libwhetvfp.a
libwhetneon_a_SOURCES = whetstone.cpp
libwhetneon_a_CXXFLAGS = $(filter-out -mfpu=vfpv3-d16,$(boinc_client_CXXFLAGS)) -DANDROID_NEON -mfloat-abi=softfp -mfpu=neon

libwhetvfp_a_SOURCES = whetstone.cpp
libwhetvfp_a_CXXFLAGS = $(boinc_client_CXXFLAGS) -DANDROID_VFP -mfloat-abi=softfp -mfpu=vfpv3-d16
endif

switcher_SOURCES = switcher.cpp
switcher_LDFLAGS = $(AM_LDFLAGS) -L../lib
switcher_LDADD = $(LIBBOINC)

## since we are using libtool we need some magic to get boinc and boinc_client
## to both be installed properly.  The next two rules do that...
all-local: boinc$(EXEEXT)

boinc$(EXEEXT): boinc_client$(EXEEXT)
	rm -f boinc$(EXEEXT) .libs/boinc$(EXEEXT)
	$(LN) boinc_client$(EXEEXT) boinc$(EXEEXT)
	if test -f .libs/boinc_client$(EXEEXT) ; then $(LN) .libs/boinc_client$(EXEEXT) .libs/boinc$(EXEEXT) ; fi

install-exec-hook:
	rm -f $(DESTDIR)$(exec_prefix)/bin/boinc$(EXEEXT)
	$(LN) $(DESTDIR)$(exec_prefix)/bin/boinc_client$(EXEEXT) $(DESTDIR)$(exec_prefix)/bin/boinc$(EXEEXT)

## these source files need to be specified because no rule uses them.

EXTRA_DIST = *.h \
    mac \
    translation \
    win
