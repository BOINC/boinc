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
#pragma implementation "DlgExitMessage.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "network.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "DlgExitMessage.h"

////@begin includes
////@end includes

////@begin XPM images
////@end XPM images

/*!
 * CDlgExitMessage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgExitMessage, wxDialog )

/*!
 * CDlgExitMessage event table definition
 */

BEGIN_EVENT_TABLE( CDlgExitMessage, wxDialog )

////@begin CDlgExitMessage event table entries
////@end CDlgExitMessage event table entries

END_EVENT_TABLE()

/*!
 * CDlgExitMessage constructors
 */

CDlgExitMessage::CDlgExitMessage( )
{
}

CDlgExitMessage::CDlgExitMessage( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgFileExit creator
 */

bool CDlgExitMessage::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
        wxASSERT(pSkinAdvanced);
        wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        strCaption.Printf(_("%s - Exit Confirmation"), pSkinAdvanced->GetApplicationName().c_str());
    }

////@begin CDlgExitMessage member initialisation
    m_DialogExitMessage = NULL;
    m_DialogShutdownCoreClient = NULL;
    m_DialogDisplay = NULL;
////@end CDlgExitMessage member initialisation

////@begin CDlgExitMessage creation
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, strCaption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgExitMessage creation
    return true;
}

/*!
 * Control creation for CDlgFileExit
 */

void CDlgExitMessage::CreateControls()
{    
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxString strExitMessage = wxEmptyString;
    wxString strAlwaysExitMessage = wxEmptyString;

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));
    
////@begin CDlgExitMessage content construction
    CDlgExitMessage* itemDialog1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(1, 2, 0, 0);
    itemDialog1->SetSizer(itemFlexGridSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(3, 1, 0, 0);
    itemBoxSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

#ifdef __WXMSW__
    strExitMessage.Printf(
        _("You have requested to exit the %s,\nwhich allows you to view and manage\nthe science applications running on your computer.\n\nIf you also want to stop running the science applications when you\nexit the Manager, then choose from the following options:"),
        pSkinAdvanced->GetApplicationName().c_str()
    );
#else
    strExitMessage.Printf(
        _("This will shut down %s and its tasks entirely until either the\n%s application or the %s screen saver is run again.\n\nIn most cases, it is better just to close the %s window\nrather than to exit the application; that will allow %s to run its\ntasks at the times you selected in your preferences."),
        pSkinAdvanced->GetApplicationShortName().c_str(),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str(),
        pSkinAdvanced->GetApplicationName().c_str(),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
#endif

    m_DialogExitMessage = new wxStaticText;
    m_DialogExitMessage->Create( itemDialog1, wxID_STATIC, strExitMessage, wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer4->Add(m_DialogExitMessage, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer4->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifdef __WXMSW__

    strAlwaysExitMessage.Printf(
        _("Stop running science applications when exiting the Manager")
	);

    m_DialogShutdownCoreClient = new wxCheckBox;
    m_DialogShutdownCoreClient->Create( itemDialog1, ID_CDLGEXITMESSAGE_SHUTDOWNCORECLIENT, strAlwaysExitMessage, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_DialogShutdownCoreClient->SetValue(false);
    itemFlexGridSizer4->Add(m_DialogShutdownCoreClient, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

#endif

    m_DialogDisplay = new wxCheckBox;
    m_DialogDisplay->Create( itemDialog1, ID_CDLGEXITMESSAGE_DISPLAY, _("Remember this decision and do not show this dialog."), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_DialogDisplay->SetValue(false);
    itemFlexGridSizer4->Add(m_DialogDisplay, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer2->Add(itemFlexGridSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton9 = new wxButton;
    itemButton9->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton9->SetDefault();
    itemFlexGridSizer8->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton;
    itemButton10->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CDlgExitMessage content construction
}

/*!
 * Should we show tooltips?
 */

bool CDlgExitMessage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDlgExitMessage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDlgExitMessage bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CDlgExitMessage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgExitMessage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDlgExitMessage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CDlgExitMessage icon retrieval
}
