all: ssim vda vdad

CCFLAGS = -I /usr/include/mysql -I ../db -I ../lib -I ../db

LIBS = ../sched/libsched.a ../lib/libboinc.a

MYSQL_LIBS = -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl

vda_lib.o: vda_lib.cpp vda_lib.h
	g++ -c $(CCFLAGS) vda_lib.cpp
vda_lib2.o: vda_lib2.cpp vda_lib.h
	g++ -c $(CCFLAGS) vda_lib2.cpp
ssim: ssim.cpp des.h vda_lib.o
	g++ -g $(CCFLAGS) -Wall -o ssim ssim.cpp vda_lib.o
vdad: vdad.cpp vda_lib.o vda_lib2.o
	g++ -g $(CCFLAGS) -Wall -o vdad vdad.cpp vda_lib.o vda_lib2.o $(LIBS) $(MYSQL_LIBS)
vda: vda.cpp vda_lib.o vda_lib2.o
	g++ -g $(CCFLAGS) -Wall -o vda vda.cpp vda_lib.o vda_lib2.o $(LIBS) $(MYSQL_LIBS)
