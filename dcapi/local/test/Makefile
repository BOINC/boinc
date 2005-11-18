
#CC=cc -n32 -g -Xcpluscomm
CC=gcc -g

DC=../..

INC=-I${DC}/include 
LIBDIR=-L${DC}/local
LIBS= -ldc

LIB=${LIBDIR} ${LIBS}

all: test_init

test_init:
	${CC} -o test_init test_init.c ${INC} ${LIB}



clean:
	rm -rf core *~ test_init 


