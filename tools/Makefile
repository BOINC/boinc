# Generated automatically from Makefile.in by configure.
VERSION = 1

MYSQL_INC = /usr/local/mysql/include

CFLAGS = -g -Wall  -DHAVE_LIBGEN=1 -DHAVE_LIBM=1 -DHAVE_LIBNSL=1 -DHAVE_LIBSOCKET=1 -DHAVE_LIBZ=1 -DHAVE_DIRENT_H=1 -DSTDC_HEADERS=1 -DHAVE_SYS_WAIT_H=1 -DHAVE_FCNTL_H=1 -DHAVE_STRINGS_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_UNISTD_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_VPRINTF=1 -DHAVE_GETHOSTNAME=1 -DHAVE_GETTIMEOFDAY=1 -DHAVE_MKDIR=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_STRSTR=1 -DHAVE_UNAME=1  \
    -DHOSTTYPE=\"@host@\" \
    -DVERSION=$(VERSION) \
    -I ../lib -I ../db -I $(MYSQL_INC)

CC = gcc $(CFLAGS)

CLIBS = -lz -lsocket -lnsl -lm -lgen 

OBJS = \
    backend_lib.o \
    process_result_template.o

PROGS = create_work add country_select
all: $(PROGS) $(OBJS)

MYSQL_DIR = /usr/local/mysql/lib
MYSQL_LIBS = \
-L $(MYSQL_DIR) \
-lmysqlclient -lz \
-lm $(NETLIBS)

LIBS = \
    backend_lib.o \
    process_result_template.o \
    ../lib/md5_file.o \
    ../lib/md5.o \
    ../db/db_mysql.o \
    ../db/mysql_util.o

.C.o:
	$(CC) -c -o $*.o $<

create_work: create_work.o $(LIBS)
	$(CC) create_work.o $(CLIBS) $(LIBS) $(MYSQL_LIBS) -o create_work

add: add.o $(LIBS)
	$(CC) add.o $(LIBS) $(CLIBS) $(MYSQL_LIBS) -o add

country_select: country_select.o ../lib/countries.o
	$(CC) country_select.o ../lib/countries.o -o country_select

clean:
	rm -f *.o create_work add core country_select dependencies

dependencies: *.C
	$(CC) -M *.C > dependencies

include dependencies

