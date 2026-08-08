// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgExclusiveApps.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "SkinManager.h"
#include "Events.h"
#include "error_numbers.h"
#include "version.h"
#include "DlgExclusiveApps.h"


using std::string;

IMPLEMENT_DYNAMIC_CLASS(CDlgExclusiveApps, wxDialog)

BEGIN_EVENT_TABLE(CDlgExclusiveApps, wxDialog)
    EVT_COMMAND(ID_LISTBOX_EXCLAPPS,wxEVT_COMMAND_LISTBOX_SELECTED,CDlgExclusiveApps::OnExclusiveAppListEvent)
    EVT_COMMAND(ID_LISTBOX_EXCLGPUAPPS,wxEVT_COMMAND_LISTBOX_SELECTED,CDlgExclusiveApps::OnExclusiveGPUAppListEvent)
    //buttons
    EVT_BUTTON(ID_ADDEXCLUSIVEAPPBUTTON,CDlgExclusiveApps::OnAddExclusiveApp)
    EVT_BUTTON(ID_REMOVEEXCLUSIVEAPPBUTTON,CDlgExclusiveApps::OnRemoveExclusiveApp)
    EVT_BUTTON(ID_ADDEXCLUSIVEGPUAPPBUTTON,CDlgExclusiveApps::OnAddExclusiveGPUApp)
    EVT_BUTTON(ID_REMOVEEXCLUSIVEGPUAPPBUTTON,CDlgExclusiveApps::OnRemoveExclusiveGPUApp)
    EVT_BUTTON(wxID_OK,CDlgExclusiveApps::OnOK)
    EVT_BUTTON(ID_HELPBOINC,CDlgExclusiveApps::OnHelp)
END_EVENT_TABLE()

/* Constructor */
CDlgExclusiveApps::CDlgExclusiveApps(wxWindow* parent) :
    wxDialog( parent, ID_ANYDIALOG, wxEmptyString, wxDefaultPosition,
    wxDefaultSize, wxDEFAULT_DIALOG_STYLE
    ) {
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    wxString title;
    title.Printf(
        _("%s - Exclusive Applications"),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );

    SetTitle(title);

    m_bInInit=false;
    m_bExclusiveAppsDataChanged=false;

    wxBoxSizer* dialogSizer = new wxBoxSizer( wxVERTICAL );
    dialogSizer->AddSpacer(10);

    wxStaticBox* exclusiveAppsListStaticBox = new wxStaticBox( this, -1, _("Suspend processor and network usage when these applications are running:") );
    wxStaticBoxSizer* exclusiveAppsListBoxSizer = new wxStaticBoxSizer( exclusiveAppsListStaticBox, wxVERTICAL );

    m_exclusiveApsListBox = new wxListBox(exclusiveAppsListStaticBox, ID_LISTBOX_EXCLAPPS, wxDefaultPosition, wxSize(-1, 145), 0, NULL, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_SORT);
    exclusiveAppsListBoxSizer->Add(m_exclusiveApsListBox, 1, wxALL|wxEXPAND, 5);

    wxBoxSizer* exclusiveAppsButtonSizer = new wxBoxSizer( wxHORIZONTAL );

    m_addExclusiveAppButton = new wxButton( exclusiveAppsListStaticBox, ID_ADDEXCLUSIVEAPPBUTTON, _("Add..."), wxDefaultPosition, wxDefaultSize, 0 );
    m_addExclusiveAppButton->SetToolTip( _("Add an application to this list"));
    exclusiveAppsButtonSizer->Add( m_addExclusiveAppButton, 0, wxRIGHT, 5 );

    exclusiveAppsButtonSizer->AddStretchSpacer();

    m_removeExclusiveAppButton = new wxButton( exclusiveAppsListStaticBox, ID_REMOVEEXCLUSIVEAPPBUTTON, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
    m_removeExclusiveAppButton->SetToolTip( _("Remove an application from this list"));
    exclusiveAppsButtonSizer->Add( m_removeExclusiveAppButton, 0, wxLEFT, 5 );

    exclusiveAppsListBoxSizer->Add(exclusiveAppsButtonSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 25 );

    dialogSizer->Add( exclusiveAppsListBoxSizer, 0, wxALL|wxEXPAND, 1 );

    dialogSizer->AddSpacer(25);

    wxStaticBox* exclusiveGPUAppsListStaticBox = new wxStaticBox( this, -1, _("Suspend GPU usage when these applications are running:") );
    wxStaticBoxSizer* exclusiveGPUAppsListBoxSizer = new wxStaticBoxSizer( exclusiveGPUAppsListStaticBox, wxVERTICAL );

    m_exclusiveGPUApsListBox = new wxListBox(exclusiveGPUAppsListStaticBox, ID_LISTBOX_EXCLGPUAPPS, wxDefaultPosition, wxSize(-1, 145), 0, NULL, wxLB_EXTENDED|wxLB_NEEDED_SB|wxLB_SORT);
    exclusiveGPUAppsListBoxSizer->Add(m_exclusiveGPUApsListBox, 1, wxALL|wxEXPAND, 5);

    wxBoxSizer* exclusiveGPUAppsButtonSizer = new wxBoxSizer( wxHORIZONTAL );

    m_addExclusiveGPUAppButton = new wxButton( exclusiveGPUAppsListStaticBox, ID_ADDEXCLUSIVEGPUAPPBUTTON, _("Add..."), wxDefaultPosition, wxDefaultSize, 0 );
    m_addExclusiveGPUAppButton->SetToolTip( _("Add an application to this list"));
    exclusiveGPUAppsButtonSizer->Add( m_addExclusiveGPUAppButton, 0, wxRIGHT, 5 );

    exclusiveGPUAppsButtonSizer->AddStretchSpacer();

    m_removeExclusiveGPUAppButton = new wxButton( exclusiveGPUAppsListStaticBox, ID_REMOVEEXCLUSIVEGPUAPPBUTTON, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
    m_removeExclusiveGPUAppButton->SetToolTip( _("Remove an application from this list"));
    exclusiveGPUAppsButtonSizer->Add( m_removeExclusiveGPUAppButton, 0, wxLEFT, 5 );

    exclusiveGPUAppsListBoxSizer->Add(exclusiveGPUAppsButtonSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 25 );

    dialogSizer->Add( exclusiveGPUAppsListBoxSizer, 0, wxALL|wxEXPAND, 1 );

    wxBoxSizer* moreOptionsLinkSizer = new wxBoxSizer( wxHORIZONTAL );

    moreOptionsLinkSizer->Add(
        new wxStaticText(
            this, wxID_ANY, _("For advanced options, refer to "),
            wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT
        ),
        0, wxLEFT, 5
    );

    moreOptionsLinkSizer->Add(
        new wxHyperlinkCtrl(
            this, wxID_ANY, wxT("https://github.com/BOINC/boinc/wiki/Client-configuration"),
            wxT("https://github.com/BOINC/boinc/wiki/Client-configuration"),
            wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE
        ),
#ifdef __WXMAC__
        0, wxLEFT, 5
#else
        0, wxLEFT, 3
#endif
    );

    dialogSizer->Add(moreOptionsLinkSizer, 0, wxALL, 10);

    m_panelButtons = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    m_btnOK = new wxButton( m_panelButtons, wxID_OK, _("&Save"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnOK->SetToolTip( _("Save all values and close the dialog") );

    buttonSizer->Add( m_btnOK, 0, wxALL, 5 );

    m_btnCancel = new wxButton( m_panelButtons, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnCancel->SetToolTip( _("Close the dialog without saving") );
    m_btnCancel->SetDefault();

    buttonSizer->Add( m_btnCancel, 0, wxALL, 5 );

    m_btnHelp = new wxButton( m_panelButtons, ID_HELPBOINC, _("&Help"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnHelp->SetToolTip( _("shows the preferences web page") );

    buttonSizer->Add( m_btnHelp, 0, wxALL, 5 );

    m_panelButtons->SetSizer( buttonSizer );
    m_panelButtons->Layout();
    buttonSizer->Fit( m_panelButtons );
    dialogSizer->Add( m_panelButtons, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1 );

    m_removeExclusiveAppButton->Disable();
    m_removeExclusiveGPUAppButton->Disable();

    ReadPreferenceSettings();

    Layout();
    SetSizerAndFit(dialogSizer);

    Centre();   // Center the dialog over the main window

#if defined(__WXMSW__) || defined(__WXGTK__)
    SetDoubleBuffered(true);
#endif
}

/* destructor */
CDlgExclusiveApps::~CDlgExclusiveApps() {
}

/* read preferences from core client and initialize control values */
void CDlgExclusiveApps::ReadPreferenceSettings() {
    m_bInInit=true;//prevent dialog handlers from doing anything
    CMainDocument* pDoc = wxGetApp().GetDocument();
    int retval;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Get cc_config.xml file flags
    log_flags.init();
    config.defaults();
    retval = pDoc->rpc.get_cc_config(config, log_flags);
    if (!retval) {
        for (unsigned int i=0; i<config.exclusive_apps.size(); ++i) {
            wxString appName = wxString(config.exclusive_apps[i].c_str(), wxConvUTF8);
            m_exclusiveApsListBox->Append(appName);
        }

        for (unsigned int i=0; i<config.exclusive_gpu_apps.size(); ++i) {
            wxString appName = wxString(config.exclusive_gpu_apps[i].c_str(), wxConvUTF8);
            m_exclusiveGPUApsListBox->Append(appName);
        }
    }

    m_bInInit=false;
}


bool CDlgExclusiveApps::SavePreferencesSettings() {
    if (m_bExclusiveAppsDataChanged) {
        CMainDocument*    pDoc = wxGetApp().GetDocument();

        wxASSERT(pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));

        wxArrayString appNames = m_exclusiveApsListBox->GetStrings();

        config.exclusive_apps.clear();
        for (unsigned int i=0; i<appNames.size(); ++i) {
            std::string s = (const char*)appNames[i].mb_str();
            config.exclusive_apps.push_back(s);
        }

       wxArrayString gpuAppNames = m_exclusiveGPUApsListBox->GetStrings();
        config.exclusive_gpu_apps.clear();
        for (unsigned int i=0; i<gpuAppNames.size(); ++i) {
            std::string s = (const char*)gpuAppNames[i].mb_str();
            config.exclusive_gpu_apps.push_back(s);
        }
        int retval = pDoc->rpc.set_cc_config(config, log_flags);
        if (!retval) {
            pDoc->rpc.read_cc_config();
        }
        return true;
    }
    return false;
}

// ------------ Event handlers starts here
// ---- Exclusive Apps list box handler
void CDlgExclusiveApps::OnExclusiveAppListEvent(wxCommandEvent& ev) {
    wxArrayInt selections;
    int numSelected;

    if(!m_bInInit) {
        numSelected = m_exclusiveApsListBox->GetSelections(selections);
        m_removeExclusiveAppButton->Enable(numSelected > 0);
    }
    ev.Skip();
}

void CDlgExclusiveApps::OnExclusiveGPUAppListEvent(wxCommandEvent& ev) {
    wxArrayInt selections;
    int numSelected;

    if(!m_bInInit) {
        numSelected = m_exclusiveGPUApsListBox->GetSelections(selections);
        m_removeExclusiveGPUAppButton->Enable(numSelected > 0);
    }
    ev.Skip();
}

// ---- command buttons handlers
// handles Add button clicked
void CDlgExclusiveApps::OnAddExclusiveApp(wxCommandEvent&) {
    AddToListBox(m_exclusiveApsListBox);
}

void CDlgExclusiveApps::OnAddExclusiveGPUApp(wxCommandEvent&) {
    AddToListBox(m_exclusiveGPUApsListBox);
}

void CDlgExclusiveApps::AddToListBox(wxListBox * theListBox) {
    wxString strMachineName;
    int i, j, n;
    bool hostIsMac = false;
    bool hostIsWin = false;
    bool isDuplicate;
    wxArrayString appNames;
    wxString errmsg;
    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (strstr(pDoc->state.host_info.os_name, "Darwin")) {
        hostIsMac = true;
    } else if (strstr(pDoc->state.host_info.os_name, "Microsoft")) {
        hostIsWin = true;
    }

    pDoc->GetConnectedComputerName(strMachineName);
    if (pDoc->IsComputerNameLocal(strMachineName)) {
        // if the client is local, use a file chooser dialog.
        // This eliminates spelling errors.
        // Note: the excluded app may be in an inaccessible directory,
        // or the user might not know where it is.
        // In this case they can just type the filename into the dialog
        //
#ifdef __WXMAC__
        wxFileDialog picker(this, _("Choose application or enter file name"),
            wxT("/Applications"), wxT(""), wxT(""),
            0
        );
#elif defined(__WXMSW__)
//TODO: fill in the default directory for MSW
        wxFileDialog picker(this, _("Choose application or enter file name"),
            wxT("C:/Program Files"), wxT(""), wxT(""),
            0
        );
#else
//TODO: fill in the default directory for Linux
        wxFileDialog picker(this, _("Choose application or enter file name"),
            wxT("/usr/bin"), wxT(""), wxT(""),
            0
        );
#endif
        if (picker.ShowModal() != wxID_OK) return;
        picker.GetFilenames(appNames);

#ifdef __WXMSW__
        for (i=appNames.Count()-1; i>=0; --i) {
            // Under Windows, filename may include paths if a shortcut selected
            appNames[i] = appNames[i].AfterLast('\\');
        }
#endif
    } else {
        // We can't use file picker if connected to a remote computer,
        // so show a dialog with textedit field so user can type app name
        //
        wxChar path_separator = wxT('/');

        wxTextEntryDialog dlg(this, _("Name of application to add?"), _("Add exclusive app"));
        if (hostIsWin) {
            path_separator = wxT('\\');
        }
        if (dlg.ShowModal() != wxID_OK) return;

        wxString theAppName = dlg.GetValue();
        // Strip off path if present
        appNames.Add(theAppName.AfterLast(path_separator));
    }

    for (i=0; i<(int)appNames.Count(); ++i) {
        if (hostIsMac) {
            int suffix = appNames[i].Find('.', true);
            if (suffix != wxNOT_FOUND) {
                appNames[i].Truncate(suffix);
            }
        }

        // Skip requests for duplicate entries
        //
        isDuplicate = false;
        n = theListBox->GetCount();
        for (j=0; j<n; ++j) {
            if ((theListBox->GetString(j)).Cmp(appNames[i]) == 0) {
                isDuplicate = true;
                break;
            }
        }
        if (isDuplicate) {
            errmsg.Printf(_("'%s' is already in the list."), appNames[i].c_str());
            wxGetApp().SafeMessageBox(errmsg, _("Add Exclusive App"),
                wxOK | wxICON_EXCLAMATION, this
            );
            continue;
        }

        theListBox->Append(appNames[i]);
        m_bExclusiveAppsDataChanged = true;
    }
}

typedef int (*sortcomparefunc)(int*, int*);

static int myCompareInts(int *first, int *second) {
    return *first - *second;
}
// handles Remove button clicked
void CDlgExclusiveApps::OnRemoveExclusiveApp(wxCommandEvent& ev) {
    wxArrayInt selections;
    int numSelected = m_exclusiveApsListBox->GetSelections(selections);

    // The selection indices are returned in random order.
    // We must sort them to ensure deleting the correct items.
    //
    selections.Sort((sortcomparefunc)&myCompareInts);
    for (int i=numSelected-1; i>=0; --i) {
        m_exclusiveApsListBox->Delete(selections[i]);
        m_bExclusiveAppsDataChanged = true;
    }
    // Check if no more exclusive apps exist.  If no more, disable remove button.
    if (!m_exclusiveApsListBox->HasClientObjectData()) {
        m_removeExclusiveAppButton->Disable();
    }
    ev.Skip();
}

void CDlgExclusiveApps::OnRemoveExclusiveGPUApp(wxCommandEvent& ev) {
    wxArrayInt selections;
    int numSelected = m_exclusiveGPUApsListBox->GetSelections(selections);

    // The selection indices are returned in random order.
    // We must sort them to ensure deleting the correct items.
    //
    selections.Sort((sortcomparefunc)&myCompareInts);
    for (int i=numSelected-1; i>=0; --i) {
        m_exclusiveGPUApsListBox->Delete(selections[i]);
        m_bExclusiveAppsDataChanged = true;
    }
    // Check if no more exclusive GPU apps exist.  If no more, disable remove button.
    if (!m_exclusiveGPUApsListBox->HasClientObjectData()) {
        m_removeExclusiveGPUAppButton->Disable();
    }
    ev.Skip();
}

// handles OK button clicked
void CDlgExclusiveApps::OnOK(wxCommandEvent& ev) {
    SavePreferencesSettings();

    ev.Skip();
}

// handles Help button clicked
void CDlgExclusiveApps::OnHelp(wxCommandEvent& ev) {
    if (IsShown()) {

        wxString strURL = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationHelpUrl();

        wxString wxurl;
        wxurl.Printf(
            wxT("%s?target=exclusive_apps&version=%s&controlid=%d"),
            strURL.c_str(),
            wxString(BOINC_VERSION_STRING, wxConvUTF8).c_str(),
            ev.GetId()
        );
        wxLaunchDefaultBrowser(wxurl);
    }
}
