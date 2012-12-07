## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

if ENABLE_CLIENT_RELEASE
  AM_LDFLAGS += -static-libtool-libs
## for an entirely statically linked library, you may want to try
## -all-static instead.  There's a good chance it won't work properly,
## so we'll use the safer "-static-libtool-libs" by default.
else
if DYNAMIC_CLIENT
## if libtool starts to need flags for dynamic linking, add them here
else
  AM_LDFLAGS += -static
endif
endif ## ENABLE_CLIENT_RELEASE

if OS_DARWIN
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Cocoa -framework Security
   mac_sources = mac/SystemMenu.m \
                 mac/mac_saver_module.cpp \
                 mac/Mac_Saver_ModuleView.m \
                 mac/MacBitmapComboBox.cpp \
                 mac/MacSysMenu.cpp \
                 mac/Mac_GUI.cpp\
                 mac/browser_safari.mm

   mac_headers = mac/*.h
else
   mac_headers =
   mac_sources = 
endif

bin_PROGRAMS = boincmgr

# keep the following alphabetic

boincmgr_SOURCES = \
    AccountInfoPage.cpp \
    AccountManagerInfoPage.cpp \
    AccountManagerProcessingPage.cpp \
    AccountManagerPropertiesPage.cpp \
    AdvancedFrame.cpp \
    AlreadyExistsPage.cpp \
    AsyncRPC.cpp \
    BOINCBaseFrame.cpp \
    BOINCBaseView.cpp \
    BOINCBaseWizard.cpp \
    BOINCBitmapComboBox.cpp \
    BOINCClientManager.cpp \
    BOINCDialupManager.cpp \
    BOINCGUIApp.cpp \
    BOINCHtmlLBox.cpp \
    BOINCInternetFSHandler.cpp \
    BOINCListCtrl.cpp \
    BOINCTaskBar.cpp \
    BOINCTaskCtrl.cpp \
    browser.cpp \
    CompletionErrorPage.cpp \
    CompletionPage.cpp \
    common/wxPieCtrl.cpp \
    DlgAbout.cpp 		\
    DlgAdvPreferences.cpp \
    DlgAdvPreferencesBase.cpp \
    DlgEventLog.cpp \
    DlgEventLogListCtrl.cpp \
    DlgExitMessage.cpp \
    DlgGenericMessage.cpp \
    DlgItemProperties.cpp \
    DlgOptions.cpp \
    DlgSelectComputer.cpp \
    gtk/taskbarex.cpp \
    LogBOINC.cpp \
    MainDocument.cpp \
    NoInternetConnectionPage.cpp \
    NotDetectedPage.cpp \
    NotFoundPage.cpp \
    NoticeListCtrl.cpp \
    ProjectInfoPage.cpp \
    ProjectListCtrl.cpp \
    ProjectProcessingPage.cpp \
    ProjectPropertiesPage.cpp \
    ProxyInfoPage.cpp \
    ProxyPage.cpp \
    sg_BoincSimpleFrame.cpp \
    sg_CustomControls.cpp \
    sg_DlgMessages.cpp \
    sg_DlgPreferences.cpp \
    sg_PanelBase.cpp \
    sg_ProjectCommandPopup.cpp \
    sg_ProjectPanel.cpp \
    sg_ProjectWebSitesPopup.cpp \
    sg_TaskCommandPopup.cpp \
    sg_TaskPanel.cpp \
    SkinManager.cpp \
    stdwx.cpp \
    TermsOfUsePage.cpp \
    UnavailablePage.cpp \
    ValidateAccountKey.cpp \
    ValidateEmailAddress.cpp \
    ValidateURL.cpp \
    ViewNotices.cpp \
    ViewProjects.cpp \
    ViewResources.cpp \
    ViewStatistics.cpp \
    ViewTransfers.cpp \
    ViewWork.cpp \
    WelcomePage.cpp \
    WizardAttach.cpp \
    wizardex.cpp \
    $(mac_sources)

EXTRA_DIST = *.h \
    res \
    msw \
    ../lib/error_numbers.h \
    locale $(mac_headers)

boincmgr_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) $(SQLITE3_CPPFLAGS) $(LIBNOTIFY_CFLAGS) $(CLIENTGUIFLAGS) `pkg-config --cflags gtk+-2.0`
boincmgr_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(SQLITE3_CPPFLAGS) $(LIBNOTIFY_CFLAGS) $(CLIENTGUIFLAGS) `pkg-config --cflags gtk+-2.0`
boincmgr_LDFLAGS = $(LIBBOINC) $(SQLITE3_LIBS) $(LIBNOTIFY_LIBS) $(CLIENTGUILIBS) $(BOINC_EXTRA_LIBS) $(CLIENTLIBS) `pkg-config --libs gtk+-2.0` -lnotify

win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win_config.h

.PHONY: version
