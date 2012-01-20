all: ssim

vda.o: vda.cpp vda.h
	g++ -c vda.cpp
ssim: ssim.cpp des.h vda.o
	g++ -g -Wall -o ssim ssim.cpp vda.o

