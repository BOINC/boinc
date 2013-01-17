all: boinc_gahp

boinc_gahp: boinc_gahp.cpp curl.cpp
	g++ -g -O0 -I../../lib \
	-o boinc_gahp boinc_gahp.cpp curl.cpp \
	-L../../lib -lboinc -lpthread -lcurl

curl_test: curl.cpp
	g++ -o curl_test curl.cpp -lcurl
