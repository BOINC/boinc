## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

# by default, "-static -static-libgcc" on linux.
#STATIC_FLAGS=@STATIC_FLAGS@

bin_PROGRAMS = boincgui

boincgui_SOURCES = \
    BOINCBaseView.cpp \
    BOINCGUIApp.cpp \
    BOINCListCtrl.cpp \
    BOINCTaskCtrl.cpp \
    DlgAbout.cpp \
    DlgAttachProject.cpp \
    DlgConnection.cpp \
    DlgOptions.cpp \
    MainDocument.cpp \
    MainFrame.cpp \
    stdwx.cpp \
    ValidateAccountKey.cpp \
    ValidateURL.cpp \
    ViewMessages.cpp \
    ViewProjects.cpp \
    ViewResources.cpp \
    ViewTransfers.cpp \
    ViewWork.cpp \
    ../lib/gui_rpc_client.C \
    ../lib/filesys.C \
    ../lib/mfile.C \
    ../lib/miofile.C \
    ../lib/parse.C \
    ../lib/util.C

boincgui_CPPFLAGS = $(AM_CPPFLAGS) -O3

all-local: client-bin
client-bin: @CLIENT_BIN_FILENAME@

win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win_config.h

# make a hard link to the client name.
@CLIENT_BIN_FILENAME@: boincgui
	rm -f $@
	ln $? $@

clean-local:
	rm -f @CLIENT_BIN_FILENAME@

.PHONY: version
