#CXXFLAGS += -g
CXX ?= g++

ifdef ANDROID
	WORKER_RELEASE_SUFFIX =
else
	ifdef RELEASE_ARCH
		WORKER_RELEASE_SUFFIX = _$(WORKER_RELEASE_VERSION)_$(RELEASE_ARCH)-pc-linux-gnu
	else
		WORKER_RELEASE_SUFFIX = _$(WORKER_RELEASE_VERSION)_x86_64-pc-linux-gnu
	endif
endif

ifdef RELEASE_ARCH
  CFLAGS += -O3 -flto
  CXXFLAGS += -O3 -flto
  LDFLAGS += -O3 -flto -s -static-libstdc++
else
  CFLAGS += -O0 -g
  CXXFLAGS += -O0 -g
  LDFLAGS += -O0 -g
endif

PROGS = worker$(WORKER_RELEASE_SUFFIX)

all: $(PROGS)

clean: distclean

distclean:
	rm -f $(PROGS) $(addsuffix .exe, $(PROGS)) *.o

worker$(WORKER_RELEASE_SUFFIX): worker.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o worker$(WORKER_RELEASE_SUFFIX) worker.cpp

install: all

.PHONY: install all distclean clean
