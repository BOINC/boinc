# Generated automatically from Makefile.in by configure.
VERSION = 1

CFLAGS = -g -Wall  -DHAVE_LIBGEN=1 -DHAVE_LIBM=1 -DHAVE_LIBNSL=1 -DHAVE_LIBSOCKET=1 -DHAVE_LIBZ=1 -DHAVE_DIRENT_H=1 -DSTDC_HEADERS=1 -DHAVE_SYS_WAIT_H=1 -DHAVE_FCNTL_H=1 -DHAVE_STRINGS_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_UNISTD_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_VPRINTF=1 -DHAVE_GETHOSTNAME=1 -DHAVE_GETTIMEOFDAY=1 -DHAVE_MKDIR=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_STRSTR=1 -DHAVE_UNAME=1  \
    -DHOSTTYPE=\"@host@\" \
    -DVERSION=$(VERSION)

CC = gcc $(CFLAGS) -I ../api

LIBS = ../api/api.o

CLIBS = -lz -lsocket -lnsl -lm -lgen 

APPS = upper_case concat uc_slow 1sec

.C.o:
	$(CC) -c -o $*.o $<
.c.o:
	$(CC) -c -o $*.o $<

all: $(APPS)

upper_case: upper_case.o
	$(CC) upper_case.o $(LIBS) -o upper_case

concat: concat.o
	$(CC) concat.o $(LIBS) -o concat

uc_slow: uc_slow.o
	$(CC) uc_slow.o $(LIBS) -o uc_slow

1sec: 1sec.o
	$(CC) 1sec.o $(LIBS) -o 1sec

clean:
	rm -f *.o $(APPS)

dependencies: *.C
	$(CC) -M *.C > dependencies

include dependencies

