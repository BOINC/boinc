# This should work on Linux.  Modify as needed for other platforms.

# Do this first:
# ln -s `g++ -print-file-name=libstdc++.a`
# This creates a symbolic link to the C++ library,
# which is linked statically

CXXFLAGS = -g \
    -I ../../boinc \
    -I ../../boinc/lib \
    -I ../../boinc/api \
    -L ../../boinc/api \
    -L ../../boinc/lib \
    -L.

PROGS = sleeper

all: $(PROGS)

clean:
	rm $(PROGS)

sleeper: sleeper.o
	g++ $(CXXFLAGS) -o sleeper sleeper.o libstdc++.a -pthread -lboinc_api -lboinc
