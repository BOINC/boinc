## -*- mode: make; tab-width: 4 -*-
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
    acct_mgr.C \
    acct_setup.C \
    app.C \
    app_control.C \
    app_graphics.C \
    app_start.C \
    auto_update.C \
    check_state.C \
    client_msgs.C \
    client_state.C \
    client_types.C \
    cpu_sched.C \
    cs_account.C \
    cs_apps.C \
    cs_benchmark.C \
    cs_cmdline.C \
    cs_files.C \
    cs_platforms.C \
    cs_prefs.C \
    cs_scheduler.C \
    cs_statefile.C \
    cs_trickle.C \
    dhrystone.C \
    dhrystone2.C \
    file_names.C \
    file_xfer.C \
    gui_http.C \
    gui_rpc_server.C \
    gui_rpc_server_ops.C \
    hostinfo_network.C \
    hostinfo_unix.C \
    http_curl.C \
    log_flags.C \
    main.C \
    net_stats.C \
    pers_file_xfer.C \
    sandbox.C \
    scheduler_op.C \
    time_stats.C \
    whetstone.C \
    work_fetch.C

boinc_client_DEPENDENCIES =
boinc_client_CPPFLAGS = -O3 -fomit-frame-pointer -fforce-addr -ffast-math $(AM_CPPFLAGS)
boinc_client_LDFLAGS = -static-libgcc
boinc_client_LDADD = -L../lib -lboinc $(PTHREAD_LIBS)

boinc_clientdir = $(bindir)
boinc_client_DATA = ../curl/ca-bundle.crt

switcher_SOURCES = switcher.C

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
