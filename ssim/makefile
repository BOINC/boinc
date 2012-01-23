all: ssim vda vda_transitioner

vda_lib.o: vda_lib.cpp vda_lib.h
	g++ -c vda_lib.cpp
ssim: ssim.cpp des.h vda_lib.o
	g++ -g -Wall -o ssim ssim.cpp vda_lib.o
vda_transitioner: vda_transitioner.cpp vda_lib.o
	g++ -g -Wall -o vda_transitioner vda_transitioner.cpp vda_lib.o
vda: vda.cpp vda_lib.o
	g++ -g -Wall -o vda vda.cpp vda_lib.o
