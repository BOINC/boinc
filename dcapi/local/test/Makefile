
#CC=cc -n32 -g -Xcpluscomm
CC=gcc -g
CPP=g++ -g

DC=../..

INC=-I/usr/local/include/glib-2.0 -I/usr/local/lib/glib-2.0/include -I${DC}/include
LIBDIR=-L${DC}/local -L${DC}/common -L/usr/local/lib
LIBS= -ldc-local -ldc-common -luuid -lglib-2.0
LIB=${LIBDIR} ${LIBS}

CLIENT_LIBS= -ldc-client-local -ldc-common
CLIENT_LIB=${LIBDIR} ${CLIENT_LIBS}

all: master client
#all: proba

master:
	${CPP} -o master master.c ${INC} ${LIB}

client:
	${CPP} -o execdir/uppercase-client uppercase-client.c ${INC} ${CLIENT_LIB}

proba:
	${CPP} -o proba proba.c

clean:
	rm -rf core *~ master execdir/uppercase-client test.log workdir proba


