# Generated automatically from Makefile.in by configure.
VERSION = 1

CFLAGS = -g -Wall

CC = gcc $(CFLAGS)

PROGS = api_test

all: $(PROGS)

OBJS = \
    api_test.o \
    api.o

.C.o:
	$(CC) -c -o $*.o $<
.c.o:
	$(CC) -c -o $*.o $<

api_test: $(OBJS)
	$(CC) $(OBJS) -o api_test

clean:
	rm -f *.o $(PROGS) core dependencies

dependencies: *.C
	$(CC) -M *.C > dependencies

include dependencies
