//////////////////////////////////////////////////////////////////////////////
// File:        app.cpp
// Purpose:     Demo application
// Maintainer:  Otto Wyss
// Created:     2003-10-10
// RCS-ID:      $Id$
// Copyright:   (c) wxGuide
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------

// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all 'standard' wxWindows headers)
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

//! wxWindows headers
#include <wx/accel.h>    // accelerator support
#include <wx/cmdline.h>  // command line support
#include <wx/config.h>   // configuration support
#include <wx/dnd.h>      // drag&drop support
#include <wx/dynarray.h> // dynamic array support
#include <wx/filename.h> // filename support
#include <wx/fs_zip.h>   // ZIP filesystem support
#include <wx/html/helpctrl.h> // Html help support
#include <wx/intl.h>     // internationalisation
#include <wx/ipc.h>      // IPC support
#include <wx/log.h>      // log message support
#include <wx/notebook.h> // notebook support
#include <wx/toolbar.h>  // toolbars support
#include <wx/settings.h> // system settings
#include <wx/snglinst.h> // single instance checker
#include <wx/statline.h> // static line
#include <wx/string.h>   // strings support
#include <wx/listctrl.h> // list control support

//! application headers
#include "defsext.h"     // Additional definitions
#include "hyperlink.h"   // HyperLink control
#include "private.h"     // Platform specific definitions


//----------------------------------------------------------------------------
// resources
//----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources)
#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__) || defined(__WXMGL__) || defined(__WXX11__)
    #include "app.xpm"
#endif

//============================================================================
// declarations
//============================================================================

const wxString APP_NAME = _T("Graphical User Interface");
const wxString APP_DESCR = _("\
BOINC Universal GUI.");

const wxString APP_MAINT = _T("BOINC Development Staff");
const wxString APP_VENDOR = _T("BOINC");
const wxString APP_COPYRIGTH = _T("");
const wxString APP_LICENCE = _T("wxWindows");

const wxString APP_VERSION = _T("2.");
const wxString APP_BUILD = _T(__DATE__);

const wxString APP_WEBSITE = _T("http://wxGuide.sourceforge.net");
const wxString APP_MAIL = _T("mailto://wyo@@users.sourceforge.net");

const wxString IPC_START = "StartOther";

const wxString LOCATION = _T("Location");
const wxString LOCATION_X = _T("xpos");
const wxString LOCATION_Y = _T("ypos");
const wxString LOCATION_W = _T("width");
const wxString LOCATION_H = _T("height");

const wxString RECENTFILES = _T("Recent Files/file");

class AppBook;


//----------------------------------------------------------------------------
//! global application name
wxString g_appname;

//! global help provider
wxHtmlHelpController *g_help = NULL;

//! global status text
wxString g_statustext;


//----------------------------------------------------------------------------
//! application APP_VENDOR-APP_NAME.
class App: public wxApp {
    friend class AppFrame;
    friend class AppIPCConnection;

public:
    //! the main function called durning application start
    virtual bool OnInit ();

    //! application exit function
    virtual int OnExit ();

private:
    //! object used to check if another program instance is running
    wxSingleInstanceChecker *m_singleInstance;

    //! the wxIPC server
    wxServerBase *m_serverIPC;

    //! variable for internationalization
    wxLocale m_locale;

    //! frame window
    WX_DEFINE_ARRAY (AppFrame*, AppFrames);
    AppFrames m_frames;
    void CreateFrame (wxArrayString *fnames);
    void RemoveFrame (AppFrame *frame);
    bool ProcessRemote (char** argv, int argc = 0);

    //! commandline
    wxArrayString *m_fnames;
    bool ProcessCmdLine (char** argv, int argc = 0);

};

// created dynamically by wxWindows
DECLARE_APP (App);

//----------------------------------------------------------------------------
//! IPC connection for application APP_VENDOR-APP_NAME.
class AppIPCConnection : public wxConnection {

public:
    //! application IPC connection
    AppIPCConnection(): wxConnection (m_buffer, WXSIZEOF(m_buffer)) {
    }

    //! execute handler
    virtual bool OnExecute (const wxString& WXUNUSED(topic),
                            char *data,
                            int size,
                            wxIPCFormat WXUNUSED(format)) {
        char** argv;
        int argc = 0;
        int i;
        for (i=0; i<size; i++) {
            if ((i > 0) && (data[i] == '\0') && (data[i-1] == '\0')) break;
            if (data[i] == '\0') argc++;
        }
        argv = new char*[argc];
        int p = 0;
        char* temp = data;
        for (i=0; i<argc; i++) {
            argv[i] = new char [strlen (temp) + 1];
            strcpy (argv[i], temp);
            p = strlen (temp) + 1;
            temp += p;
        }
        bool ok = wxGetApp().ProcessRemote (argv, argc);
        for (i=0; i<argc; i++) {
            delete [] argv[i];
        }
        delete [] argv;
        return ok;
    }

private:
   // character buffer
   char m_buffer[4096];

};

//----------------------------------------------------------------------------
//! IPC server for application APP_VENDOR-APP_NAME.
class AppIPCServer : public wxServer {

public:
    //! accept conncetion handler
    virtual wxConnectionBase *OnAcceptConnection (const wxString& topic) {
        if (topic != IPC_START) return NULL;
        return new AppIPCConnection;
    }
};

//----------------------------------------------------------------------------
//! frame of the application APP_VENDOR-APP_NAME.
class AppFrame: public wxFrame {
    friend class App;
    friend class AppBook;
    friend class AppAbout;
    friend class AllWindows;
    friend class DropFiles;

public:
    //! constructor
    AppFrame (const wxString &title,
              const wxArrayString &fnames,
              int frameNr);

    //! destructor
    ~AppFrame ();

    //! event handlers
    //! common
    void OnActivate (wxActivateEvent &event);
    void OnActivateApp (wxActivateEvent &event);
    void OnClose (wxCloseEvent &event);
    void OnAbout (wxCommandEvent &event);
    void OnExit (wxCommandEvent &event);
    void OnHelp (wxCommandEvent &event);
    void OnIdle (wxIdleEvent &event);
    void OnTimerEvent (wxTimerEvent &event);
    //! file
    void OnFileClose (wxCommandEvent &event);
    //! notebook
    void OnPageChange (wxNotebookEvent &event);
    // demo events
    void OnDemoEvent (wxCommandEvent &event);
    //! demo UI events
    void OnDemoEventUI (wxUpdateUIEvent &event);
    //! view UI events
    void OnToolbarsUI (wxUpdateUIEvent &event);
    void OnStatusbarUI (wxUpdateUIEvent &event);
    //! windows update UI
    void OnPagePrevUI (wxUpdateUIEvent &event);
    void OnPageNextUI (wxUpdateUIEvent &event);
    //! others update UI
    void OnStatusBarUI (wxUpdateUIEvent &event);

    //! help controller
    wxHtmlHelpController& GetHelpController() {return *g_help; }

private:
    // activate state
    bool m_activateInProgress;

    // demo
    wxListCtrl *m_demo;
    int m_demoNr;
    void FileOpen (wxArrayString fnames);

    //! main window position and size
    int m_frameNr;
    wxRect DetermineFrameSize (wxConfig* config = NULL);
    void StoreFrameSize (wxRect rect, wxConfig* config = NULL);

    //! creates the application menu bar
    void CreateMenu ();

    //! creates status bar
    wxStatusBar *m_statusbar;
    void CreateStatusbar ();
    void DeleteStatusbar ();

    // print preview position and size
    wxRect DeterminePrintSize ();

    // update functions
    wxTimer *m_updateTimer;
    void UpdateStatustext ();
    void UpdateTitle ();

    //! notebook page
    AppBook *m_book;
    wxNotebookSizer *m_bookSizer;
    int m_pageNr;
    int m_hitNr;
    bool m_closeBook;
    void PageHasChanged (int pageNr = -1);
    int GetPageNr (const wxString &fname);

    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
//! notebook of the application APP_VENDOR-APP_NAME
class AppBook: public wxNotebook {

public:
    //! constructor
    AppBook (wxWindow *parent,
             wxWindowID id = -1,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = 0);

    //! destructor
    ~AppBook ();

    // event handlers
    void OnTabActivate (wxMouseEvent &event);

private:
    AppFrame *m_frame;

    // context menu of the tab region
    wxMenu *m_bookMenu1;
    wxMenu *m_bookMenu2;
    void CreateBookMenus ();

    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
//! about box of the application APP_VENDOR-APP_NAME
class AppAbout: public wxDialog {

public:
    //! constructor
    AppAbout (wxWindow *parent,
              int milliseconds = 0,
              long style = 0);

    //! destructor
    ~AppAbout ();

    // event handlers
    void OnHelp (wxCommandEvent &event);
    void OnTimerEvent (wxTimerEvent &event);

private:
    // timer
    wxTimer *m_timer;

    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
//! AllWindows
class AllWindows: public wxDialog {

public:
    //! constructor
    AllWindows (AppFrame *frame);

    // event handlers
    void OnSetActive (wxCommandEvent& event);
    void OnCancel (wxCommandEvent& event);
    void OnOkay (wxCommandEvent& event);

private:
    AppFrame *m_frame;
    wxListBox *m_windows;

    DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
//! drop files of the application APP_VENDOR-APP_NAME
class DropFiles: public wxFileDropTarget {

public:
    //! constructor
    DropFiles (AppFrame *frame) {m_frame = frame;}

    virtual bool OnDropFiles (wxCoord x, wxCoord y, const wxArrayString& filenames);

private:
    AppFrame *m_frame;
};

//----------------------------------------------------------------------------
//! printout of the application APP_NAME
class Printout: public wxPrintout {

public:

    //! Constructor
    Printout (wxChar *title = _T("Test printout")): wxPrintout(title) {}

    //! Event handlers
    bool OnPrintPage(int page);

    //! Print functions
    bool HasPage(int page);
    bool OnBeginDocument(int startPage, int endPage);
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);

};

//============================================================================
// implementation
//============================================================================

IMPLEMENT_APP (App)

//----------------------------------------------------------------------------
// App
//----------------------------------------------------------------------------

bool App::OnInit () {

    //_CrtSetBreakAlloc (<memory_number>);

    // set application and vendor name
    SetAppName (APP_NAME);
    SetVendorName (APP_VENDOR);
    g_appname = (APP_VENDOR);
    g_appname.Append (_T("-"));
    g_appname.Append (APP_NAME);

    // do essential initialization
    m_singleInstance = NULL;
    m_serverIPC = NULL;
    m_fnames = NULL;

    // Set and check for single instance running
    wxString name = g_appname + wxString::Format ("%s", wxGetUserId().c_str());
    m_singleInstance = new wxSingleInstanceChecker (name);
    if (m_singleInstance->IsAnotherRunning()) {
        wxClient client;
        wxConnectionBase *conn = client.MakeConnection ("", name + _T(".ipc"),
                                                        IPC_START);
        if (conn) {
            wxString dataStr;
            int i;
            for (i = 0; i < argc; ++i) {
                dataStr.Append (argv[i]);
                dataStr.Append (" ");
            }
            char data[4096];
            strcpy (data, dataStr.c_str());
            int size = 0;
            for (i = 0; i < argc; ++i) {
                size += strlen (argv[i]);
                data[size] = '\0';
                size += 1;
            }
            data[size] = '\0';
            size += 1;
            if (conn->Execute (data, size)) return false;
        }
        delete conn;
    }

    // IPC server
    m_serverIPC = new AppIPCServer ();
    if (!m_serverIPC->Create (name + _T(".ipc"))) {
        delete m_serverIPC;
        m_serverIPC = NULL;
    }

    // initialize localisazion
    m_locale.Init();
    m_locale.AddCatalog (APP_NAME);

    // initializes help (Html help needs the Zip filestytem!?!)
    wxFileSystem::AddHandler (new wxZipFSHandler);
    wxString path (wxGetCwd() + _T("\\"));
    g_help = new wxHtmlHelpController;
    g_help->Initialize (path + APP_NAME);

    // get and process command line
    m_fnames = new wxArrayString();
    ProcessCmdLine (argv, argc);

    // create application frame
    int nr = m_frames.GetCount();
    m_frames.Add (new AppFrame (g_appname, *m_fnames, nr));

    // open application frame
    m_frames[0]->Layout ();
    m_frames[0]->Show (true);
    SetTopWindow (m_frames[0]);

    return true;
}

int App::OnExit () {

    // delete single instance checker
    if (m_singleInstance) delete m_singleInstance;

    // delete IPC server
    if (m_serverIPC) delete m_serverIPC;

    // delete help
    if (g_help) delete g_help;

    // delete commandline fnames
    if (m_fnames) delete m_fnames;

    return 0;
}

bool App::ProcessCmdLine (char** argv, int argc) {

    // get and process command line
    static const wxCmdLineEntryDesc cmdLineDesc[] = {
        {wxCMD_LINE_PARAM,  NULL, NULL, _T("input files"),
         wxCMD_LINE_VAL_STRING,
         wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE},
        {wxCMD_LINE_NONE}
    };
    wxCmdLineParser parser (cmdLineDesc, argc, argv);

    // get filenames from the commandline
    m_fnames->Clear();
    if (parser.Parse() == 0) {
        for (size_t paramNr=0; paramNr < parser.GetParamCount(); paramNr++) {
            m_fnames->Add (parser.GetParam (paramNr));
        }
    }

    return true;
}

bool App::ProcessRemote (char** argv, int argc) {

    // process remote request
    ProcessCmdLine (argv, argc);
    AppFrame* frame = (AppFrame*)wxTheApp->GetTopWindow();
    frame->Raise();

    return true;
}

void App::CreateFrame (wxArrayString *fnames) {

    int nr = m_frames.GetCount();
    m_frames.Add (new AppFrame (g_appname, *fnames, nr));
    m_frames[nr]->Layout ();
    m_frames[nr]->Show (true);
    SetTopWindow (m_frames[nr]);
}

void App::RemoveFrame (AppFrame *frame) {
    m_frames.Remove (frame);
}

//----------------------------------------------------------------------------
// AppFrame
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (AppFrame, wxFrame)
    // common
    EVT_ACTIVATE (                   AppFrame::OnActivate)
    EVT_ACTIVATE_APP (               AppFrame::OnActivateApp)
    EVT_CLOSE (                      AppFrame::OnClose)
    EVT_IDLE (                       AppFrame::OnIdle)
    EVT_TIMER (myID_UPDATETIMER,     AppFrame::OnTimerEvent)
    // file
    EVT_MENU (wxID_EXIT,             AppFrame::OnExit)
    // help
    EVT_MENU (wxID_ABOUT,            AppFrame::OnAbout)
    EVT_MENU (wxID_HELP,             AppFrame::OnHelp)
    EVT_MENU (wxID_HELP_CONTENTS,    AppFrame::OnHelp)
    // extra update UI
    // window update UI
END_EVENT_TABLE ()

AppFrame::AppFrame (const wxString &title,
                    const wxArrayString &fnames,
                    int frameNr)
        : wxFrame ((wxFrame *)NULL, -1, title, wxDefaultPosition, wxDefaultSize,
                    wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE) {

    // intitialize important variables
    m_activateInProgress = false;
    m_demo = NULL;
    m_demoNr = 0;
    m_updateTimer = NULL;

    // set icon and background
    SetIcon (wxICON (app));
    SetBackgroundColour (_T("WHITE"));

    // set frame size
    m_frameNr = frameNr;
    SetSize (DetermineFrameSize ());

    // create menu
    CreateMenu();

    // initialize statusbar
    m_statusbar = NULL;
    CreateStatusbar ();

    m_updateTimer = new wxTimer (this, myID_UPDATETIMER);

    // create frame panel
    wxPanel *panel = new wxPanel (this, -1, wxDefaultPosition, wxDefaultSize,
                                 wxTAB_TRAVERSAL|wxCLIP_CHILDREN|wxNO_BORDER);

    // initialize notebook
    m_book = new AppBook (panel, -1, wxDefaultPosition, wxDefaultSize,
                          wxNB_FIXEDWIDTH);
    m_bookSizer = new wxNotebookSizer (m_book);
    m_pageNr = -1;
    m_hitNr = -1;
    m_closeBook = false;

    // layout frame panel
    wxBoxSizer *panelsizer = new wxBoxSizer (wxVERTICAL);
    panelsizer->Add (new wxStaticLine (panel, -1), 0, wxEXPAND);
    panelsizer->Add (0, 2);
    panelsizer->Add (m_bookSizer, 1, wxEXPAND);
    panel->SetSizerAndFit (panelsizer);

    // open first page
    m_demoNr += 1;
    m_demo = new wxListCtrl (m_book, -1, wxDefaultPosition,
        wxDefaultSize, wxLC_REPORT, wxDefaultValidator, "listCtrl");
    m_demo->InsertColumn(0, _T("Project"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(1, _T("Account"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(2, _T("Total Credit"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(3, _T("Avg. Credit"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(4, _T("Resource Share"), wxLIST_FORMAT_LEFT, -1);
    m_pageNr = m_book->GetPageCount();
    m_book->AddPage (m_demo, _T("Projects"), true);

    m_demoNr += 1;
    m_demo = new wxListCtrl (m_book, -1, wxDefaultPosition,
        wxDefaultSize, wxLC_REPORT, wxDefaultValidator, "listCtrl");
    m_demo->InsertColumn(0, _T("Project"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(1, _T("Application"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(2, _T("Name"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(3, _T("CPU time"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(4, _T("Progress"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(4, _T("To Completetion"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(4, _T("Report Deadline"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(4, _T("Status"), wxLIST_FORMAT_LEFT, -1);
    m_pageNr = m_book->GetPageCount();
    m_book->AddPage (m_demo, _T("Work"), true);

    m_demoNr += 1;
    m_demo = new wxListCtrl (m_book, -1, wxDefaultPosition,
        wxDefaultSize, wxLC_REPORT, wxDefaultValidator, "listCtrl");
    m_demo->InsertColumn(0, _T("Project"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(1, _T("File"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(2, _T("Progress"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(3, _T("Size"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(4, _T("Time"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(4, _T("Speed"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(4, _T("Status"), wxLIST_FORMAT_LEFT, -1);
    m_pageNr = m_book->GetPageCount();
    m_book->AddPage (m_demo, _T("Transfers"), true);

    m_demoNr += 1;
    m_demo = new wxListCtrl (m_book, -1, wxDefaultPosition,
        wxDefaultSize, wxLC_REPORT, wxDefaultValidator, "listCtrl");
    m_demo->InsertColumn(0, _T("Project"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(1, _T("Time"), wxLIST_FORMAT_LEFT, -1);
    m_demo->InsertColumn(2, _T("Message"), wxLIST_FORMAT_LEFT, -1);
    m_pageNr = m_book->GetPageCount();
    m_book->AddPage (m_demo, _T("Message"), true);

    m_demoNr += 1;
    m_demo = new wxListCtrl (m_book, -1, wxDefaultPosition,
        wxDefaultSize, wxLC_REPORT, wxDefaultValidator, "listCtrl");
    m_pageNr = m_book->GetPageCount();
    m_book->AddPage (m_demo, _T("Disk Cost"), true);



    // update information
    g_statustext = _("Welcome to the ") + g_appname + _(" application.");
}

AppFrame::~AppFrame () {
    if (m_updateTimer) {
        delete m_updateTimer;
        m_updateTimer = NULL;
    }
    delete m_book;
}

// common event handlers
void AppFrame::OnActivate (wxActivateEvent &event) {
    if (!event.GetActive() || m_activateInProgress) return;
    wxWindow *win = wxGetActiveWindow();
    if (win) ((App*)wxTheApp)->SetTopWindow (win);
    if (!m_demo) return;
    m_activateInProgress = true;
    // any processing?
    m_demo->SetFocus();
    m_activateInProgress = false;
}

void AppFrame::OnActivateApp (wxActivateEvent &event) {
    OnActivate (event);
}

void AppFrame::OnClose (wxCloseEvent &event) {
    wxCommandEvent evt;
    int i;
    int cnt = m_book->GetPageCount();
    m_closeBook = true;
    if (myIsKeyDown (WXK_CONTROL) && !myIsKeyDown ('Q')) {
        StoreFrameSize (GetRect ());
    }
    ((App*)wxTheApp)->RemoveFrame (this);
    Destroy();
}

void AppFrame::OnAbout (wxCommandEvent &WXUNUSED(event)) {
    AppAbout (this);
}

void AppFrame::OnExit (wxCommandEvent &WXUNUSED(event)) {
    Close (true);
}

void AppFrame::OnHelp (wxCommandEvent &WXUNUSED(event)) {
    wxWindow *active = wxGetActiveWindow();
    wxString helptext;
    while (active && helptext.IsEmpty()) {
        helptext = active->GetHelpText();
        active = GetParent();
    }
    g_help->DisplayContents();
}

void AppFrame::OnIdle (wxIdleEvent &event) {
    if (m_updateTimer && !m_updateTimer->IsRunning ()) {;
        m_updateTimer->Start (100, wxTIMER_ONE_SHOT);
        UpdateStatustext();
        UpdateTitle();
    }
    event.Skip();
}

void AppFrame::OnTimerEvent (wxTimerEvent &event) {
    if (m_updateTimer) m_updateTimer->Stop ();
}

void AppFrame::OnFileClose (wxCommandEvent &WXUNUSED(event)) {
    if (!m_demo) return;
}

// private functions
void AppFrame::CreateMenu () {

    // File menu
    wxMenu *menuFile = new wxMenu;
    menuFile->Append (wxID_CLOSE, _("&Close\tCtrl+W"));

    // Help menu
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append (wxID_HELP_CONTENTS, _("&Content\tF1"));
    menuHelp->Append (wxID_HELP, _("&Index\tF1"));
    menuHelp->AppendSeparator();
    menuHelp->Append (wxID_ABOUT, _("&About ..\tShift+F1"));

    // construct menu
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append (menuFile, _("&File"));
    menuBar->Append (menuHelp, _("&Help"));
    SetMenuBar (menuBar);

}

void AppFrame::CreateStatusbar () {
    if (m_statusbar) return;
    int ch = GetCharWidth();
    const int widths[] = {-1, 20*ch};
    m_statusbar = CreateStatusBar (WXSIZEOF(widths), wxST_SIZEGRIP, myID_STATUSBAR);
    m_statusbar->SetStatusWidths (WXSIZEOF(widths), widths);
    SendSizeEvent();
}

void AppFrame::DeleteStatusbar () {
    if (!m_statusbar) return;
    SetStatusBar (NULL);
    delete m_statusbar;
    m_statusbar = NULL;
    SendSizeEvent();
}

wxRect AppFrame::DetermineFrameSize (wxConfig* config) {

    const int minFrameWidth = 640;
    const int minFrameHight = 480;

    wxSize scr = wxGetDisplaySize();

    // determine default frame position/size
    wxRect normal;
    if (scr.x <= 640) {
        normal.x = (scr.x - 400) / 2;
        normal.width = 400;
    }else{
        normal.x = (scr.x - 480) / 2;
        normal.width = 480;
    }
    if (scr.y <= 640) {
        normal.y = (scr.y - 200) / 2;
        normal.height = 200;
        normal.y = (scr.y - 200) / 2;
        normal.height = 200;
    }else{
        normal.y = (scr.y - 240) / 2;
        normal.height = 240;
    }

    // load stored size or defaults
    wxRect rect = normal;
    wxConfig* cfg = config;
// wxFileConfig allows empty name might not be supported
#if APPNAME_FILECONFIG
    if (!config) cfg = new wxConfig ();
#else
    if (!config) cfg = new wxConfig (wxTheApp->GetAppName());
#endif
    int i;
    for (i = 0; i <= m_frameNr; i++) {
        wxString key = LOCATION + wxString::Format ("%d", m_frameNr - i);
        if (cfg->Exists (key)) {
            rect.x = cfg->Read (key + _T("/") + LOCATION_X, rect.x);
            rect.y = cfg->Read (key + _T("/") + LOCATION_Y, rect.y);
            rect.width = cfg->Read (key + _T("/") + LOCATION_W, rect.width);
            rect.height = cfg->Read (key + _T("/") + LOCATION_H, rect.height);
            break;
        }
    }
    if (!config) delete cfg;

    // check for reasonable values (within screen)
    rect.x = wxMin (abs (rect.x), (scr.x - minFrameWidth));
    rect.y = wxMin (abs (rect.y), (scr.y - minFrameHight));
    rect.width = wxMax (abs (rect.width), (minFrameWidth));
    rect.width = wxMin (abs (rect.width), (scr.x - rect.x));
    rect.height = wxMax (abs (rect.height), (minFrameHight));
    rect.height = wxMin (abs (rect.height), (scr.y - rect.y));

    return rect;
}

void AppFrame::StoreFrameSize (wxRect rect, wxConfig* config) {

    // store size
    wxConfig* cfg = config;
// wxFileConfig allows empty name might not be supported
#if APPNAME_FILECONFIG
    if (!config) cfg = new wxConfig ();
#else
    if (!config) cfg = new wxConfig (wxTheApp->GetAppName());
#endif
    wxString key = LOCATION + wxString::Format ("%d", m_frameNr);
    cfg->Write (key + _T("/") + LOCATION_X, rect.x);
    cfg->Write (key + _T("/") + LOCATION_Y, rect.y);
    cfg->Write (key + _T("/") + LOCATION_W, rect.width);
    cfg->Write (key + _T("/") + LOCATION_H, rect.height);
    if (!config) delete cfg;
}


int AppFrame::GetPageNr (const wxString &fname) {
    int pageNr;
    wxStaticText *demo;
    for (pageNr = 0; pageNr < (int)m_book->GetPageCount(); pageNr++) {
        demo = (wxStaticText *) m_book->GetPage (pageNr);
        if (demo && (demo->GetLabel() == fname)) return pageNr;
    }
    return -1;
}

void AppFrame::UpdateStatustext () {
    if (!m_statusbar) return;
    wxStatusBar* sb = GetStatusBar();
    if (g_statustext != sb->GetStatusText (0)) SetStatusText (g_statustext, 0);
    if (m_demo) {
        wxString text = m_book->GetPageText (m_pageNr);
        if (text != sb->GetStatusText (1)) SetStatusText (text, 1);
    }
}

void AppFrame::UpdateTitle () {
    wxString title = g_appname;
    if (m_demo) {
        wxString fname = m_demo->GetLabel();
        if (fname.IsEmpty()) fname = _T("Testing");
    }
    if (title != GetTitle()) SetTitle (title);
}

//----------------------------------------------------------------------------
// AppBook
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (AppBook, wxNotebook)
    EVT_RIGHT_UP ( AppBook::OnTabActivate)
END_EVENT_TABLE()

AppBook::AppBook (wxWindow *parent,
                  wxWindowID id,
                  const wxPoint& pos,
                  const wxSize& size,
                  long style)
       : wxNotebook (parent, id, pos, size, style) {

    // initialize frame pointer
    m_frame = (AppFrame*)wxTheApp->GetTopWindow();

    // create menu
    m_bookMenu1 = new wxMenu;
    m_bookMenu2 = new wxMenu;
// wxNotebook::HitTest might not be supported
#if NOTEBOOK_HITTEST
    CreateBookMenus ();
#endif

}

AppBook::~AppBook () {
    delete m_bookMenu1;
    delete m_bookMenu2;
}

//----------------------------------------------------------------------------
// event handlers
void AppBook::OnTabActivate (wxMouseEvent &event) {
    wxPoint pt;
    pt.x = event.GetX();
    pt.y = event.GetY();
// wxNotebook::HitTest might not be supported
#if NOTEBOOK_HITTEST
    long flags = 0;
    int pageNr = HitTest (pt, &flags);
    if (pageNr < 0) return;
    m_frame->m_hitNr = pageNr;
    if (pageNr == m_frame->m_pageNr) {
        PopupMenu (m_bookMenu1, pt);
    }else{
        PopupMenu (m_bookMenu2, pt);
    }
#endif
}

// private functions
void AppBook::CreateBookMenus () {

    wxMenuBar *menuBar = m_frame->GetMenuBar();
    wxString label;

    // create menu 1
    if (menuBar->FindItem (wxID_SAVE)) {
        label = menuBar->GetLabel (wxID_SAVE);
        m_bookMenu1->Append (wxID_SAVE, label);
    }
    if (menuBar->FindItem (wxID_SAVEAS)) {
        label = menuBar->GetLabel (wxID_SAVEAS);
        m_bookMenu1->Append (wxID_SAVEAS, label);
    }
    if (menuBar->FindItem (wxID_CLOSE)) {
        label = menuBar->GetLabel (wxID_CLOSE);
        m_bookMenu1->Append (wxID_CLOSE, label);
    }
    m_bookMenu1->AppendSeparator();
    if (menuBar->FindItem (wxID_PRINT)) {
        label = menuBar->GetLabel (wxID_PRINT);
        m_bookMenu1->Append (wxID_PRINT, label);
    }

    // create menu 2
    m_bookMenu2->Append (myID_PAGEACTIVE, _("Activate"));

}

//----------------------------------------------------------------------------
// AppAbout
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE (AppAbout, wxDialog)
    EVT_MENU  (wxID_HELP,  AppAbout::OnHelp)
    EVT_TIMER (myID_ABOUTTIMER, AppAbout::OnTimerEvent)
END_EVENT_TABLE ()

AppAbout::AppAbout (wxWindow *parent,
                    int milliseconds,
                    long style)
        : wxDialog (parent, -1, wxEmptyString,
                    wxDefaultPosition, wxDefaultSize,
                    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

    //accelerators (for help)
    const int nEntries = 1 ;
    wxAcceleratorEntry entries[nEntries];
    entries[0].Set (wxACCEL_NORMAL, WXK_F1, wxID_HELP);
    wxAcceleratorTable accel (nEntries, entries);
    SetAcceleratorTable (accel);

    // set timer if any
    m_timer = NULL;
    if (milliseconds > 0) {
        m_timer = new wxTimer (this, myID_ABOUTTIMER);
        m_timer->Start (milliseconds, wxTIMER_ONE_SHOT);
    }

    // sets the application title
    SetTitle (_("About .."));

    // about info
    wxGridSizer *aboutinfo = new wxGridSizer (2, 0, 2);
    aboutinfo->Add (new wxStaticText(this, -1, _("Written by: ")),
                    0, wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, -1, APP_MAINT),
                    1, wxEXPAND | wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, -1, _("Version: ")),
                    0, wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, -1, APP_VERSION),
                    1, wxEXPAND | wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, -1, _("Licence type: ")),
                    0, wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, -1, APP_LICENCE),
                    1, wxEXPAND | wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, -1, _("Copyright: ")),
                    0, wxALIGN_LEFT);
    aboutinfo->Add (new wxStaticText(this, -1, APP_COPYRIGTH),
                    1, wxEXPAND | wxALIGN_LEFT);

    // about icontitle//info
    wxBoxSizer *aboutpane = new wxBoxSizer (wxHORIZONTAL);
    wxBitmap bitmap = wxBitmap(wxICON (app));
    aboutpane->Add (new wxStaticBitmap (this, -1, bitmap),
                    0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 20);
    aboutpane->Add (aboutinfo, 1, wxEXPAND);
    aboutpane->Add (60, 0);

    // about complete
    wxBoxSizer *totalpane = new wxBoxSizer (wxVERTICAL);
    totalpane->Add (0, 20);
    wxStaticText *appname = new wxStaticText(this, -1, g_appname);
    appname->SetFont (wxFont (24, wxDEFAULT, wxNORMAL, wxBOLD));
    totalpane->Add (appname, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, 40);
    totalpane->Add (0, 10);
    totalpane->Add (aboutpane, 0, wxEXPAND | wxALL, 4);
    totalpane->Add (new wxStaticText(this, -1, APP_DESCR),
                    0, wxALIGN_CENTER | wxALL, 10);
    myHyperLink *website = new myHyperLink (this, -1, APP_WEBSITE);
    wxString url = APP_WEBSITE;
    url.Append ("/indexdemo.html");
    website->SetURL (url);
    totalpane->Add (website, 0, wxALIGN_CENTER | wxALL, 4);
    totalpane->Add (new wxStaticLine(this, -1), 0, wxEXPAND | wxALL, 10);
    wxButton *okButton = new wxButton (this, wxID_OK, _("OK"));
    okButton->SetDefault();
    totalpane->Add (okButton, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizerAndFit (totalpane);

    ShowModal();
}

AppAbout::~AppAbout () {
    if (m_timer)  {
        delete m_timer;
        m_timer = NULL;
    }
}

//----------------------------------------------------------------------------
// event handlers
void AppAbout::OnHelp (wxCommandEvent &WXUNUSED(event)) {
    g_help->Display(_T("introduction.html"));
}

void AppAbout::OnTimerEvent (wxTimerEvent &event) {
    if (m_timer) delete m_timer;
    m_timer = NULL;
    EndModal (wxID_OK);
}


