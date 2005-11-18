
#CC=cc -n32 -g -Xcpluscomm
CC=gcc -g
AR=ar

INC=-I../include

LIBDC=libdc-local

all:
	${CC} -c dc.c cfg.c logger.c rm.c ${INC} 
	${AR} rc ${LIBDC}.a dc.o cfg.o logger.o rm.o

clean:
	rm -rf core *~ *.o ${LIBDC}.a


