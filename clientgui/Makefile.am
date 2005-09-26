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
    DlgAccountManagerStatus.cpp \
    DlgDialupCredentials.cpp \
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
    wizards/AccountInfoPage.cpp \
    wizards/AccountKeyPage.cpp \
    wizards/AccountManagerInfoPage.cpp \
    wizards/AccountManagerProcessingPage.cpp \
    wizards/AccountManagerPropertiesPage.cpp \
    wizards/AlreadyAttachedPage.cpp \
    wizards/AlreadyExistsPage.cpp \
    wizards/BOINCBaseWizard.cpp \
    wizards/CompletionErrorPage.cpp \
    wizards/CompletionPage.cpp \
    wizards/NoInternetConnectionPage.cpp \
    wizards/NotDetectedPage.cpp \
    wizards/NotFoundPage.cpp \
    wizards/ProjectInfoPage.cpp \
    wizards/ProjectProcessingPage.cpp \
    wizards/ProjectPropertiesPage.cpp \
    wizards/ProxyPage.cpp \
    wizards/UnavailablePage.cpp \
    wizards/WelcomePage.cpp \
    wizards/WizardAccountManager.cpp \
    wizards/WizardAttachProject.cpp \
    wizards/wizardex.cpp \
    hyperlink.cpp $(mac_sources)

EXTRA_DIST = *.h \
    res \
    msw \
    ../lib/error_numbers.h \
    locale $(mac_headers)

boinc_gui_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) $(CLIENTGUIFLAGS) -I../wizards
boinc_gui_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(CLIENTGUIFLAGS) -I../wizards
boinc_gui_LDADD = -L../lib -lboinc $(CLIENTGUILIBS)

all-local: client_gui-bin
client_gui-bin: @CLIENT_GUI_BIN_FILENAME@

win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win_config.h

# make a hard link to the client name.
@CLIENT_GUI_BIN_FILENAME@: boinc_gui
	rm -f $@
	rm -f $@.unmodified
	cp $? $@.unmodified
	@LN@ $? $@
	@STRIP@ $@

clean-local:
	rm -f @CLIENT_GUI_BIN_FILENAME@

.PHONY: version
