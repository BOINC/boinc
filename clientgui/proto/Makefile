# Purpose: makefile for wxGuide-Demo
# Created 2003-09-18

CXX = $(shell wx-config --cxx)

PROGRAM = demo

OBJECTS = app.o filelist.o hyperlink.o prefdlg.o

vpath %.h src
vpath %.cpp src

# implementation

.SUFFIXES:	.o .cpp

.cpp.o :
	$(CXX) -c `wx-config --cxxflags` -o $@ $<

all:    $(PROGRAM)

$(PROGRAM):	$(OBJECTS)
	$(CXX) -o $(PROGRAM) $(OBJECTS) `wxgtk-2.4-config --libs`

clean:
	rm -f *.o
