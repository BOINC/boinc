## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

LIBS += @CLIENTLIBS@

bin_PROGRAMS = boinc_gui

boinc_gui_SOURCES = \
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
    ../lib/util.C \
    ../lib/md5_file.C \
    ../lib/md5.c

EXTRA_DIST = BOINCBaseView.h  DlgAttachProject.h  ValidateAccountKey.h  ViewWork.h 	\
	BOINCGUIApp.h	 DlgConnection.h     ValidateURL.h	   _wx_intellisense.h 		\
	BOINCListCtrl.h  DlgOptions.h	     ViewMessages.h	   resource.h				\
	BOINCTaskBar.h	 Events.h	     ViewProjects.h	   stdwx.h						\
	BOINCTaskCtrl.h  MainDocument.h      ViewResources.h							\
	DlgAbout.h	 MainFrame.h	     ViewTransfers.h								\
	res  msw locale

boinc_gui_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) $(CLIENTGUIFLAGS)
boinc_gui_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(CLIENTGUIFLAGS)
boinc_gui_LDADD = $(CLIENTGUILIBS)

all-local: client_gui-bin
client_gui-bin: @CLIENT_GUI_BIN_FILENAME@

win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win_config.h

# make a hard link to the client name.
@CLIENT_GUI_BIN_FILENAME@: boinc_gui
	rm -f $@
	ln $? $@

clean-local:
	rm -f @CLIENT_GUI_BIN_FILENAME@

.PHONY: version
