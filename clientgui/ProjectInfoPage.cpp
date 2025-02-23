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
#include "res/androidicon2.xpm"
#include "res/freebsdicon.xpm"
#include "res/linuxarmicon2.xpm"
#include "res/amdicon2.xpm"
#include "res/nvidiaicon2.xpm"
#include "res/intelgpuicon2.xpm"
#include "res/virtualboxicon.xpm"
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
        m_bProjectSupportsAndroid = false;
        m_bProjectSupportsFreeBSD = false;
        m_bProjectSupportsLinuxARM = false;
        m_bProjectSupportsCUDA = false;
        m_bProjectSupportsCAL = false;
        m_bProjectSupportsIntelGPU  = false;
        m_bProjectSupportsVirtualBox = false;
    }

public:
    wxString m_strURL;
    wxString m_strWebURL;
    wxString m_strName;
    wxString m_strDescription;
    wxString m_strGeneralArea;
    wxString m_strSpecificArea;
    wxString m_strOrganization;
    bool m_bSupportedPlatformFound;
    bool m_bProjectSupportsWindows;
    bool m_bProjectSupportsMac;
    bool m_bProjectSupportsLinux;
    bool m_bProjectSupportsAndroid;
    bool m_bProjectSupportsFreeBSD;
    bool m_bProjectSupportsLinuxARM;
    bool m_bProjectSupportsCUDA;
    bool m_bProjectSupportsCAL;
    bool m_bProjectSupportsIntelGPU;
    bool m_bProjectSupportsVirtualBox;
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
    EVT_LIST_ITEM_SELECTED( ID_PROJECTS, CProjectInfoPage::OnProjectSelected )
    EVT_WIZARDEX_PAGE_CHANGED( wxID_ANY, CProjectInfoPage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( wxID_ANY, CProjectInfoPage::OnPageChanging )
    EVT_WIZARDEX_CANCEL( wxID_ANY, CProjectInfoPage::OnCancel )
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

    delete m_apl;
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
    m_pProjectDetailsSupportedPlatformAndroidCtrl = NULL;
    m_pProjectDetailsSupportedPlatformFreeBSDCtrl = NULL;
    m_pProjectDetailsSupportedPlatformLinuxArmCtrl = NULL;
    m_pProjectDetailsSupportedPlatformATICtrl = NULL;
    m_pProjectDetailsSupportedPlatformNvidiaCtrl = NULL;
    m_pProjectDetailsSupportedPlatformIntelGPUCtrl = NULL;
    m_pProjectDetailsSupportedPlatformVirtualBoxCtrl = NULL;
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
    const int descriptionWidth = 350;
#else
    const int descriptionWidth = 310;
#endif

    wxArrayString aCategories;
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    CProjectInfoPage* itemWizardPage23 = this;

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage23->SetSizer(itemBoxSizer24);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage23, wxID_STATIC, _("Choose a project"), wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create( itemWizardPage23, wxID_STATIC, _("To choose a project, click its name or type its URL below."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer24->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(1, 0, 0);
    itemFlexGridSizer4->AddGrowableRow(0);
    itemFlexGridSizer4->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer4, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(2, 0, 0);
    itemFlexGridSizer4->Add(itemFlexGridSizer6, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer6->Add(itemBoxSizer7, 0, wxALIGN_LEFT|wxALIGN_TOP, 0);

    m_pProjectCategoriesStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, _("Categories:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_pProjectCategoriesStaticCtrl, 0, wxALIGN_LEFT|wxRIGHT|wxBOTTOM, 5);

    // We must populate the combo box before our sizers can calculate its width.
    // The combo box will be repopulated in  CProjectInfoPage::OnPageChanged(),
    // so we don't need to worry about duplicate entries here.
    // Get the project list
    m_apl = new ALL_PROJECTS_LIST;
    std::string tempstring;
    if (pDoc) {
        pDoc->rpc.get_all_projects_list(*m_apl);

        for (unsigned int i = 0; i < m_apl->projects.size(); i++) {
            wxString strGeneralArea = wxGetTranslation(wxString(m_apl->projects[i]->general_area.c_str(), wxConvUTF8));
            aCategories.Add(strGeneralArea);
            tempstring = m_apl->projects[i]->url;
            // Canonicalize/trim/store the URLs of all projects. This will be used later on for the wxListBox
            // to visually indicate any projects that are currently attached, as well as checking for when a
            // project or manual URL is selected.
            //
            canonicalize_master_url(tempstring);
            TrimURL(tempstring);
            m_pTrimmedURL.push_back(tempstring);
        }
        // Take all projects that the Client is already attached to and create an array of their
        // canonicalized and trimmed URLs. This will be used for comparing against the master list of projects
        // to visually indicate which projectes have already been attached.
        //
        for (int i = 0; i < pDoc->GetProjectCount(); i++) {
            PROJECT* pProject = pDoc->project(i);
            if (!pProject) {
                continue;
            }
            tempstring = pProject->master_url;
            canonicalize_master_url(tempstring);
            TrimURL(tempstring);
            m_pTrimmedURL_attached.push_back(tempstring);
        }
    }
    m_pProjectCategoriesCtrl = new wxComboBox( itemWizardPage23, ID_CATEGORIES, wxT(""), wxDefaultPosition, wxDefaultSize, aCategories, wxCB_READONLY
#ifndef __WXMAC__   // wxCB_SORT is not available in wxCocoa 3.0
    |wxCB_SORT
#endif
    );
    itemBoxSizer7->Add(m_pProjectCategoriesCtrl, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    m_pProjectsStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, _("Projects:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_pProjectsStaticCtrl, 0, wxALIGN_LEFT|wxTOP|wxRIGHT|wxBOTTOM, 5);

    wxFlexGridSizer* itemFlexGridSizer11 = new wxFlexGridSizer(1, 0, 0);
    itemFlexGridSizer11->AddGrowableRow(0);
    itemFlexGridSizer11->AddGrowableCol(0);
    itemBoxSizer7->Add(itemFlexGridSizer11, 0, wxGROW|wxALL, 0);

    m_pProjectsCtrl = new wxListCtrl(itemWizardPage23, ID_PROJECTS, wxDefaultPosition, wxSize(-1, 175), wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING);
    m_pProjectsCtrl->InsertColumn(0, wxT(""));
    m_pProjectsCtrl->SetColumnWidth(0, -2);
    itemFlexGridSizer11->Add(m_pProjectsCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    m_pProjectDetailsStaticCtrl = new wxStaticBox(itemWizardPage23, wxID_ANY, _("Project details"));
    wxStaticBoxSizer* itemStaticBoxSizer13 = new wxStaticBoxSizer(m_pProjectDetailsStaticCtrl, wxVERTICAL);
    itemFlexGridSizer6->Add(itemStaticBoxSizer13, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectDetailsDescriptionCtrl = new wxTextCtrl( itemWizardPage23, ID_PROJECTDESCRIPTION, wxT(""), wxDefaultPosition, wxSize(descriptionWidth, 100), wxTE_MULTILINE|wxTE_READONLY );
    itemStaticBoxSizer13->Add(m_pProjectDetailsDescriptionCtrl, 0, wxGROW|wxLEFT|wxTOP|wxBOTTOM, 5);
    wxFlexGridSizer* itemFlexGridSizer16 = new wxFlexGridSizer(2, 0, 0);
    itemFlexGridSizer16->AddGrowableCol(1);
    itemStaticBoxSizer13->Add(itemFlexGridSizer16, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsResearchAreaStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, _("Research area:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(m_pProjectDetailsResearchAreaStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 2);

    m_pProjectDetailsResearchAreaCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(m_pProjectDetailsResearchAreaCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxFlexGridSizer* itemFlexGridSizer19 = new wxFlexGridSizer(2, 0, 0);
    itemFlexGridSizer19->AddGrowableCol(1);
    itemStaticBoxSizer13->Add(itemFlexGridSizer19, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsOrganizationStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, _("Organization:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(m_pProjectDetailsOrganizationStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 2);

    m_pProjectDetailsOrganizationCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(m_pProjectDetailsOrganizationCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxFlexGridSizer* itemFlexGridSizer20 = new wxFlexGridSizer(2, 0, 0);
    itemFlexGridSizer20->AddGrowableCol(1);
    itemStaticBoxSizer13->Add(itemFlexGridSizer20, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsURLStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, _("Web site:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer20->Add(m_pProjectDetailsURLStaticCtrl, 0, wxALIGN_LEFT|wxRIGHT|wxBOTTOM, 2);

    m_pProjectDetailsURLCtrl = new wxHyperlinkCtrl( itemWizardPage23, wxID_STATIC, wxT("BOINC"), wxT("https://boinc.berkeley.edu/"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT);
    itemFlexGridSizer20->Add(m_pProjectDetailsURLCtrl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxFlexGridSizer* itemFlexGridSizer24 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer24->AddGrowableRow(1);
    itemFlexGridSizer24->AddGrowableCol(0);
    itemStaticBoxSizer13->Add(itemFlexGridSizer24, 0, wxGROW|wxALL, 0);

    m_pProjectDetailsSupportedPlatformsStaticCtrl = new wxStaticText( itemWizardPage23, wxID_STATIC, _("Supported systems:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer24->Add(m_pProjectDetailsSupportedPlatformsStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer24->Add(itemBoxSizer26, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    m_pProjectDetailsSupportedPlatformWindowsCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("windowsicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformWindowsCtrl->SetToolTip(_("Supports Microsoft Windows"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformWindowsCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformMacCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("macosicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformMacCtrl->SetToolTip(_("Supports Mac OS X"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformMacCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformLinuxCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("linuxicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformLinuxCtrl->SetToolTip(_("Supports Linux on Intel"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformLinuxCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformAndroidCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("androidicon2.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformAndroidCtrl->SetToolTip(_("Supports Android"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformAndroidCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformFreeBSDCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("freebsdicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformFreeBSDCtrl->SetToolTip(_("Supports FreeBSD"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformFreeBSDCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformLinuxArmCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("linuxarmicon2.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformLinuxArmCtrl->SetToolTip(_("Supports Linux on ARM (e.g. Raspberry Pi)"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformLinuxArmCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformNvidiaCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("nvidiaicon2.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformNvidiaCtrl->SetToolTip(_("Supports NVIDIA GPUs"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformNvidiaCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformATICtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("amdicon2.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformATICtrl->SetToolTip(_("Supports AMD GPUs"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformATICtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformIntelGPUCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("intelgpuicon2.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformIntelGPUCtrl->SetToolTip(_("Supports Intel GPUs"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformIntelGPUCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformVirtualBoxCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("virtualboxicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    m_pProjectDetailsSupportedPlatformVirtualBoxCtrl->SetToolTip(_("Supports VirtualBox"));
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformVirtualBoxCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_pProjectDetailsSupportedPlatformBlankCtrl = new wxStaticBitmap( itemWizardPage23, wxID_STATIC, GetBitmapResource(wxT("blankicon.xpm")), wxDefaultPosition, wxSize(16,16), 0 );
    itemBoxSizer26->Add(m_pProjectDetailsSupportedPlatformBlankCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxFlexGridSizer* itemFlexGridSizer33 = new wxFlexGridSizer(2, 0, 0);
    itemFlexGridSizer33->AddGrowableCol(1);
    itemFlexGridSizer4->Add(itemFlexGridSizer33, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pProjectURLStaticCtrl = new wxStaticText( itemWizardPage23, ID_PROJECTURLSTATICCTRL, _("Project URL:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer33->Add(m_pProjectURLStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectURLCtrl = new wxTextCtrl( itemWizardPage23, ID_PROJECTURLCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer33->Add(m_pProjectURLCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer33->Add(0, 10, 0);

    // Set validators
    m_pProjectURLCtrl->SetValidator(CValidateURL(&m_strProjectURL));

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
    else if (name == wxT("androidicon2.xpm"))
    {
        wxBitmap bitmap(androidicon2_xpm);
        return bitmap;
    }
    else if (name == wxT("freebsdicon.xpm"))
    {
        wxBitmap bitmap(FreeBSD_xpm);
        return bitmap;
    }
    else if (name == wxT("linuxarmicon2.xpm"))
    {
        wxBitmap bitmap(linuxarmicon2_xpm);
        return bitmap;
    }
    else if (name == wxT("amdicon2.xpm"))
    {
        wxBitmap bitmap(amdicon2_xpm);
        return bitmap;
    }
    else if (name == wxT("nvidiaicon2.xpm"))
    {
        wxBitmap bitmap(nvidiaicon2_xpm);
        return bitmap;
    }
    else if (name == wxT("intelgpuicon2.xpm"))
    {
        wxBitmap bitmap(intelgpuicon2_xpm);
        return bitmap;
    }
    else if (name == wxT("virtualboxicon.xpm"))
    {
        wxBitmap bitmap(virtualboxicon_xpm);
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

    m_pProjectsCtrl->DeleteAllItems();

    long lastproject = -1;
    //  Color 117,117,117 has a 4.6:1 contract ratio that passes accessibility
    wxColour addedcolour = wxColour(117, 117, 117);
    // Populate the list control with the list of project names that belong to either the specific
    // category or all of them.
    //
    for (unsigned int i=0; i<m_Projects.size(); i++) {  // cycle through all projects
        if ((m_pProjectCategoriesCtrl->GetValue() == _("All")) ||
            (m_pProjectCategoriesCtrl->GetValue() == m_Projects[i]->m_strGeneralArea)
        ) {
            lastproject = m_pProjectsCtrl->InsertItem(i, m_Projects[i]->m_strName);
            if (lastproject != -1) {
                m_pProjectsCtrl->SetItemPtrData(lastproject, reinterpret_cast<wxUIntPtr>(m_Projects[i]));
                // Since this project was added to the wxListCtrl, check to see if the project has already been attached.
                // If it has, grey out the text for a visual indicator that the project has been added.
                //
                for (unsigned int j = 0; j < m_pTrimmedURL_attached.size(); j++) {  // cycle through all attached projects
                    if (m_pTrimmedURL[i] == m_pTrimmedURL_attached[j]) {
                        m_pProjectsCtrl->SetItemTextColour(lastproject, addedcolour);
                        break;
                    }
                }
            }
        }
    }
    // Adjust the size of the column width so that a horizontal scroll bar doesn't appear when a vertical scroll bar is present.
    int width = m_pProjectsCtrl->GetClientSize().GetWidth();
    m_pProjectsCtrl->SetColumnWidth(0, width);

    // Set the first item to be the selected and focused item.
    if (!m_pProjectsCtrl->IsEmpty()) {
        m_pProjectsCtrl->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        m_pProjectsCtrl->SetItemState(0, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnProjectCategorySelected - Function End"));
}


/*
 * wxEVT_LIST_ITEM_SELECTED event handler for ID_PROJECTS
 */

void CProjectInfoPage::OnProjectSelected( wxListEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectInfoPage::OnProjectSelected - Function Begin"));

    if (m_pProjectsCtrl->GetSelectedItemCount() == 1) {
        wxListItem pProjectSelected;
        pProjectSelected.SetId(event.m_itemIndex);
        pProjectSelected.SetColumn(0);
        pProjectSelected.SetMask(wxLIST_MASK_TEXT);

        const CProjectInfo* pProjectInfo = reinterpret_cast<CProjectInfo*>(m_pProjectsCtrl->GetItemData(pProjectSelected.GetId()));
        if (pProjectInfo) {
            wxString strWebURL = pProjectInfo->m_strWebURL;
            EllipseStringIfNeeded(strWebURL, m_pProjectDetailsURLCtrl);

            // Populate the project details area
            wxString desc = pProjectInfo->m_strDescription;
            // Change all occurrences of "<sup>n</sup>" to "^n"
            desc.Replace(wxT("<sup>"), wxT("^"), true);
            desc.Replace(wxT("</sup>"), wxT(""), true);
            desc.Replace(wxT("&lt;"), wxT("<"), true);

            m_pProjectDetailsURLCtrl->SetLabel(strWebURL);
            m_pProjectDetailsURLCtrl->SetURL(pProjectInfo->m_strWebURL);
            m_pProjectDetailsURLCtrl->SetToolTip(pProjectInfo->m_strWebURL);
            m_pProjectDetailsDescriptionCtrl->SetValue(desc);

            m_pProjectDetailsSupportedPlatformWindowsCtrl->Hide();
            m_pProjectDetailsSupportedPlatformMacCtrl->Hide();
            m_pProjectDetailsSupportedPlatformLinuxCtrl->Hide();
            m_pProjectDetailsSupportedPlatformAndroidCtrl->Hide();
            m_pProjectDetailsSupportedPlatformFreeBSDCtrl->Hide();
            m_pProjectDetailsSupportedPlatformLinuxArmCtrl->Hide();
            m_pProjectDetailsSupportedPlatformNvidiaCtrl->Hide();
            m_pProjectDetailsSupportedPlatformATICtrl->Hide();
            m_pProjectDetailsSupportedPlatformIntelGPUCtrl->Hide();
            m_pProjectDetailsSupportedPlatformVirtualBoxCtrl->Hide();
            if (pProjectInfo->m_bProjectSupportsWindows) m_pProjectDetailsSupportedPlatformWindowsCtrl->Show();
            if (pProjectInfo->m_bProjectSupportsMac) m_pProjectDetailsSupportedPlatformMacCtrl->Show();
            if (pProjectInfo->m_bProjectSupportsLinux) m_pProjectDetailsSupportedPlatformLinuxCtrl->Show();
            if (pProjectInfo->m_bProjectSupportsAndroid) m_pProjectDetailsSupportedPlatformAndroidCtrl->Show();
            if (pProjectInfo->m_bProjectSupportsFreeBSD) m_pProjectDetailsSupportedPlatformFreeBSDCtrl->Show();
            if (pProjectInfo->m_bProjectSupportsLinuxARM) m_pProjectDetailsSupportedPlatformLinuxArmCtrl->Show();
            if (pProjectInfo->m_bProjectSupportsCAL) m_pProjectDetailsSupportedPlatformATICtrl->Show();
            if (pProjectInfo->m_bProjectSupportsCUDA) m_pProjectDetailsSupportedPlatformNvidiaCtrl->Show();
            if (pProjectInfo->m_bProjectSupportsIntelGPU) m_pProjectDetailsSupportedPlatformIntelGPUCtrl->Show();
            if (pProjectInfo->m_bProjectSupportsVirtualBox) m_pProjectDetailsSupportedPlatformVirtualBoxCtrl->Show();

            // Populate non-control data for use in other places of the wizard
            m_strProjectURL = pProjectInfo->m_strURL;
            m_bProjectSupported = pProjectInfo->m_bSupportedPlatformFound;

            Layout();
            TransferDataToWindow();

            wxString strResearchArea = pProjectInfo->m_strSpecificArea;
            EllipseStringIfNeeded(strResearchArea, m_pProjectDetailsResearchAreaCtrl);
            wxString strOrganization = pProjectInfo->m_strOrganization;
            EllipseStringIfNeeded(strOrganization, m_pProjectDetailsOrganizationCtrl);

            m_pProjectDetailsResearchAreaCtrl->SetLabel(strResearchArea);
            m_pProjectDetailsResearchAreaCtrl->SetToolTip(pProjectInfo->m_strSpecificArea);
            m_pProjectDetailsOrganizationCtrl->SetLabel(strOrganization);
            m_pProjectDetailsOrganizationCtrl->SetToolTip(pProjectInfo->m_strOrganization);
        }
    }

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
    wxArrayString               aClientPlatforms;
    wxArrayString               aProjectPlatforms;
    wxArrayString               aCategories;
    bool                        bCategoryFound = false;
    CProjectInfo*               pProjectInfo = NULL;


    // Populate the ProjectInfo data structure with the list of projects we want to show and
    // any other activity we need to prep the page.
    if (!m_bProjectListPopulated) {

        // Convert the supported client platforms into something useful
        for (i=0; i<pDoc->state.platforms.size(); i++) {
            aClientPlatforms.Add(wxString(pDoc->state.platforms[i].c_str(), wxConvUTF8));
        }

        // Iterate through the project list and add them to the ProjectInfo data structure
        for (i=0; i<m_apl->projects.size(); i++) {
            pProjectInfo = new CProjectInfo();
            m_Projects.push_back(pProjectInfo);

            // Convert the easy stuff
            pProjectInfo->m_strURL = wxGetTranslation(wxString(m_apl->projects[i]->url.c_str(), wxConvUTF8));
            pProjectInfo->m_strWebURL = wxGetTranslation(wxString(m_apl->projects[i]->web_url.c_str(), wxConvUTF8));
            pProjectInfo->m_strName = wxGetTranslation(wxString(m_apl->projects[i]->name.c_str(), wxConvUTF8));
            pProjectInfo->m_strDescription = wxGetTranslation(wxString(m_apl->projects[i]->description.c_str(), wxConvUTF8));
            pProjectInfo->m_strGeneralArea = wxGetTranslation(wxString(m_apl->projects[i]->general_area.c_str(), wxConvUTF8));
            pProjectInfo->m_strSpecificArea = wxGetTranslation(wxString(m_apl->projects[i]->specific_area.c_str(), wxConvUTF8));
            pProjectInfo->m_strOrganization = wxGetTranslation(wxString(m_apl->projects[i]->home.c_str(), wxConvUTF8));

            // Use project URL if web_url is not available
            if (!pProjectInfo->m_strWebURL) {
                pProjectInfo->m_strWebURL = pProjectInfo->m_strURL;
            }

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
            for (j=0; j<m_apl->projects[i]->platforms.size(); j++) {
                aProjectPlatforms.Add(wxString(m_apl->projects[i]->platforms[j].c_str(), wxConvUTF8));
            }

            // Can the core client support a platform that this project supports?
            //
            for (j = 0;j < aClientPlatforms.size(); j++) {
                wxString strClientPlatform = aClientPlatforms[j];
                for (k = 0;k < aProjectPlatforms.size(); k++) {
                    wxString strProjectPlatform = aProjectPlatforms[k];
                    wxString strRootProjectPlatform = strProjectPlatform.SubString(0, strProjectPlatform.Find(_T("[")) - 1);
                    wxString strProjectPlanClass = strProjectPlatform.Mid(strProjectPlatform.Find(_T("[")));

                    if (strProjectPlatform.Find(_T("windows")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsWindows = true;
                    }

                    if (strProjectPlatform.Find(_T("apple")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsMac = true;
                    }

                    if (strProjectPlatform.Find(_T("linux")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsLinux = true;
                    }

                    if (strProjectPlatform.Find(_T("android")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsAndroid = true;
                    }

                    if (strProjectPlatform.Find(_T("freebsd")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsFreeBSD = true;
                    }

                    if (strProjectPlatform.Find(_T("arm-unknown-linux-gnu")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsLinuxARM = true;
                    }

                    if (strProjectPlatform.Find(_T("arm-unknown-linux-gnueabihf")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsLinuxARM = true;
                    }

                    if (strProjectPlanClass.Find(_T("cuda")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsCUDA = true;
                        if (!pDoc->state.host_info.coprocs.have_nvidia()) continue;
                    }

                    if (strProjectPlanClass.Find(_T("nvidia")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsCUDA = true;
                        if (!pDoc->state.host_info.coprocs.have_nvidia()) continue;
                    }

                    if (strProjectPlanClass.Find(_T("ati")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsCAL = true;
                        if (!pDoc->state.host_info.coprocs.have_ati()) continue;
                    }

                    if (strProjectPlanClass.Find(_T("amd")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsCAL = true;
                        if (!pDoc->state.host_info.coprocs.have_ati()) continue;
                    }

                    if (strProjectPlanClass.Find(_T("intel_gpu")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsIntelGPU = true;
                        if (!pDoc->state.host_info.coprocs.have_intel_gpu()) continue;
                    }

                    if (strProjectPlanClass.Find(_T("vbox")) != wxNOT_FOUND) {
                        pProjectInfo->m_bProjectSupportsVirtualBox = true;
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
        }


        // Populate the category combo box
        if (!m_pProjectCategoriesCtrl->IsListEmpty()) {
            m_pProjectCategoriesCtrl->Clear();
        }
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

    CWizardAttach* pWA = ((CWizardAttach*)GetParent());
    CMainDocument* pDoc = wxGetApp().GetDocument();
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString       strTitle;
    int            iAnswer;
    bool           bAlreadyAttached = false;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    strTitle.Printf(
        wxT("%s"),
        pSkinAdvanced->GetApplicationName().c_str()
    );


    // Check to see if the project is supported:
    if (!m_bProjectSupported) {

        iAnswer = wxGetApp().SafeMessageBox(
            _("This project may not have work for your type of computer.  Do you want to add it anyway?"),
            strTitle,
            wxCENTER | wxYES_NO | wxICON_INFORMATION
        );

        // Project is not supported
        if (wxNO == iAnswer) {
            event.Veto();
        }

    }

    // Check if we are already attached to that project:
    const std::string http = "http://";
    const std::string https = "https://";

    std::string new_project_url = (const char*)m_strProjectURL.mb_str();
    canonicalize_master_url(new_project_url);
    TrimURL(new_project_url);
     for (int i = 0; i < pDoc->GetProjectCount(); ++i) {
        PROJECT* project = pDoc->project(i);
        if (project) {
            std::string project_url = project->master_url;

            canonicalize_master_url(project_url);
            TrimURL(project_url);

            if (project_url == new_project_url) {
                bAlreadyAttached = true;
                break;
            }
        }
    }

    if (bAlreadyAttached) {

        wxGetApp().SafeMessageBox(
            _("You already added this project. Please choose a different project."),
            strTitle,
            wxCENTER | wxOK | wxICON_INFORMATION
        );
        event.Veto();

    } else {

        // Update authoritative data in CWizardAttach
        pWA->SetProjectURL(m_strProjectURL);

    }
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

void CProjectInfoPage::RefreshPage() {
    // Trigger initial event to populate the list control
    wxCommandEvent evtEvent(wxEVT_COMMAND_COMBOBOX_SELECTED, ID_CATEGORIES);
    ProcessEvent(evtEvent);
}


// Function to "trim" the URL of the http(s) prefix and the last slash.
// Prior to running this function, the string should be canonicalized using
// the canonicalize_master_url function.
//
void CProjectInfoPage::TrimURL(std::string& purl) {
    const std::string http = "http://";
    const std::string https = "https://";
    // Remove http(s):// at the beginning of project address
    // there is no reason to connect to secure address project
    // if we're already connected to the non-secure address
    // or vice versa
    // also clear last '/' character if present
    //
    size_t pos = purl.find(http);
    if (pos != std::string::npos) {
        purl.erase(pos, http.length());
    }
    else if ((pos = purl.find(https)) != std::string::npos) {
        purl.erase(pos, https.length());
    }
    if (purl.length() >= 1 && purl[purl.length() - 1] == '/') {
        purl.erase(purl.length() - 1, 1);
    }
}
