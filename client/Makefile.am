## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

# (for a while we used "-static -static-libgcc" on linux, but this is obsolete
# now)
#STATIC_FLAGS=@STATIC_FLAGS@

client-bin: @CLIENT_BIN_FILENAME@

LIBS += @CLIENTLIBS@

bin_PROGRAMS = boinc_client

EXTRA_PROGRAMS = cpu_benchmark

boinc_client_SOURCES = \
    app.C \
    app_control.C \
    app_graphics.C \
    app_start.C \
    check_state.C \
    client_msgs.C \
    client_state.C \
    client_types.C \
    cs_account.C \
    cs_apps.C \
    cs_benchmark.C \
    cs_cmdline.C \
    cs_data.C \
    cs_files.C \
    cs_prefs.C \
    cs_scheduler.C \
    cs_statefile.C \
    cs_trickle.C \
    dhrystone.C \
    dhrystone2.C \
    file_names.C \
    file_xfer.C \
    gui_rpc_server.C \
    hostinfo_network.C \
    hostinfo_unix.C \
    http.C \
    log_flags.C \
    main.C \
    net_stats.C \
    net_xfer.C \
    pers_file_xfer.C \
    proxy.C \
    scheduler_op.C \
    ss_logic.C \
    time_stats.C \
    whetstone.C

boinc_client_DEPENDENCIES = $(LIBRSA)
boinc_client_CPPFLAGS = -I $(srcdir)/win $(AM_CPPFLAGS) -O3
boinc_client_LDADD = -L../lib -lboinc $(RSA_LIBS) $(PTHREAD_LIBS)
#boinc_client_LDFLAGS = $(STATIC_FLAGS)

# the following don't do anything
cpu_benchmark_SOURCES = whetstone.C dhrystone.C
cpu_benchmark_CFLAGS = -O3 $(AM_CFLAGS)

all-local: client-bin

# make a hard link to the client name.
@CLIENT_BIN_FILENAME@: boinc_client
	rm -f $@
	ln $? $@

## these source files need to be specified because no rule uses them.

EXTRA_DIST =                                    \
    app.h                                       \
    client_msgs.h                               \
    client_state.h                              \
    client_types.h                              \
    cpp.h                                       \
    cpu_benchmark.h                             \
    dhrystone.h                                 \
    file_names.h                                \
    file_xfer.h                                 \
    gui_rpc_server.h                            \
    gui_titles.h                                \
    hostinfo_network.h                          \
    http.h                                      \
    log_flags.h                                 \
    mac                                         \
    main.h                                      \
    net_stats.h                                 \
    net_xfer.h                                  \
    pers_file_xfer.h                            \
    proxy.h                                     \
    scheduler_op.h                              \
    ss_logic.h                                  \
    ssl_http.h                                  \
    ssl_net_xfer.h                              \
    time_stats.h                                \
    translation                                 \
    win

clean-local:
	rm -f @CLIENT_BIN_FILENAME@
