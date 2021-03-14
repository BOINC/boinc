all: boinc_gahp

clean:
	rm -f boinc_gahp libcurl.a

distclean: clean

distclean-recursive: clean

boinc_gahp: boinc_gahp.cpp ../../lib/remote_submit.h ../../lib/remote_submit.cpp ../../svn_version.h ../../version.h
	$(CXX) $(CPPFLAGS) -static -pthread -g -O0 -I../../lib -I../.. \
	-o boinc_gahp boinc_gahp.cpp ../../lib/remote_submit.cpp \
	$(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a libcurl.a

test: test.cpp ../../lib/remote_submit.cpp ../../lib/remote_submit.h
	$(CXX) $(CPPFLAGS) -static -g -o test -I../../lib \
	test.cpp ../../lib/remote_submit.cpp \
	$(BOINC_LIB_DIR)/libboinc.a libcurl.a
