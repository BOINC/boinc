## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

# (for a while we used "-static -static-libgcc" on linux, but this is obsolete
# now)
#STATIC_FLAGS=@STATIC_FLAGS@

client-bin: @CLIENT_BIN_FILENAME@

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
    whetstone.C \
    ../lib/app_ipc.C \
    ../lib/base64.C \
    ../lib/crypt.C \
    ../lib/diagnostics.C \
    ../lib/exception.C \
    ../lib/filesys.C \
    ../lib/hostinfo.C \
    ../lib/language.C \
    ../lib/md5_file.C \
    ../lib/md5.c \
    ../lib/mfile.C \
    ../lib/miofile.C \
    ../lib/msg_log.C \
    ../lib/parse.C \
    ../lib/prefs.C \
    ../lib/proxy_info.C \
    ../lib/shmem.C \
    ../lib/util.C

boinc_client_DEPENDENCIES = $(LIBRSA)
boinc_client_CPPFLAGS = -I $(srcdir)/win $(AM_CPPFLAGS) -O3
boinc_client_LDADD = $(RSA_LIBS) $(PTHREAD_LIBS)
#boinc_client_LDFLAGS = $(STATIC_FLAGS)

# the following don't do anything
cpu_benchmark_SOURCES = whetstone.C dhrystone.C
cpu_benchmark_CFLAGS = -O3 $(AM_CFLAGS)

all-local: client-bin


win/win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win/win_config.h

# make a hard link to the client name.
@CLIENT_BIN_FILENAME@: boinc_client
	rm -f $@
	ln $? $@

## these source files need to be specified because no rule on unix uses them.

EXTRA_DIST = \
	app.h		 dhrystone.h	     http.h	       proxy.h  				\
	client_msgs.h	 file_names.h	     log_flags.h       scheduler_op.h	\
	client_state.h	 file_xfer.h	     main.h	       ss_logic.h			\
	client_types.h	 gui_rpc_server.h    net_stats.h       ssl_http.h		\
	cpp.h		 gui_titles.h	     net_xfer.h        ssl_net_xfer.h		\
	cpu_benchmark.h  hostinfo_network.h  pers_file_xfer.h  time_stats.h		\
	mac win translation

clean-local:
	rm -f @CLIENT_BIN_FILENAME@

.PHONY: version
