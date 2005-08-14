## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

LIBS += @CLIENTLIBS@


if OS_DARWIN
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Cocoa
   mac_sources = mac/SystemMenu.m \
                 mac/mac_saver_module.cpp \
                 mac/Mac_Saver_ModuleView.m \
                 mac/MacSysMenu.cpp 
    mac_headers = mac/*.h
else
   mac_headers =
   mac_sources = 
endif

bin_PROGRAMS = boinc_gui

boinc_gui_SOURCES = \
    BOINCBaseView.cpp \
    BOINCGUIApp.cpp \
    BOINCListCtrl.cpp \
    BOINCTaskBar.cpp \
    BOINCTaskCtrl.cpp \
    DlgAbout.cpp \
    DlgAccountManagerSignup.cpp \
    DlgAccountManagerStatus.cpp \
    DlgDialupCredentials.cpp \
    DlgAttachProject.cpp \
    DlgOptions.cpp \
    DlgSelectComputer.cpp \
    LogBOINC.cpp \
    MainDocument.cpp \
    MainFrame.cpp \
    stdwx.cpp \
    ValidateAccountKey.cpp \
    ValidateURL.cpp \
    ViewMessages.cpp \
    ViewProjects.cpp \
    ViewResources.cpp \
    ViewStatistics.cpp \
    ViewTransfers.cpp \
    ViewWork.cpp \
    WizAttachProject.cpp \
    hyperlink.cpp $(mac_sources)

EXTRA_DIST = *.h \
    res \
    msw \
    ../lib/error_numbers.h \
    locale $(mac_headers)

boinc_gui_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) $(CLIENTGUIFLAGS)
boinc_gui_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(CLIENTGUIFLAGS)
boinc_gui_LDADD = -L../lib -lboinc $(CLIENTGUILIBS)

all-local: client_gui-bin
client_gui-bin: @CLIENT_GUI_BIN_FILENAME@

win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win_config.h

# make a hard link to the client name.
@CLIENT_GUI_BIN_FILENAME@: boinc_gui
	rm -f $@
	@LN@ $? $@
	@STRIP@ $@

clean-local:
	rm -f @CLIENT_GUI_BIN_FILENAME@

.PHONY: version
