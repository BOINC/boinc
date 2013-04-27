CXXFLAGS += -g

PROGS = worker

all: $(PROGS)

clean: distclean

distclean:
	rm -f $(PROGS) *.o

worker: worker.o
	g++ $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o worker worker.o
