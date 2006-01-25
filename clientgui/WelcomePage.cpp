// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "WelcomePage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WelcomePage.h"

////@begin XPM images
////@end XPM images


/*!
 * CWelcomePage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CWelcomePage, wxWizardPageEx )
 
/*!
 * CWelcomePage event table definition
 */
 
BEGIN_EVENT_TABLE( CWelcomePage, wxWizardPageEx )
 
////@begin CWelcomePage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CWelcomePage::OnPageChanged )
    EVT_WIZARDEX_PAGE_CHANGING( -1, CWelcomePage::OnPageChanging )
    EVT_WIZARDEX_CANCEL( -1, CWelcomePage::OnCancel )

////@end CWelcomePage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CWelcomePage constructors
 */
 
CWelcomePage::CWelcomePage( )
{
}
 
CWelcomePage::CWelcomePage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * WizardPage creator
 */
 
bool CWelcomePage::Create( CBOINCBaseWizard* parent )
{

////@begin CWelcomePage member initialisation
#if defined(__WXDEBUG__)
    m_ErrProjectPropertiesCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectCommCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectPropertiesURLCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrAccountCreationDisabledCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrClientAccountCreationDisabledCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrAccountAlreadyExistsCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectAlreadyAttachedCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrProjectAttachFailureCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrGoogleCommCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrYahooCommCtrl = NULL;
#endif
#if defined(__WXDEBUG__)
    m_ErrNetDetectionCtrl = NULL;
#endif
////@end CWelcomePage member initialisation
 
////@begin CWelcomePage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CWelcomePage creation

    return TRUE;
}
 
/*!
 * Control creation for WizardPage
 */
 
void CWelcomePage::CreateControls()
{    
////@begin CWelcomePage content construction
    CWelcomePage* itemWizardPage2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage2->SetSizer(itemBoxSizer3);

    wxStaticText* m_Title = new wxStaticText;
    wxStaticText* m_Description = new wxStaticText;
    if (IS_ATTACHTOPROJECTWIZARD()) {
        m_Title->Create( itemWizardPage2, wxID_STATIC, _("Attach to project"), wxDefaultPosition, wxDefaultSize, 0 );
        m_Title->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
        itemBoxSizer3->Add(m_Title, 0, wxALIGN_LEFT|wxALL, 5);

        m_Description->Create( itemWizardPage2, wxID_STATIC, _("We'll now guide you through the process of attaching to a project."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer3->Add(m_Description, 0, wxALIGN_LEFT|wxALL, 5);
    }

    if (IS_ACCOUNTMANAGERATTACHWIZARD()) {
        m_Title->Create( itemWizardPage2, wxID_STATIC, _("Account manager"), wxDefaultPosition, wxDefaultSize, 0 );
        m_Title->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
        itemBoxSizer3->Add(m_Title, 0, wxALIGN_LEFT|wxALL, 5);

        m_Description->Create( itemWizardPage2, wxID_STATIC, _("We'll now guide you through the process of \nadding/updating/removing an account manager."), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer3->Add(m_Description, 0, wxALIGN_LEFT|wxALL, 5);
    }

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

#if defined(__WXDEBUG__)
    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemWizardPage2, wxID_ANY, _("Debug Flags"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(-1, 2, 0, 0);
    itemFlexGridSizer8->AddGrowableCol(0);
    itemFlexGridSizer8->AddGrowableCol(1);
    itemStaticBoxSizer7->Add(itemFlexGridSizer8, 0, wxGROW|wxALL, 5);

    m_ErrProjectPropertiesCtrl = new wxCheckBox;
    m_ErrProjectPropertiesCtrl->Create( itemWizardPage2, ID_ERRPROJECTPROPERTIES, _("Project Properties Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectPropertiesCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectPropertiesCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectCommCtrl = new wxCheckBox;
    m_ErrProjectCommCtrl->Create( itemWizardPage2, ID_ERRPROJECTCOMM, _("Project Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectPropertiesURLCtrl = new wxCheckBox;
    m_ErrProjectPropertiesURLCtrl->Create( itemWizardPage2, ID_ERRPROJECTPROPERTIESURL, _("Project Properties URL Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectPropertiesURLCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectPropertiesURLCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrAccountCreationDisabledCtrl = new wxCheckBox;
    m_ErrAccountCreationDisabledCtrl->Create( itemWizardPage2, ID_ERRACCOUNTCREATIONDISABLED, _("Account Creation Disabled"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrAccountCreationDisabledCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrAccountCreationDisabledCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrClientAccountCreationDisabledCtrl = new wxCheckBox;
    m_ErrClientAccountCreationDisabledCtrl->Create( itemWizardPage2, ID_ERRCLIENTACCOUNTCREATIONDISABLED, _("Client Account Creation Disabled"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrClientAccountCreationDisabledCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrClientAccountCreationDisabledCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrAccountAlreadyExistsCtrl = new wxCheckBox;
    m_ErrAccountAlreadyExistsCtrl->Create( itemWizardPage2, ID_ERRACCOUNTALREADYEXISTS, _("Account Already Exists"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrAccountAlreadyExistsCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrAccountAlreadyExistsCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectAlreadyAttachedCtrl = new wxCheckBox;
    m_ErrProjectAlreadyAttachedCtrl->Create( itemWizardPage2, ID_ERRPROJECTALREADYATTACHED, _("Project Already Attached"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectAlreadyAttachedCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectAlreadyAttachedCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrProjectAttachFailureCtrl = new wxCheckBox;
    m_ErrProjectAttachFailureCtrl->Create( itemWizardPage2, ID_ERRPROJECTATTACHFAILURE, _("Project Attach Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrProjectAttachFailureCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrProjectAttachFailureCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrGoogleCommCtrl = new wxCheckBox;
    m_ErrGoogleCommCtrl->Create( itemWizardPage2, ID_ERRGOOGLECOMM, _("Google Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrGoogleCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrGoogleCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ErrYahooCommCtrl = new wxCheckBox;
    m_ErrYahooCommCtrl->Create( itemWizardPage2, ID_ERRYAHOOCOMM, _("Yahoo Comm Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrYahooCommCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrYahooCommCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    itemFlexGridSizer8->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ErrNetDetectionCtrl = new wxCheckBox;
    m_ErrNetDetectionCtrl->Create( itemWizardPage2, ID_ERRNETDETECTION, _("Net Detection Failure"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ErrNetDetectionCtrl->SetValue(FALSE);
    itemFlexGridSizer8->Add(m_ErrNetDetectionCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

#endif

    wxStaticText* itemStaticText22 = new wxStaticText;
    itemStaticText22->Create( itemWizardPage2, wxID_STATIC, _("To continue, click Next."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText22, 0, wxALIGN_LEFT|wxALL, 5);

////@end CWelcomePage content construction

}

/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CWelcomePage::GetPrev() const
{
    return NULL;
}
 
/*!
 * Gets the next page.
 */
 
wxWizardPageEx* CWelcomePage::GetNext() const
{
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else if (IS_ATTACHTOPROJECTWIZARD()) {
        return PAGE_TRANSITION_NEXT(ID_PROJECTINFOPAGE);
    } else if (IS_ACCOUNTMANAGERATTACHWIZARD()) {
        return PAGE_TRANSITION_NEXT(ID_ACCOUNTMANAGERINFOPAGE);
    }
    return NULL;
}
 
/*!
 * Should we show tooltips?
 */
 
bool CWelcomePage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CWelcomePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWelcomePage bitmap retrieval
    return wxNullBitmap;
////@end CWelcomePage bitmap retrieval
}
 
/*!
 * Get icon resources
 */
 
wxIcon CWelcomePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWelcomePage icon retrieval
    return wxNullIcon;
////@end CWelcomePage icon retrieval
}
   
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WELCOMEPAGE
 */
 
void CWelcomePage::OnPageChanged( wxWizardExEvent& event ) {
    event.Skip();
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE
 */

void CWelcomePage::OnPageChanging( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;
 
    unsigned long ulFlags = 0;
 
#if defined(__WXDEBUG__)
    if (m_ErrProjectPropertiesCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTPROPERTIES;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrProjectPropertiesURLCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTPROPERTIESURL;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrProjectCommCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTCOMM;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrGoogleCommCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRGOOGLECOMM;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrYahooCommCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRYAHOOCOMM;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrAccountAlreadyExistsCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRACCOUNTALREADYEXISTS;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrAccountCreationDisabledCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRACCOUNTCREATIONDISABLED;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrClientAccountCreationDisabledCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRCLIENTACCOUNTCREATIONDISABLED;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrProjectAttachFailureCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTATTACH;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrProjectAlreadyAttachedCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRPROJECTALREADYATTACHED;
#endif
#if defined(__WXDEBUG__)
    if (m_ErrNetDetectionCtrl->GetValue()) 
        ulFlags |= WIZDEBUG_ERRNETDETECTION;
#endif
 
    PROCESS_DEBUG_FLAG( ulFlags );
}
  
/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE
 */
 
void CWelcomePage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}

