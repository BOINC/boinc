
#CC=cc -n32 -g -Xcpluscomm
#CC=gcc-3.4 -g
#CPP=g++-3.4 -g
CC=gcc -g
CPP=g++ -g
AR=ar

BOINC=/home/boinc/boinc_public

INC=-I../include 
BOINC_INC=-I${BOINC}/tools -I${BOINC}/sched -I${BOINC}/db -I${BOINC}/lib -I${BOINC}/RSAEuro/source -I/usr/include/mysql



all:
	${CC} -c -Wall dc.c cfg.c logger.c result.c ${INC} 
	$(CPP) -c -Wall validate_util.C ${INC} ${BOINC_INC}
	${CPP} -c -Wall wu.C assimilator.C ${INC} ${BOINC_INC}
	${AR} rc libdc-boinc-local.a dc.o cfg.o logger.o result.o wu.o assimilator.o validate_util.o

clean:
	rm -rf core *~ *.o libdc-boinc-local.a


