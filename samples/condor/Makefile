all: boinc_gahp

boinc_gahp: boinc_gahp.cpp job_rpc.cpp job_rpc.h
	g++ -g -O0 -I../../lib \
	-o boinc_gahp boinc_gahp.cpp job_rpc.cpp \
	-L../../lib -lboinc -lpthread -lcurl

test: test.cpp job_rpc.cpp
	g++ -g -o test -I../../lib \
	test.cpp job_rpc.cpp \
	-L../../lib -lboinc -lcurl
