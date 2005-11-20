
#CC=cc -n32 -g -Xcpluscomm
#CC=gcc-3.4 -g
#CPP=g++-3.4 -g
CC=gcc -g
CPP=g++ -g
AR=ar
RANLIB=ranlib

# User configurable items
BOINC=../../boinc
MYSQL_INC=/sw/include/mysql

INC=-I../include -I/usr/local/include
BOINC_INC=-I${BOINC}/tools -I${BOINC}/sched -I${BOINC}/db -I${BOINC}/lib -I${MYSQL_INC} 

all:
	${CC} -c -Wall dc.c cfg.c logger.c result.c ${INC}
	$(CPP) -c -Wall validate_util.C ${INC} ${BOINC_INC} 
	${CPP} -c -Wall wu.C assimilator.C ${INC} ${BOINC_INC}
	${AR} rc libdc-boinc-local.a dc.o cfg.o logger.o result.o wu.o assimilator.o validate_util.o
	${RANLIB} libdc-boinc-local.a

clean:
	rm -rf core *~ *.o libdc-boinc-local.a


