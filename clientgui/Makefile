CXX = $(shell wx-config --cxx)

CFLAGS = -g -I ../lib

PROGRAM = boinc_gui

all: $(PROGRAM)

OBJS = \
    BOINCBaseView.o \
    BOINCGUIApp.o \
    BOINCListCtrl.o \
    BOINCTaskCtrl.o \
    DlgAbout.o \
    DlgAttachProject.o \
    DlgConnection.o \
    DlgOptions.o \
    MainDocument.o \
    MainFrame.o \
    stdwx.o \
    ValidateAccountKey.o \
    ValidateURL.o \
    ViewMessages.o \
    ViewProjects.o \
    ViewResources.o \
    ViewTransfers.o \
    ViewWork.o \
    ../lib/gui_rpc_client.o \
    ../lib/parse.o \
    ../lib/mfile.o \
    ../lib/miofile.o \
    ../lib/filesys.o \
    ../lib/util.o

boinc_gui: $(OBJS)
	$(CXX) -o $(PROGRAM) $(OBJS) `wx-config --libs`

.cpp.o:
	$(CXX) -c $(CFLAGS) `wx-config --cxxflags` $<
