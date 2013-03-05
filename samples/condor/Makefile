all: boinc_gahp

boinc_gahp: boinc_gahp.cpp boinc_gahp.h curl.cpp curl.h
	g++ -g -O0 -I../../lib \
	-o boinc_gahp boinc_gahp.cpp curl.cpp \
	-L../../lib -lboinc -lpthread -lcurl

test: test.cpp curl.cpp
	g++ -g -o test -I../../lib \
	test.cpp curl.cpp -lcurl
