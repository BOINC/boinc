## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

# (for a while we used "-static -static-libgcc" on linux, but this is obsolete
# now)
#STATIC_FLAGS=@STATIC_FLAGS@

client-bin: @CLIENT_BIN_FILENAME@

LIBS += @CLIENTLIBS@

if OS_DARWIN
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Cocoa
endif

bin_PROGRAMS = boinc_client switcher

boinc_client_SOURCES = \
    acct_mgr.cpp \
    acct_setup.cpp \
    app.cpp \
    app_control.cpp \
    app_graphics.cpp \
    app_start.cpp \
    auto_update.cpp \
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
    cs_platforms.cpp \
    cs_prefs.cpp \
    cs_scheduler.cpp \
    cs_statefile.cpp \
    cs_trickle.cpp \
    dhrystone.cpp \
    dhrystone2.cpp \
    file_names.cpp \
    file_xfer.cpp \
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
    sandbox.cpp \
    scheduler_op.cpp \
    time_stats.cpp \
    whetstone.cpp \
    work_fetch.cpp

boinc_client_DEPENDENCIES =
boinc_client_CPPFLAGS = -O3 -fomit-frame-pointer -fforce-addr -ffast-math $(AM_CPPFLAGS)
boinc_client_LDFLAGS = -static-libgcc
boinc_client_LDADD = $(LIBBOINC) $(PTHREAD_LIBS)

boinc_clientdir = $(bindir)
boinc_client_DATA = ../curl/ca-bundle.crt

switcher_SOURCES = switcher.cpp

all-local: client-bin

# make a hard link to the client name.
@CLIENT_BIN_FILENAME@: boinc_client
	rm -f $@
	rm -f $@.unmodified
	cp $? $@.unmodified
	@LN@ $? $@
	@STRIP@ $@

## these source files need to be specified because no rule uses them.

EXTRA_DIST = *.h \
    mac \
    translation \
    win

clean-local:
	rm -f @CLIENT_BIN_FILENAME@
	rm -f @CLIENT_BIN_FILENAME@.unmodified
