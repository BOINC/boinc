CXXFLAGS += -g
CXX ?= g++

PROGS = worker

all: $(PROGS)

clean: distclean

distclean:
	rm -f $(PROGS) *.o

worker: worker.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -static -o worker worker.o

install: all

.PHONY: install all distclean clean
