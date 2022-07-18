CXXFLAGS += -g
CXX ?= g++

PROGS = worker

all: $(PROGS)

clean: distclean

distclean:
	rm -f $(PROGS) $(addsuffix .exe, $(PROGS)) *.o

worker: worker.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o worker worker.o

install: all

.PHONY: install all distclean clean
