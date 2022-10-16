## -*- mode: makefile; tab-width: 4 -*-
## $Id$

## To build for Macintosh (Darwin), please read the instructions at
## mac_build/HowToBuildBOINC_XCode.rtf

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
    common/wxPieCtrl.cpp \
    CompletionErrorPage.cpp \
    CompletionPage.cpp \
    DlgAbout.cpp 		\
    DlgAdvPreferences.cpp \
    DlgAdvPreferencesBase.cpp \
    DlgDiagnosticLogFlags.cpp \
    DlgEventLog.cpp \
    DlgEventLogListCtrl.cpp \
    DlgExclusiveApps.cpp \
    DlgExitMessage.cpp \
    DlgGenericMessage.cpp \
    DlgHiddenColumns.cpp \
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
    ProjectProcessingPage.cpp \
    ProjectPropertiesPage.cpp \
    ProjectWelcomePage.cpp \
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
    WizardAttach.cpp \
    wizardex.cpp

EXTRA_DIST = *.h \
    res \
    msw \
    ../lib/error_numbers.h \
    locale

boincmgr_CPPFLAGS = $(AM_CPPFLAGS) $(WX_CPPFLAGS) $(LIBNOTIFY_CFLAGS) $(CLIENTGUIFLAGS) $(GTK_CFLAGS)
boincmgr_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(LIBNOTIFY_CFLAGS) $(CLIENTGUIFLAGS) $(GTK_CFLAGS)
boincmgr_LDADD = $(LIBBOINC) $(LIBNOTIFY_LIBS) $(CLIENTGUILIBS) $(BOINC_EXTRA_LIBS) $(CLIENTLIBS) $(GTK_LIBS)

win_config.h: $(top_srcdir)/config.h
	grep '#define.*BOINC.*VERSION' $^ > $@

version: win_config.h

.PHONY: version
