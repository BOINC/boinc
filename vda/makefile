all: ssim vda vdad

CCFLAGS = -I /usr/include/mysql -I ../db -I ../lib

LIBS = ../sched/libsched.a ../lib/libboinc.a

MYSQL_LIBS = -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

vda_lib.o: vda_lib.cpp vda_lib.h
	g++ -c vda_lib.cpp
ssim: ssim.cpp des.h vda_lib.o
	g++ -g -Wall -o ssim ssim.cpp vda_lib.o
vdad: vdad.cpp vda_lib.o
	g++ -g $(CCFLAGS) -Wall -o vdad vdad.cpp vda_lib.o $(LIBS) $(MYSQL_LIBS)
vda: vda.cpp vda_lib.o
	g++ -g $(CCFLAGS) -Wall -o vda vda.cpp vda_lib.o $(LIBS) $(MYSQL_LIBS)
