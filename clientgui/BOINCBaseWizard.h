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

#ifndef BOINC_BOINCBASEWIZARD_H
#define BOINC_BOINCBASEWIZARD_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCBaseWizard.cpp"
#endif

/*!
 * CBOINCBaseWizard class declaration
 */

class CBOINCBaseWizard: public wxWizardEx {
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
};

#endif
