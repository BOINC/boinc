# Generated automatically from Makefile.in by configure.
VERSION = 1

MYSQL_DIR = /usr/local/mysql/include

CFLAGS = -g -Wall  -DHAVE_LIBGEN=1 -DHAVE_LIBM=1 -DHAVE_LIBNSL=1 -DHAVE_LIBSOCKET=1 -DHAVE_LIBZ=1 -DHAVE_DIRENT_H=1 -DSTDC_HEADERS=1 -DHAVE_SYS_WAIT_H=1 -DHAVE_FCNTL_H=1 -DHAVE_STRINGS_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_UNISTD_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_VPRINTF=1 -DHAVE_GETHOSTNAME=1 -DHAVE_GETTIMEOFDAY=1 -DHAVE_MKDIR=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_STRSTR=1 -DHAVE_UNAME=1  \
    -DHOSTTYPE=\"@host@\" \
    -DVERSION=$(VERSION) \
    -I $(MYSQL_DIR) \
    -pedantic

CC = gcc $(CFLAGS)

LIBS = ../api/api.o

CLIBS = -lz -lsocket -lnsl -lm -lgen 

.C.o:
	$(CC) -c -o $*.o $<

all: db_mysql.o mysql_util.o

clean:
	rm -f dependencies *.o

dependencies: *.C
	$(CC) -M *.C > dependencies

include dependencies

