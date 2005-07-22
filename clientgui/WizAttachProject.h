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
#ifndef _WIZATTACHPROJECT_H_
#define _WIZATTACHPROJECT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "WizAttachProject.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/wizard.h"
#include "hyperlink.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class WelcomePage;
class ProjectInfoPage;
class AccountInfoPage;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ATTACHPROJECTWIZARD 10032
#define SYMBOL_CWIZATTACHPROJECT_IDNAME ID_ATTACHPROJECTWIZARD
#define ID_WELCOMEPAGE 10033
#define ID_PROJECTINFOPAGE 10034
#define ID_PROJECTURLCTRL 10035
#define ID_PROJECRINFOBOINCLINK 10036
#define ID_ACCOUNTINFOPAGE 10037
#define ID_ACCOUNTCREATEBUTTON 10038
#define ID_ACCOUNTUSEXISTINGBUTTON 10039
#define ID_TEXTCTRL 10040
#define ID_TEXTCTRL1 10041
#define ID_TEXTCTRL2 10042
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * CWizAttachProject class declaration
 */

class CWizAttachProject: public wxWizard
{    
    DECLARE_DYNAMIC_CLASS( CWizAttachProject )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWizAttachProject( );
    CWizAttachProject( wxWindow* parent, wxWindowID id = SYMBOL_CWIZATTACHPROJECT_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWIZATTACHPROJECT_IDNAME, const wxPoint& pos = wxDefaultPosition );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CWizAttachProject event handler declarations

////@end CWizAttachProject event handler declarations

////@begin CWizAttachProject member function declarations

    /// Runs the wizard.
    bool Run();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWizAttachProject member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWizAttachProject member variables
    WelcomePage* m_WelcomePage;
    ProjectInfoPage* m_ProjectInfoPage;
    AccountInfoPage* m_AccountInfoPage;
////@end CWizAttachProject member variables
};

/*!
 * WelcomePage class declaration
 */

class WelcomePage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( WelcomePage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WelcomePage( );

    WelcomePage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin WelcomePage event handler declarations

////@end WelcomePage event handler declarations

////@begin WelcomePage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WelcomePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin WelcomePage member variables
////@end WelcomePage member variables
};

/*!
 * ProjectInfoPage class declaration
 */

class ProjectInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( ProjectInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ProjectInfoPage( );

    ProjectInfoPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin ProjectInfoPage event handler declarations

////@end ProjectInfoPage event handler declarations

////@begin ProjectInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ProjectInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ProjectInfoPage member variables
////@end ProjectInfoPage member variables
};

/*!
 * AccountInfoPage class declaration
 */

class AccountInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS( AccountInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    AccountInfoPage( );

    AccountInfoPage( wxWizard* parent );

    /// Creation
    bool Create( wxWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin AccountInfoPage event handler declarations

////@end AccountInfoPage event handler declarations

////@begin AccountInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end AccountInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin AccountInfoPage member variables
    wxRadioButton* m_AccountCreate;
    wxRadioButton* m_AccountUseExisting;
////@end AccountInfoPage member variables
};

#endif
    // _WIZATTACHPROJECT_H_
