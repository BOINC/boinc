CXXFLAGS += -g
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

PROGS = worker$(WORKER_RELEASE_SUFFIX)

all: $(PROGS)

clean: distclean

distclean:
	rm -f $(PROGS) $(addsuffix .exe, $(PROGS)) *.o

worker$(WORKER_RELEASE_SUFFIX): worker.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o worker$(WORKER_RELEASE_SUFFIX) worker.o

install: all

.PHONY: install all distclean clean
