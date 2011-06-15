// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ProjectInfoPage.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "url.h"
#include "error_numbers.h"

#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "ValidateURL.h"
#include "BOINCBaseWizard.h"
#include "WizardAttach.h"
#include "ProjectInfoPage.h"


#include "res/windowsicon.xpm"
#include "res/macosicon.xpm"
#include "res/linuxicon.xpm"
#include "res/atiicon.xpm"
#include "res/nvidiaicon.xpm"
#include "res/multicore.xpm"


/*!
 * CProject type
 */
 
class CProjectInfo : public wxObject
{
    DECLARE_DYNAMIC_CLASS( CProjectInfo )
    CProjectInfo() {
        m_bSupportedPlatformFound = false;
        m_bProjectSupportsWindows = false;
        m_bProjectSupportsMac = false;
        m_bProjectSupportsLinux = false;
        m_bProjectSupportsNvidiaGPU = false;
        m_bProjectSupportsATIGPU = false;
        m_bProjectSupportsMulticore = false;
    }

public:
    wxString m_strURL;
    wxString m_strName;
    wxString m_strDescription;
    wxString m_strGeneralArea;
    wxString m_strSpecificArea;
    wxString m_strOrganization;
    bool m_bSupportedPlatformFound;
    bool m_bProjectSupportsWindows;
    bool m_bProjectSupportsMac;
    bool m_bProjectSupportsLinux;
    bool m_bProjectSupportsNvidiaGPU;
    bool m_bProjectSupportsATIGPU;
    bool m_bProjectSupportsMulticore;
};

IMPLEMENT_DYNAMIC_CLASS( CProjectInfo, wxObject )


/*!
 * CProjectInfoPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CProjectInfoPage, wxWizardPageEx )
 
/*!
 * CProjectInfoPage event table definition
 */
 
BEGIN_EVENT_TABLE( CProjectInfoPage, wxWizardPageEx )
 
////@begin CProjectInfoPage event table entries
    EVT_COMBOBOX( ID_CATEGORIES, CProjectInfoPage::OnProjectCategorySelected )
    EVT_LISTBOX( ID_PROJECTS, CProjectInfoPage::OnProjectSelected )
    EVT_WIZARDEX_PAGE_CHANGED( -1, CProjectInfoPage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CProjectInfoPage::OnPageChanging )
    EVT_WIZARDEX_CANCEL( -1, CProjectInfoPage::OnCancel )
////@end CProjectInfoPage event table entries
 
END_EVENT_TABLE()


/*!
 * CProjectInfoPage constructors
 */
 
CProjectInfoPage::CProjectInfoPage( )
{
}

CProjectInfoPage::CProjectInfoPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}


/*!
 * CProjectInfoPage destructor
 */

CProjectInfoPage::~CProjectInfoPage( )
{
    for (std::vector<CProjectInfo*>::iterator iter = m_Projects.begin(); iter != m_Projects.end(); ++iter)
    {
        CProjectInfo* pEntry = (CProjectInfo*)*iter;
        delete pEntry;
    }
    m_Projects.clear();
}


/*!
 * CProjectInfoPage creator
 */
 
bool CProjectInfoPage::Create( CBOINCBaseWizard* parent )
{
////@begin CProjectInfoPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDescriptionStaticCtrl = NULL;
    m_pProjectCategoriesStaticCtrl = NULL;
    m_pProjectCategoriesCtrl = NULL;
    m_pProjectsStaticCtrl = NULL;
    m_pProjectsCtrl = NULL;
    m_pProjectDetailsStaticCtrl = NULL;
    m_pProjectDetailsDescriptionStaticCtrl = NULL;
    m_pProjectDetailsDescriptionCtrl = NULL;
    m_pProjectDetailsResearchAreaStaticCtrl = NULL;
    m_pProjectDetailsResearchAreaCtrl = NULL;
    m_pProjectDetailsOrganizationStaticCtrl = NULL;
    m_pProjectDetailsOrganizationCtrl = NULL;
    m_pProjectDetailsURLStaticCtrl = NULL;
    m_pProjectDetailsURLCtrl = NULL;
    m_pProjectDetailsSupportedPlatformsStaticCtrl = NULL;
    m_pProjectDetailsSupportedPlatformWindowsCtrl = NULL;
    m_pProjectDetailsSupportedPlatformMacCtrl = NULL;
    m_pProjectDetailsSupportedPlatformLinuxCtrl = NULL;
    m_pProjectDetailsSupportedPlatformATICtrl = NULL;
    m_pProjectDetailsSupportedPlatformNvidiaCtrl = NULL;
    m_pProjectDetailsSupportedPlatformMultiCoreCtrl = NULL;
    m_pProjectURLStaticCtrl = NULL;
    m_pProjectURLCtrl = NULL;
////@end CProjectInfoPage member initialisation

    m_Projects.clear();
    m_bProjectSupported = false;
    m_bProjectListPopulated = false;
 
////@begin CProjectInfoPage creation
    wxWizardPageEx::Create( parent, ID_PROJECTINFOPAGE );

    CreateControls();

    GetSizer()->Fit(this);
////@end CProjectInfoPage creation
    return TRUE;
}


/*!
 * Control creation for WizardPage
 */
 
void CProjectInfoPage::CreateControls()
{    
////@begin CProjectInfoPage content construction
    CProjectInfoPage* itemWizardPage23 = this;

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage23->SetSizer(itemBoxSizer24);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(1, 1, 0, 0);
    itemFlexGridSizer4->AddGrowableRow(0);
    itemFlexGridSizer4->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer4, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer6->AddGrowableRow(1);
    itemFlexGridSizer6->AddGrowableCol(1);
    itemFlexGridSizer4->Add(itemFlexGridSizer6, 0, wxGROW|wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer6->Add(itemBoxSizer7, 0, wxALIGN_LEFT|wxALIGN_TOP, 0);

    m_pProjectCategoriesStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_pProjectCategoriesStaticCtrl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxArrayString m_pProjectCategoriesCtrlStrings;
    m_pProjectCategoriesCtrl = new wxComboBox( itemWizardPage23, ID_CATEGORIES, wxT(""), wxDefaultPosition, wxSize(150, -1), m_pProjectCategoriesCtrlStrings, wxCB_READONLY|wxCB_SORT );
    itemBoxSizer7->Add(m_pProjectCategoriesCtrl, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    m_pProjectsStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_pProjectsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
    itemFlexGridSizer11->AddGrowableRow(0);
    itemFlexGridSizer11->AddGrowableCol(0);
    itemBoxSizer7->Add(itemFlexGridSizer11, 0, wxGROW|wxALL, 5);

    wxArrayString m_pProjectsCtrlStrings;
    m_pProjectsCtrl = new wxListBox( itemWizardPage23, ID_PROJECTS, wxDefaultPosition, wxSize(150, 175), m_pProjectsCtrlStrings, wxLB_SINGLE|wxLB_SORT );
    itemFlexGridSizer11->Add(m_pProjectsCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    m_pProjectDetailsStaticCtrl = new wxStaticBox(itemWizardPage23, wxID_ANY, wxT(""));
    wxStaticBoxSizer* itemStaticBoxSizer13 = new wxStaticBoxSizer(m_pProjectDetailsStaticCtrl, wxVERTICAL);
    itemFlexGridSizer6->Add(itemStaticBoxSizer13, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pProjectDetailsDescriptionStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer13->Add(m_pProjectDetailsDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT, 5);

    m_pProjectDetailsDescriptionCtrl = new wxTextCtrl( itemWizardPage23, ID_PROJECTDESCRIPTION, wxEmptyString, wxDefaultPosition, wxSize(200, 100), wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2|wxTE_WORDWRAP );
    itemStaticBoxSizer13->Add(m_pProjectDetailsDescriptionCtrl, 0, wxGROW|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxFlexGridSizer* itemFlexGridSizer16 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer16->AddGrowableCol(1);
    itemStaticBoxSizer13->Add(itemFlexGridSizer16, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsResearchAreaStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(m_pProjectDetailsResearchAreaStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 2);

    m_pProjectDetailsResearchAreaCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(m_pProjectDetailsResearchAreaCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxFlexGridSizer* itemFlexGridSizer19 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer19->AddGrowableCol(1);
    itemStaticBoxSizer13->Add(itemFlexGridSizer19, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsOrganizationStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(m_pProjectDetailsOrganizationStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 2);

    m_pProjectDetailsOrganizationCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(m_pProjectDetailsOrganizationCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxFlexGridSizer* itemFlexGridSizer20 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer20->AddGrowableCol(1);
    itemStaticBoxSizer13->Add(itemFlexGridSizer20, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsURLStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer20->Add(m_pProjectDetailsURLStaticCtrl, 0, wxALIGN_LEFT|wxRIGHT|wxBOTTOM, 2);

    m_pProjectDetailsURLCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer20->Add(m_pProjectDetailsURLCtrl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxFlexGridSizer* itemFlexGridSizer24 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer24->AddGrowableRow(1);
    itemFlexGridSizer24->AddGrowableCol(0);
    itemStaticBoxSizer13->Add(itemFlexGridSizer24, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsSupportedPlatformsStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer24->Add(m_pProjectDetailsSupportedPlatformsStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer24->Add(itemBoxSizer26, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    m_pProjectDetailsSupportedPlatformWindowsCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("windowsicon.xpm")), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformWindowsCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformMacCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("macosicon.xpm")), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformMacCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformLinuxCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("linuxicon.xpm")), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformLinuxCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformATICtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("atiicon.xpm")), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformATICtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformNvidiaCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("nvidiaicon.xpm")), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformNvidiaCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformMultiCoreCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("multicore.xpm")), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformMultiCoreCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxFlexGridSizer* itemFlexGridSizer33 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer33->AddGrowableCol(1);
    itemFlexGridSizer4->Add(itemFlexGridSizer33, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pProjectURLStaticCtrl = new wxStaticText( itemWizardPage23, ID_PROJECTURLSTATICCTRL, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer33->Add(m_pProjectURLStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectURLCtrl = new wxTextCtrl( itemWizardPage23, ID_PROJECTURLCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer33->Add(m_pProjectURLCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_pProjectURLCtrl->SetValidator( CValidateURL( & m_strProjectURL ) );

    ////@end CProjectInfoPage content construction
}


/*!
 * Gets the previous page.
 */
wxWizardPageEx* CProjectInfoPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}


/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CProjectInfoPage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
    }
    return NULL;
}


/*!
 * Should we show tooltips?
 */
 
bool CProjectInfoPage::ShowToolTips()
{
    return TRUE;
}


/*!
 * Get bitmap resources
 */
 
wxBitmap CProjectInfoPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProjectInfoPage bitmap retrieval
    wxUnusedVar(name);
    if (name == wxT("windowsicon.xpm"))
    {
        wxBitmap bitmap(Win32_xpm);
        return bitmap;
    }
    else if (name == wxT("macosicon.xpm"))
    {
        wxBitmap bitmap(MacOS_xpm);
        return bitmap;
    }
    else if (name == wxT("linuxicon.xpm"))
    {
        wxBitmap bitmap(Linux_xpm);
        return bitmap;
    }
    else if (name == wxT("atiicon.xpm"))
    {
        wxBitmap bitmap(atiicon_xpm);
        return bitmap;
    }
    else if (name == wxT("nvidiaicon.xpm"))
    {
        wxBitmap bitmap(nvidiaicon_xpm);
        return bitmap;
    }
    else if (name == wxT("multicore.xpm"))
    {
        wxBitmap bitmap(multicore_xpm);
        return bitmap;
    }
    return wxNullBitmap;
////@end CProjectInfoPage bitmap retrieval
}


/*!
 * Get icon resources
 */
 
wxIcon CProjectInfoPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CProjectInfoPage icon retrieval
    return wxNullIcon;
////@end CProjectInfoPage icon retrieval
}


/*
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_CATEGORIES
 */

void CProjectInfoPage::OnProjectCategorySelected( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnProjectCategorySelected - Function Begin"));

    wxLogTrace(wxT("Function Status"), wxT("CProjectInfoPage::OnProjectCategorySelected - Clearing list box"));
    m_pProjectsCtrl->Clear();

    // Populate the list box with the list of project names that belong to eith the specific
    // category or all of them.
    wxLogTrace(wxT("Function Status"), wxT("CProjectInfoPage::OnProjectCategorySelected - Adding desired projects"));
    for (unsigned int i=0; i<m_Projects.size(); i++) {
        if ((m_pProjectCategoriesCtrl->GetValue() == _("All")) || 
            (m_pProjectCategoriesCtrl->GetValue() == m_Projects[i]->m_strGeneralArea)
        ) {
            wxLogTrace(
                wxT("Function Status"), 
                wxT("CProjectInfoPage::OnProjectCategorySelected - Adding '%s'"),
                m_Projects[i]->m_strName.c_str()
            );
            m_pProjectsCtrl->Append(m_Projects[i]->m_strName, m_Projects[i]);
        }
    }

    // Set the first item to be the selected item and then pop the next event.
    m_pProjectsCtrl->SetSelection(0);
    wxCommandEvent evtEvent(wxEVT_COMMAND_LISTBOX_SELECTED, ID_PROJECTS);
    ProcessEvent(evtEvent);

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnProjectCategorySelected - Function End"));
}


/*
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_PROJECTS
 */

void CProjectInfoPage::OnProjectSelected( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnProjectSelected - Function Begin"));

    CProjectInfo* pProjectInfo = (CProjectInfo*)m_pProjectsCtrl->GetClientData(m_pProjectsCtrl->GetSelection());

    m_pProjectDetailsSupportedPlatformWindowsCtrl->Hide();
    m_pProjectDetailsSupportedPlatformMacCtrl->Hide();
    m_pProjectDetailsSupportedPlatformLinuxCtrl->Hide();
    m_pProjectDetailsSupportedPlatformATICtrl->Hide();
    m_pProjectDetailsSupportedPlatformNvidiaCtrl->Hide();
    m_pProjectDetailsSupportedPlatformMultiCoreCtrl->Hide();

    // Populate the project details area
    m_pProjectDetailsDescriptionCtrl->SetLabel(pProjectInfo->m_strDescription);
    m_pProjectDetailsResearchAreaCtrl->SetLabel(pProjectInfo->m_strSpecificArea);
    m_pProjectDetailsOrganizationCtrl->SetLabel(pProjectInfo->m_strOrganization);
    m_pProjectDetailsURLCtrl->SetLabel(pProjectInfo->m_strURL);
    if (pProjectInfo->m_bProjectSupportsWindows) m_pProjectDetailsSupportedPlatformWindowsCtrl->Show();
    if (pProjectInfo->m_bProjectSupportsMac) m_pProjectDetailsSupportedPlatformMacCtrl->Show();
    if (pProjectInfo->m_bProjectSupportsLinux) m_pProjectDetailsSupportedPlatformLinuxCtrl->Show();
    if (pProjectInfo->m_bProjectSupportsATIGPU) m_pProjectDetailsSupportedPlatformATICtrl->Show();
    if (pProjectInfo->m_bProjectSupportsNvidiaGPU) m_pProjectDetailsSupportedPlatformNvidiaCtrl->Show();
    if (pProjectInfo->m_bProjectSupportsMulticore) m_pProjectDetailsSupportedPlatformMultiCoreCtrl->Show();

    // Populate non-control data for use in other places of the wizard
    SetProjectURL( pProjectInfo->m_strURL );
    SetProjectSupported( pProjectInfo->m_bSupportedPlatformFound );

    TransferDataToWindow();
    Layout();
    FitInside();

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnProjectSelected - Function End"));
}


/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnPageChanged - Function Begin"));

    CMainDocument*              pDoc = wxGetApp().GetDocument();
    unsigned int                i = 0, j = 0, k = 0;
    ALL_PROJECTS_LIST           pl;
    wxArrayString               aClientPlatforms;
    wxArrayString               aProjectPlatforms;
    wxArrayString               aCategories;
    bool                        bCategoryFound = false;
    CProjectInfo*               pProjectInfo = NULL;


    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pProjectCategoriesStaticCtrl);
    wxASSERT(m_pProjectCategoriesCtrl);
    wxASSERT(m_pProjectsStaticCtrl);
    wxASSERT(m_pProjectsCtrl);
    wxASSERT(m_pProjectDetailsStaticCtrl);
    wxASSERT(m_pProjectDetailsDescriptionStaticCtrl);
    wxASSERT(m_pProjectDetailsDescriptionCtrl);
    wxASSERT(m_pProjectDetailsResearchAreaStaticCtrl);
    wxASSERT(m_pProjectDetailsResearchAreaCtrl);
    wxASSERT(m_pProjectDetailsOrganizationStaticCtrl);
    wxASSERT(m_pProjectDetailsOrganizationStaticCtrl);
    wxASSERT(m_pProjectDetailsURLStaticCtrl);
    wxASSERT(m_pProjectDetailsURLCtrl);
    wxASSERT(m_pProjectDetailsSupportedPlatformsStaticCtrl);
    wxASSERT(m_pProjectDetailsSupportedPlatformWindowsCtrl);
    wxASSERT(m_pProjectDetailsSupportedPlatformMacCtrl);
    wxASSERT(m_pProjectDetailsSupportedPlatformLinuxCtrl);
    wxASSERT(m_pProjectDetailsSupportedPlatformATICtrl);
    wxASSERT(m_pProjectDetailsSupportedPlatformNvidiaCtrl);
    wxASSERT(m_pProjectDetailsSupportedPlatformMultiCoreCtrl);
    wxASSERT(m_pProjectURLStaticCtrl);
    wxASSERT(m_pProjectURLCtrl);


    m_pTitleStaticCtrl->SetLabel(
        _("Choose a project")
    );

    m_pDescriptionStaticCtrl->SetLabel(
        _("To choose a project, click its name or type its URL below.")
    );

    m_pProjectCategoriesStaticCtrl->SetLabel(
        _("Projects Categories:")
    );

    m_pProjectsStaticCtrl->SetLabel(
        _("Projects:")
    );

    m_pProjectDetailsStaticCtrl->SetLabel(
        _("Details")
    );

    m_pProjectDetailsDescriptionStaticCtrl->SetLabel(
        _("Description:")
    );

    m_pProjectDetailsResearchAreaStaticCtrl->SetLabel(
        _("Research Area:")
    );

    m_pProjectDetailsOrganizationStaticCtrl->SetLabel(
        _("Organization:")
    );

    m_pProjectDetailsURLStaticCtrl->SetLabel(
        _("Project URL:")
    );

    m_pProjectDetailsSupportedPlatformsStaticCtrl->SetLabel(
        _("Supported Platform(s):")
    );

    m_pProjectURLStaticCtrl->SetLabel(
        _("Project URL:")
    );


    // Populate the ProjectInfo data structure with the list of projects we want to show and
    // any other activity we need to prep the page.
    if (!m_bProjectListPopulated) {

        // Get the project list
        pDoc->rpc.get_all_projects_list(pl);

        // Convert the supported client platforms into something useful
        for (i=0; i<pDoc->state.platforms.size(); i++) {
            aClientPlatforms.Add(wxString(pDoc->state.platforms[i].c_str(), wxConvUTF8));
        }

        // Iterate through the project list and add them to the ProjectInfo data structure
        for (i=0; i<pl.projects.size(); i++) {
            pProjectInfo = new CProjectInfo();
            m_Projects.push_back(pProjectInfo);

            wxLogTrace(
                wxT("Function Status"),
                wxT("CProjectInfoPage::OnPageChanged - Name: '%s', URL: '%s'"),
                wxString(pl.projects[i]->name.c_str(), wxConvUTF8).c_str(),
                wxString(pl.projects[i]->url.c_str(), wxConvUTF8).c_str()
            );

            // Convert the easy stuff
            pProjectInfo->m_strURL = wxString(pl.projects[i]->url.c_str(), wxConvUTF8);
            pProjectInfo->m_strName = wxString(pl.projects[i]->name.c_str(), wxConvUTF8);
            pProjectInfo->m_strDescription = wxString(pl.projects[i]->description.c_str(), wxConvUTF8);
            pProjectInfo->m_strGeneralArea = wxString(pl.projects[i]->general_area.c_str(), wxConvUTF8);
            pProjectInfo->m_strSpecificArea = wxString(pl.projects[i]->specific_area.c_str(), wxConvUTF8);
            pProjectInfo->m_strOrganization = wxString(pl.projects[i]->home.c_str(), wxConvUTF8);
            
            // Add the category if it isn't already in the category list
            bCategoryFound = false;
            for (j=0; j<aCategories.size(); j++) {
                if (aCategories[j] == pProjectInfo->m_strGeneralArea) {
                    bCategoryFound = true;
                }
            }
            if (!bCategoryFound) {
                aCategories.Add(pProjectInfo->m_strGeneralArea);
            }

            // Convert the supported project platforms into something useful
            aProjectPlatforms.Clear();
            for (j=0; j<pl.projects[i]->platforms.size(); j++) {
                aProjectPlatforms.Add(wxString(pl.projects[i]->platforms[j].c_str(), wxConvUTF8));
            }

            // Can the core client support a platform that this project supports?
            for (j = 0;j < aClientPlatforms.size(); j++) {
                for (k = 0;k < aProjectPlatforms.size(); k++) {
                    wxString strClientPlatform = aClientPlatforms[j];
                    wxString strProjectPlatform = aProjectPlatforms[k];
                    wxString strRootProjectPlatform = strProjectPlatform.SubString(0, strProjectPlatform.Find(_T("[")) - 1);
                    
                    if (strProjectPlatform.Find(_T("windows")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsWindows = true;
                        if (strClientPlatform == strRootProjectPlatform) {
                            pProjectInfo->m_bSupportedPlatformFound = true;
                        }
                    }

                    if (strProjectPlatform.Find(_T("apple")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsMac = true;
                        if (strClientPlatform == strRootProjectPlatform) {
                            pProjectInfo->m_bSupportedPlatformFound = true;
                        }
                    }

                    if (strProjectPlatform.Find(_T("linux")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsLinux = true;
                        if (strClientPlatform == strRootProjectPlatform) {
                            pProjectInfo->m_bSupportedPlatformFound = true;
                        }
                    }

                    if (strProjectPlatform.Find(_T("[cuda")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsNvidiaGPU = true;
                        if ((pDoc->state.have_cuda) && (strClientPlatform == strRootProjectPlatform)) {
                            pProjectInfo->m_bSupportedPlatformFound = true;
                        }
                    }

                    if (strProjectPlatform.Find(_T("[ati")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsATIGPU = true;
                        if ((pDoc->state.have_ati) && (strClientPlatform == strRootProjectPlatform)) {
                            pProjectInfo->m_bSupportedPlatformFound = true;
                        }
                    }

                    if (strProjectPlatform.Find(_T("[mt")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsMulticore = true;
                        if ((pDoc->host.p_ncpus >= 4) && (strClientPlatform == strRootProjectPlatform)) {
                            pProjectInfo->m_bSupportedPlatformFound = true;
                        }
                    }

                    if (strClientPlatform == strRootProjectPlatform) {
                        pProjectInfo->m_bSupportedPlatformFound = true;
                    }
                }
            }

            wxLogTrace(
                wxT("Function Status"),
                wxT("CProjectInfoPage::OnPageChanged - Windows: '%d', Mac: '%d', Linux: '%d', Nvidia: '%d', ATI: '%d', Multicore: '%d', Platform: '%d'"),
                pProjectInfo->m_bProjectSupportsWindows,
                pProjectInfo->m_bProjectSupportsMac,
                pProjectInfo->m_bProjectSupportsLinux,
                pProjectInfo->m_bProjectSupportsNvidiaGPU,
                pProjectInfo->m_bProjectSupportsATIGPU,
                pProjectInfo->m_bProjectSupportsMulticore,
                pProjectInfo->m_bSupportedPlatformFound
            );
        }


        // Populate the category combo box
        m_pProjectCategoriesCtrl->Clear();
        m_pProjectCategoriesCtrl->Append(_("All"));
        for (i=0; i<aCategories.size(); i++) {
            m_pProjectCategoriesCtrl->Append(aCategories[i]);
        }
        m_pProjectCategoriesCtrl->SetValue(_("All"));

        // Trigger initial event to populate the list control
        wxCommandEvent evtEvent(wxEVT_COMMAND_COMBOBOX_SELECTED, ID_CATEGORIES);
        ProcessEvent(evtEvent);

        m_bProjectListPopulated = true;
    }

    m_pProjectsCtrl->SetFocus();

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnPageChanged - Function End"));
}


/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanging( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnPageChanging - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument(); 
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString       strTitle;
    int            iAnswer;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    strTitle.Printf(
        wxT("%s"), 
        pSkinAdvanced->GetApplicationName().c_str()
    );


    // Check to see if the project is supported:
    if ( !GetProjectSupported() ) {

        iAnswer = wxGetApp().SafeMessageBox(
            _("This project may not have work for your type of computer.  Do you want to add it anyway?"),
            strTitle,
            wxCENTER | wxYES_NO | wxICON_INFORMATION
        );

        // Project is not supported
        if (wxNO == iAnswer) {
            event.Veto();
        }

    } else {

        // Check if we are already attached to that project: 
 	    for (int i = 0; i < pDoc->GetProjectCount(); ++i) { 
 	        PROJECT* project = pDoc->project(i);
            if (project) {
                std::string project_url = project->master_url;
                std::string new_project_url = (const char*)m_strProjectURL.mb_str();

                canonicalize_master_url(project_url);
                canonicalize_master_url(new_project_url);
                
                if (project_url == new_project_url) {
                    wxGetApp().SafeMessageBox(
                        _("You already added this project. Please choose a different project."),
                        strTitle,
                        wxCENTER | wxICON_INFORMATION
                    );

                    // We are already attached to that project, 
                    event.Veto();
                    break;
                }
            } 
        } 
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnPageChanging - Function End"));
}


/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

