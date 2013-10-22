all: boinc_gahp

clean:
	rm boinc_gahp

boinc_gahp: boinc_gahp.cpp ../../lib/remote_submit.h ../../lib/remote_submit.cpp
	g++ -g -O0 -I../../lib \
	-o boinc_gahp boinc_gahp.cpp ../../lib/remote_submit.cpp \
	-L../../lib -lboinc -lpthread -lcurl

test: test.cpp ../../lib/remote_submit.cpp ../../lib/remote_submit.h
	g++ -g -o test -I../../lib \
	test.cpp ../../lib/remote_submit.cpp \
	-L../../lib -lboinc -lcurl
