CXXFLAGS += -g
CXX ?= g++

PROGS = worker

all: $(PROGS)

clean: distclean

distclean:
	rm -f $(PROGS) *.o

worker: worker.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o worker worker.o

install: all
	@echo "--- The example app $(PROGS) is not installed ---"

.PHONY: install all distclean clean
