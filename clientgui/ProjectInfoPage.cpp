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
#include "res/blankicon.xpm"


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
        m_bProjectSupportsCUDA = false;
        m_bProjectSupportsCAL = false;
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
    bool m_bProjectSupportsCUDA;
    bool m_bProjectSupportsCAL;
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
    m_pProjectDetailsSupportedPlatformBlankCtrl = NULL;
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
#ifdef __WXMAC__
#define LISTBOXWIDTH 225
#define DESCRIPTIONSWIDTH 350
#else
#define LISTBOXWIDTH 150
#define DESCRIPTIONSWIDTH 200
#endif

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
    itemBoxSizer7->Add(m_pProjectCategoriesStaticCtrl, 0, wxALIGN_LEFT|wxRIGHT|wxBOTTOM, 5);

    wxArrayString m_pProjectCategoriesCtrlStrings;
    m_pProjectCategoriesCtrl = new wxComboBox( itemWizardPage23, ID_CATEGORIES, wxT(""), wxDefaultPosition, wxSize(LISTBOXWIDTH, -1), m_pProjectCategoriesCtrlStrings, wxCB_READONLY|wxCB_SORT );
    itemBoxSizer7->Add(m_pProjectCategoriesCtrl, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    m_pProjectsStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_pProjectsStaticCtrl, 0, wxALIGN_LEFT|wxTOP|wxRIGHT|wxBOTTOM, 5);

    wxFlexGridSizer* itemFlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
    itemFlexGridSizer11->AddGrowableRow(0);
    itemFlexGridSizer11->AddGrowableCol(0);
    itemBoxSizer7->Add(itemFlexGridSizer11, 0, wxGROW|wxALL, 0);

    wxArrayString m_pProjectsCtrlStrings;
    m_pProjectsCtrl = new wxListBox( itemWizardPage23, ID_PROJECTS, wxDefaultPosition, wxSize(LISTBOXWIDTH, 175), m_pProjectsCtrlStrings, wxLB_SINGLE|wxLB_SORT );
    itemFlexGridSizer11->Add(m_pProjectsCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    m_pProjectDetailsStaticCtrl = new wxStaticBox(itemWizardPage23, wxID_ANY, wxT(""));
    wxStaticBoxSizer* itemStaticBoxSizer13 = new wxStaticBoxSizer(m_pProjectDetailsStaticCtrl, wxVERTICAL);
    itemFlexGridSizer6->Add(itemStaticBoxSizer13, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectDetailsDescriptionCtrl = new wxTextCtrl( itemWizardPage23, ID_PROJECTDESCRIPTION, wxT(""), wxDefaultPosition, wxSize(DESCRIPTIONSWIDTH, 100), wxTE_MULTILINE|wxTE_READONLY );
    itemStaticBoxSizer13->Add(m_pProjectDetailsDescriptionCtrl, 0, wxGROW|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxFlexGridSizer* itemFlexGridSizer16 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer16->AddGrowableCol(1);
    itemStaticBoxSizer13->Add(itemFlexGridSizer16, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsResearchAreaStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(m_pProjectDetailsResearchAreaStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 2);

    m_pProjectDetailsResearchAreaCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(m_pProjectDetailsResearchAreaCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxFlexGridSizer* itemFlexGridSizer19 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer19->AddGrowableCol(1);
    itemStaticBoxSizer13->Add(itemFlexGridSizer19, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsOrganizationStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(m_pProjectDetailsOrganizationStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 2);

    m_pProjectDetailsOrganizationCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(m_pProjectDetailsOrganizationCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxFlexGridSizer* itemFlexGridSizer20 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer20->AddGrowableCol(1);
    itemStaticBoxSizer13->Add(itemFlexGridSizer20, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsURLStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer20->Add(m_pProjectDetailsURLStaticCtrl, 0, wxALIGN_LEFT|wxRIGHT|wxBOTTOM, 2);

    m_pProjectDetailsURLCtrl = new wxHyperlinkCtrl( itemWizardPage23, wxID_STATIC, wxT("BOINC"), wxT("http://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT);
    itemFlexGridSizer20->Add(m_pProjectDetailsURLCtrl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxFlexGridSizer* itemFlexGridSizer24 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer24->AddGrowableRow(1);
    itemFlexGridSizer24->AddGrowableCol(0);
    itemStaticBoxSizer13->Add(itemFlexGridSizer24, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsSupportedPlatformsStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer24->Add(m_pProjectDetailsSupportedPlatformsStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer24->Add(itemBoxSizer26, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    m_pProjectDetailsSupportedPlatformWindowsCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("windowsicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformWindowsCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformMacCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("macosicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformMacCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformLinuxCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("linuxicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformLinuxCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformATICtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("atiicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformATICtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformNvidiaCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("nvidiaicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformNvidiaCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformBlankCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("blankicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformBlankCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxFlexGridSizer* itemFlexGridSizer33 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer33->AddGrowableCol(1);
    itemFlexGridSizer4->Add(itemFlexGridSizer33, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pProjectURLStaticCtrl = new wxStaticText( itemWizardPage23, ID_PROJECTURLSTATICCTRL, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer33->Add(m_pProjectURLStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectURLCtrl = new wxTextCtrl( itemWizardPage23, ID_PROJECTURLCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer33->Add(m_pProjectURLCtrl, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifdef __WXMAC__
    itemFlexGridSizer33->Add(0, 20, 0);
#endif

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
    else if (name == wxT("blankicon.xpm"))
    {
        wxBitmap bitmap(blankicon_xpm);
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

    wxString strURL = pProjectInfo->m_strURL;
    EllipseStringIfNeeded(strURL, m_pProjectDetailsURLCtrl);

    // Populate the project details area
    wxString desc = pProjectInfo->m_strDescription;
    // Change all occurrences of "<sup>n</sup>" to "^n"
    desc.Replace(wxT("<sup>"), wxT("^"), true);
    desc.Replace(wxT("</sup>"), wxT(""), true);

    m_pProjectDetailsDescriptionCtrl->SetValue(desc);
    m_pProjectDetailsURLCtrl->SetLabel(strURL);
    m_pProjectDetailsURLCtrl->SetURL(pProjectInfo->m_strURL);
    // Set tooltip to full text in case ellipsed
    m_pProjectDetailsURLCtrl->SetToolTip(pProjectInfo->m_strURL);

    m_pProjectDetailsSupportedPlatformWindowsCtrl->Hide();
    m_pProjectDetailsSupportedPlatformMacCtrl->Hide();
    m_pProjectDetailsSupportedPlatformLinuxCtrl->Hide();
    m_pProjectDetailsSupportedPlatformATICtrl->Hide();
    m_pProjectDetailsSupportedPlatformNvidiaCtrl->Hide();
    if (pProjectInfo->m_bProjectSupportsWindows) m_pProjectDetailsSupportedPlatformWindowsCtrl->Show();
    if (pProjectInfo->m_bProjectSupportsMac) m_pProjectDetailsSupportedPlatformMacCtrl->Show();
    if (pProjectInfo->m_bProjectSupportsLinux) m_pProjectDetailsSupportedPlatformLinuxCtrl->Show();
    if (pProjectInfo->m_bProjectSupportsCAL) m_pProjectDetailsSupportedPlatformATICtrl->Show();
    if (pProjectInfo->m_bProjectSupportsCUDA) m_pProjectDetailsSupportedPlatformNvidiaCtrl->Show();

    // Populate non-control data for use in other places of the wizard
    SetProjectURL( pProjectInfo->m_strURL );
    SetProjectSupported( pProjectInfo->m_bSupportedPlatformFound );

    TransferDataToWindow();
    Layout();

    wxString strResearchArea = pProjectInfo->m_strSpecificArea;
    EllipseStringIfNeeded(strResearchArea, m_pProjectDetailsResearchAreaCtrl);
    m_pProjectDetailsResearchAreaCtrl->SetLabel(strResearchArea);
    // Set tooltip to full text in case ellipsed
    m_pProjectDetailsResearchAreaCtrl->SetToolTip(pProjectInfo->m_strSpecificArea);

    wxString strOrganization = pProjectInfo->m_strOrganization;
    EllipseStringIfNeeded(strOrganization, m_pProjectDetailsOrganizationCtrl);
    m_pProjectDetailsOrganizationCtrl->SetLabel(strOrganization);
    // Set tooltip to full text in case ellipsed
    m_pProjectDetailsOrganizationCtrl->SetToolTip(pProjectInfo->m_strOrganization);

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
    wxASSERT(m_pProjectURLStaticCtrl);
    wxASSERT(m_pProjectURLCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Choose a project")
    );

    m_pDescriptionStaticCtrl->SetLabel(
        _("To choose a project, click its name or type its URL below.")
    );

    m_pProjectCategoriesStaticCtrl->SetLabel(
        _("Categories:")
    );

    m_pProjectsStaticCtrl->SetLabel(
        _("Projects:")
    );

    m_pProjectDetailsStaticCtrl->SetLabel(
        _("Project details")
    );

    m_pProjectDetailsResearchAreaStaticCtrl->SetLabel(
        _("Research area:")
    );

    m_pProjectDetailsOrganizationStaticCtrl->SetLabel(
        _("Organization:")
    );

    m_pProjectDetailsURLStaticCtrl->SetLabel(
        _("Web site:")
    );

    m_pProjectDetailsSupportedPlatformsStaticCtrl->SetLabel(
        _("Supported systems:")
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
            pProjectInfo->m_strURL = wxGetTranslation(wxString(pl.projects[i]->url.c_str(), wxConvUTF8));
            pProjectInfo->m_strName = wxGetTranslation(wxString(pl.projects[i]->name.c_str(), wxConvUTF8));
            pProjectInfo->m_strDescription = wxGetTranslation(wxString(pl.projects[i]->description.c_str(), wxConvUTF8));
            pProjectInfo->m_strGeneralArea = wxGetTranslation(wxString(pl.projects[i]->general_area.c_str(), wxConvUTF8));
            pProjectInfo->m_strSpecificArea = wxGetTranslation(wxString(pl.projects[i]->specific_area.c_str(), wxConvUTF8));
            pProjectInfo->m_strOrganization = wxGetTranslation(wxString(pl.projects[i]->home.c_str(), wxConvUTF8));
            
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
            //
            // NOTE: if the platform entry contains a modifier such as [cuda] or [ati], 
            // that capability is required.  If a project offers both a cuda application 
            // and a CPU-only application for an operating system, it must have two 
            // separate platform entries for that OS, one with [cuda] and one without.
            // Likewise for ati and mt.
            //
            for (j = 0;j < aClientPlatforms.size(); j++) {
                wxString strClientPlatform = aClientPlatforms[j];
                for (k = 0;k < aProjectPlatforms.size(); k++) {
                    wxString strProjectPlatform = aProjectPlatforms[k];
                    wxString strRootProjectPlatform = strProjectPlatform.SubString(0, strProjectPlatform.Find(_T("[")) - 1);
                    
                    if (strProjectPlatform.Find(_T("windows")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsWindows = true;
                    }

                    if (strProjectPlatform.Find(_T("apple")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsMac = true;
                    }
                    
                    if (strProjectPlatform.Find(_T("linux")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsLinux = true;
                    }
                    
                    if (strProjectPlatform.Find(_T("[cuda")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsCUDA = true;
						if (!pDoc->state.have_nvidia) continue;
                    }

                    if (strProjectPlatform.Find(_T("[ati")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsCAL = true;
						if (!pDoc->state.have_ati) continue;
                    }

                    if (strClientPlatform == strRootProjectPlatform) {
                        pProjectInfo->m_bSupportedPlatformFound = true;
                    }
                }
            }

			// If project doesn't export its platforms, assume we're supported
			//
			if (aProjectPlatforms.size() == 0) {
				pProjectInfo->m_bSupportedPlatformFound = true;
			}

            wxLogTrace(
                wxT("Function Status"),
                wxT("CProjectInfoPage::OnPageChanged - Windows: '%d', Mac: '%d', Linux: '%d', Nvidia: '%d', ATI: '%d', Platform: '%d'"),
                pProjectInfo->m_bProjectSupportsWindows,
                pProjectInfo->m_bProjectSupportsMac,
                pProjectInfo->m_bProjectSupportsLinux,
                pProjectInfo->m_bProjectSupportsCUDA,
                pProjectInfo->m_bProjectSupportsCAL,
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


void CProjectInfoPage::EllipseStringIfNeeded(wxString& s, wxWindow *win) {
    int x, y;
    int w, h;
    wxSize sz = win->GetParent()->GetSize();
    win->GetPosition(&x, &y);
    int maxWidth = sz.GetWidth() - x - 10;
    
    win->GetTextExtent(s, &w, &h);
    
    // Adapted from ellipis code in wxRendererGeneric::DrawHeaderButtonContents()
    if (w > maxWidth) {
        int ellipsisWidth;
        win->GetTextExtent( wxT("..."), &ellipsisWidth, NULL);
        if (ellipsisWidth > maxWidth) {
            s.Clear();
            w = 0;
        } else {
            do {
                s.Truncate( s.length() - 1 );
                win->GetTextExtent( s, &w, &h);
            } while (((w + ellipsisWidth) > maxWidth) && s.length() );
            s.append( wxT("...") );
            w += ellipsisWidth;
        }
    }
}
