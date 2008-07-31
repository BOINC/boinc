CXXFLAGS = -g 

PROGS = worker

all: $(PROGS)

clean:
	rm $(PROGS)

worker: worker.o
	g++ $(CXXFLAGS) -o worker worker.o
