## $Id$

include $(top_srcdir)/Makefile.incl

# by default, "-static -static-libgcc" on linux.
STATIC_FLAGS=@STATIC_FLAGS@

bin_PROGRAMS = boinc_client

EXTRA_PROGRAMS = cpu_benchmark

boinc_client_SOURCES =			\
    app.C				    	\
    check_state.C               \
    client_messages.C			\
    client_state.C				\
    client_types.C				\
    cpu_benchmark.C				\
    cs_account.C				\
    cs_apps.C					\
    cs_benchmark.C				\
    cs_cmdline.C				\
    cs_files.C					\
    cs_prefs.C					\
    cs_scheduler.C				\
    cs_statefile.C				\
    file_names.C				\
    file_xfer.C					\
    hostinfo.C					\
    hostinfo_unix.C				\
    http.C					    \
    log_flags.C					\
    main.C					    \
    message.C					\
    net_stats.C					\
    net_xfer.C					\
    pers_file_xfer.C			\
    prefs.C					    \
    scheduler_op.C				\
    ss_logic.C					\
    time_stats.C				\
    ../lib/app_ipc.C			\
    ../lib/filesys.C			\
    ../lib/language.C			\
    ../lib/parse.C				\
    ../lib/shmem.C				\
    ../lib/md5_file.C			\
    ../lib/md5.c				\
    ../lib/crypt.C				\
    ../lib/util.C				\
    ../lib/messages.C

boinc_client_DEPENDENCIES = $(LIBRSA)
boinc_client_CPPFLAGS = -I $(srcdir)/win $(AM_CPPFLAGS)
boinc_client_LDADD = $(RSA_LIBS)
boinc_client_LDFLAGS = $(STATIC_FLAGS)

cpu_benchmark_SOURCES = cpu_benchmark.C
cpu_benchmark_CFLAGS = -O6 $(AM_CFLAGS)

all-local: client-bin-gz
client-bin: @CLIENT_BIN_FILENAME@.gz
client-bin-gz: @CLIENT_BIN_FILENAME@.gz

win/win_config.h: $(top_srcdir)/config.h
	grep '#define.*VERSION' $^ > $@

win/resource.rc: $(top_srcdir)/config.h
	cd win && ./update-resource-version

version: win/win_config.h win/resource.rc

# make a hard link to the client name.
@CLIENT_BIN_FILENAME@: boinc_client
	rm -f $@
	ln $^ $@

@CLIENT_BIN_FILENAME@.gz: @CLIENT_BIN_FILENAME@
	gzip -c $^ > $@

## these source files need to be specified because no rule on unix uses them.
EXTRA_DIST =					\
	*.h					\
	mac/*.h mac/*.cpp			\
        win/*.h win/*.cpp win/*.rc		\
        win/*.ico win/*.def win/res/*.*		\
        translation/language.* 

clean-local:
	rm -f @CLIENT_BIN_FILENAME@ @CLIENT_BIN_FILENAME@.gz

.PHONY: version
