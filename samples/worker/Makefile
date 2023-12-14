CXXFLAGS += -g
CXX ?= g++

WORKER_RELEASE = 2_x86_64-pc-linux-gnu

PROGS = worker_$(WORKER_RELEASE)

all: $(PROGS)

clean: distclean

distclean:
	rm -f $(PROGS) $(addsuffix .exe, $(PROGS)) *.o

worker_$(WORKER_RELEASE): worker.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o worker_$(WORKER_RELEASE) worker.o

install: all

.PHONY: install all distclean clean
