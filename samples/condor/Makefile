all: boinc_gahp

clean:
	rm -f boinc_gahp

distclean: clean

distclean-recursive: clean

boinc_gahp: boinc_gahp.cpp ../../lib/remote_submit.h ../../lib/remote_submit.cpp ../../svn_version.h ../../version.h
	g++ -g -O2 -I../../lib -I../.. \
	-o boinc_gahp boinc_gahp.cpp ../../lib/remote_submit.cpp \
	-L../../lib -lboinc -lpthread -lcurl

test: test.cpp ../../lib/remote_submit.cpp ../../lib/remote_submit.h
	g++ -g -o test -I../../lib \
	test.cpp ../../lib/remote_submit.cpp \
	-L../../lib -lboinc -lcurl
