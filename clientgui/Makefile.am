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
    BOINCClientManager.cpp \
    BOINCDialupManager.cpp \
    BOINCGUIApp.cpp \
    BOINCListCtrl.cpp \
    BOINCTaskBar.cpp \
    BOINCTaskCtrl.cpp \
    browser.cpp \
    CompletionErrorPage.cpp \
    CompletionPage.cpp \
    common/wxPieCtrl.cpp \
    common/wxFlatNotebook.cpp \
    DlgAbout.cpp 		\
    DlgAdvPreferences.cpp \
    DlgAdvPreferencesBase.cpp \
    DlgExitMessage.cpp \
    DlgGenericMessage.cpp \
    DlgItemProperties.cpp \
    DlgOptions.cpp \
    DlgSelectComputer.cpp \
    hyperlink.cpp \
    LogBOINC.cpp \
    MainDocument.cpp \
    NoInternetConnectionPage.cpp \
    NotDetectedPage.cpp \
    NotFoundPage.cpp \
    ProjectInfoPage.cpp \
    ProjectListCtrl.cpp \
    ProjectProcessingPage.cpp \
    ProjectPropertiesPage.cpp \
    ProxyInfoPage.cpp \
    ProxyPage.cpp \
    sg_BoincSimpleGUI.cpp \
    sg_ClientStateIndicator.cpp \
    sg_CustomControls.cpp \
    sg_DlgMessages.cpp \
    sg_DlgPreferences.cpp \
    sg_ImageButton.cpp \
    sg_ImageLoader.cpp \
    sg_ProgressBar.cpp \
    sg_ProjectsComponent.cpp \
    sg_SGUIListControl.cpp \
    sg_StatImageLoader.cpp \
    sg_ViewTabPage.cpp \
    SkinManager.cpp \
    stdwx.cpp \
    TermsOfUsePage.cpp \
    UnavailablePage.cpp \
    ValidateAccountKey.cpp \
    ValidateEmailAddress.cpp \
    ValidateURL.cpp \
    ViewMessages.cpp \
    ViewNews.cpp \
    ViewProjects.cpp \
    ViewResources.cpp \
    ViewStatistics.cpp \
    ViewTransfers.cpp \
    ViewWork.cpp \
    WelcomePage.cpp \
    WizardAttachProject.cpp \
    wizardex.cpp \
    $(mac_sources)

EXTRA_DIST = *.h \
    res \
    msw \
    ../lib/error_numbers.h \
    locale $(mac_headers)

boincmgr_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) $(CLIENTGUIFLAGS)
boincmgr_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(CLIENTGUIFLAGS)
boincmgr_LDADD = $(LIBBOINC) $(CLIENTGUILIBS) $(BOINC_EXTRA_LIBS) $(CLIENTLIBS)

win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win_config.h

.PHONY: version
