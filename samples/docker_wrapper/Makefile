all: docker_wrapper

CXXFLAGS = -g -I../../lib -I../../api\
    -L../../lib -L../../api \
    -Wformat-overflow=0

docker_wrapper: docker_wrapper.cpp
	g++ $(CXXFLAGS) docker_wrapper.cpp \
	-lboinc_api -lboinc \
    -o docker_wrapper
