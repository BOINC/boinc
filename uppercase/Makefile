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
    -L /usr/X11R6/lib \
    -L.

# the following should be freeglut; use nm to check
LIBGLUT = /usr/local/lib/libglut.a
LIBGLU = /usr/X11R6/lib/libGLU.a
LIBJPEG = /usr/lib/libjpeg.a

PROGS = upper_case upper_case.so

all: $(PROGS)

clean:
	rm $(PROGS)

# the -Wl,--export-dynamic causes the main program's symbols
# to be exported to the graphics library

upper_case: upper_case.o
	g++ $(CXXFLAGS) -Wl,--export-dynamic -o upper_case upper_case.o libstdc++.a -pthread -lboinc_api -lboinc -lboinc_graphics_lib -ldl

upper_case.so: uc_graphics.o
	g++ $(CXXFLAGS) -o upper_case.so \
    -shared -fPIC -pthread \
    uc_graphics.o \
    libstdc++.a \
    -lboinc_graphics_impl -lboinc \
    $(LIBGLUT) $(LIBGLU) $(LIBJPEG) \
    -lGL -lX11 -lXmu -lm \
