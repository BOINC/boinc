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

#ifndef _WIZ_BOINCBASEWIZARD_H_
#define _WIZ_BOINCBASEWIZARD_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCBaseWizard.cpp"
#endif

/*!
 * CBOINCBaseWizard class declaration
 */

class CBOINCBaseWizard: public wxWizardEx
{    
    DECLARE_DYNAMIC_CLASS( CBOINCBaseWizard )

public:
    /// Constructors
    CBOINCBaseWizard();
    CBOINCBaseWizard(wxWindow *parent,
             int id = wxID_ANY,
             const wxString& title = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition,
             long style = wxDEFAULT_DIALOG_STYLE);

    /// Track page transitions
    wxWizardPageEx* PopPageTransition();
    virtual wxWizardPageEx* _PopPageTransition();
    wxWizardPageEx* PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID );
    virtual wxWizardPageEx* _PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID );
    std::stack<wxWizardPageEx*> m_PageTransition;

    /// Cancel Event Infrastructure
    bool IsCancelInProgress() const;
    void ProcessCancelEvent( wxWizardExEvent& event );
    virtual void _ProcessCancelEvent( wxWizardExEvent& event );
    bool m_bCancelInProgress;

    /// Button State Infrastructure
    wxButton* GetNextButton() const;
    void SimulateNextButton();
    void EnableNextButton();
    void DisableNextButton();
    wxButton* GetBackButton() const;
    void SimulateBackButton();
    void EnableBackButton();
    void DisableBackButton();
    wxButton* GetCancelButton() const;
    void SimulateCancelButton();
    void EnableCancelButton();
    void DisableCancelButton();

    /// Wizard Detection
    bool IsAttachToProjectWizard;
    bool IsAccountManagerWizard;
    bool IsAccountManagerUpdateWizard;

    /// Global Wizard Status
    PROJECT_CONFIG      project_config;
    ACCOUNT_IN          account_in;
    ACCOUNT_OUT         account_out;
    bool                account_created_successfully;
    bool                attached_to_project_successfully;
    bool                close_when_completed;
    wxString            project_name;
    wxString            project_url;
    wxString            project_authenticator;
};

#endif // _WIZ_BOINCBASEWIZARD_H_
