# Generated automatically from Makefile.in by configure.
VERSION = 1

CFLAGS = -g -Wall  -DHAVE_LIBM=1 -DHAVE_LIBNSL=1 -DHAVE_LIBSOCKET=1 -DHAVE_LIBGEN=1 -DHAVE_STRFIND=1 -DHAVE_LIBSTDC__=1 -DHAVE_DIRENT_H=1 -DSTDC_HEADERS=1 -DHAVE_FCNTL_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_UNISTD_H=1 -DHAVE_SYS_SELECT_H=1 -DHAVE_SYS_STATVFS_H=1 -DHAVE_SYS_SWAP_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_STRSTR=1 -DHAVE_LOCKF=1 -DHAVE_STRFTIME=1 -DHAVE_LIB_SENDMAIL=1  \
    -DHOSTTYPE=\"sparc-sun-solaris2.7\" \
    -DVERSION=$(VERSION)

CC = g++ $(CFLAGS) -I ../lib

CLIBS = -lstdc++ -lgen -lsocket -lnsl -lm 

CLIENT_PROG = boinc_$(VERSION)_sparc-sun-solaris2.7
PROGS = $(CLIENT_PROG) test_net_xfer test_http test_file_xfer

all: $(PROGS)

OBJS = \
    accounts.o \
    app.o \
    client_state.o \
    cs_apps.o \
    cs_files.o \
    cs_scheduler.o \
    file_names.o \
    file_xfer.o \
    filesys.o \
    hostinfo.o \
    hostinfo_unix.o \
    http.o \
    log_flags.o \
    net_stats.o \
    net_xfer.o \
    parse.o \
    prefs.o \
    scheduler_reply.o \
    time_stats.o \
    types.o \
    util.o \
    version.o \
    ../lib/md5_file.o \
    ../lib/md5.o

TEST_NET_XFER_OBJS = \
    filesys.o \
    http.o \
    log_flags.o \
    parse.o \
    net_xfer.o

TEST_HTTP_OBJS = $(TEST_NET_XFER_OBJS)

TEST_FX_OBJS = \
    $(TEST_HTTP_OBJS) \
    util.o \
    file_xfer.o

.C.o:
	$(CC) -c -o $*.o $<
.c.o:
	$(CC) -c -o $*.o $<

$(CLIENT_PROG): main.o $(OBJS)
	$(CC) main.o $(OBJS) $(CLIBS) -o $(CLIENT_PROG)

test_net_xfer: test_net_xfer.o $(TEST_NET_XFER_OBJS)
	$(CC) test_net_xfer.o $(TEST_NET_XFER_OBJS) $(CLIBS) -o test_net_xfer

test_http: test_http.o $(TEST_HTTP_OBJS)
	$(CC) test_http.o $(TEST_HTTP_OBJS) $(CLIBS) -o test_http

test_file_xfer: test_file_xfer.o $(TEST_FX_OBJS)
	$(CC) test_file_xfer.o $(TEST_FX_OBJS) $(CLIBS) -o test_file_xfer

tar:
	tar cf boinc_client.tar *.C *.h Makefile.in

clean:
	rm -f *.o $(PROGS) core dependencies config.cache

dependencies: *.C
	$(CC) -M *.C > dependencies

include dependencies
