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
#ifndef BOINC_ACCOUNTMANAGERINFOPAGE_H
#define BOINC_ACCOUNTMANAGERINFOPAGE_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "AccountManagerInfoPage.cpp"
#endif

/*!
 * CProjectListItem class declaration
 */

class CAcctMgrListItem: public wxObject
{
    DECLARE_DYNAMIC_CLASS( CAcctMgrListItem )
public:

    wxString GetURL() const { return m_strURL ; }
    void SetURL(wxString value) { m_strURL = value ; }

    wxString GetName() const { return m_strName ; }
    void SetName(wxString value) { m_strName = value ; }

    wxString GetImage() const { return m_strImage ; }
    void SetImage(wxString value) { m_strImage = value ; }

    wxString GetDescription() const { return m_strDescription ; }
    void SetDescription(wxString value) { m_strDescription = value ; }

private:
    wxString m_strURL;
    wxString m_strName;
    wxString m_strImage;
    wxString m_strDescription;
};


/*!
 * CAccountManagerInfoPage class declaration
 */

class CAccountManagerInfoPage: public wxWizardPageEx
{
    DECLARE_DYNAMIC_CLASS( CAccountManagerInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountManagerInfoPage( );

    CAccountManagerInfoPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountManagerInfoPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanging( wxWizardExEvent& event );

    /// wxEVT_LISTBOX event handler for ID_PROJECTS
    void OnProjectSelected( wxCommandEvent& event );

    /// wxEVT_TEXT event handler for ID_PROJECTURLCTRL
    void OnURLChanged( wxCommandEvent& event );

    /// wxEVT_BUTTON event handler for ID_PROJECTWEBPAGECTRL
    void OnProjectItemDisplay( wxCommandEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CAccountManagerInfoPage event handler declarations

////@begin CAccountManagerInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAccountManagerInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAccountManagerInfoPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDescriptionStaticCtrl;
    wxListBox* m_pProjectListCtrl;
    wxStaticText* m_pProjectDetailsStaticCtrl;
    wxTextCtrl* m_pProjectDetailsDescriptionCtrl;
    wxButton* m_pOpenWebSiteButton;
    wxStaticText* m_pProjectUrlStaticCtrl;
    wxTextCtrl* m_pProjectUrlCtrl;
////@end CAccountManagerInfoPage member variables
    bool m_bAccountManagerListPopulated;
};

#endif
