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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
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
             const wxBitmap& bitmap = wxNullBitmap,
             const wxPoint& pos = wxDefaultPosition,
             long style = wxDEFAULT_DIALOG_STYLE);

    /// Diagnostics functions
    virtual void SetDiagFlags( unsigned long ulFlags );
    virtual bool IsDiagFlagsSet( unsigned long ulFlags );
    unsigned long m_ulDiagFlags;

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

    /// Wizard Detection
    bool IsAttachToProjectWizard;
    bool IsAccountManagerWizard;
    bool IsAccountManagerUpdateWizard;
    bool IsAccountManagerRemoveWizard;

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
