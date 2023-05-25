// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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
#pragma implementation "DlgSelectComputer.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "SkinManager.h"

////@begin includes
////@end includes

#include "DlgSelectComputer.h"

////@begin XPM images
////@end XPM images

/*!
 * CDlgSelectComputer type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDlgSelectComputer, wxDialog )

/*!
 * CDlgSelectComputer event table definition
 */

BEGIN_EVENT_TABLE( CDlgSelectComputer, wxDialog )

////@begin CDlgSelectComputer event table entries
    EVT_TEXT( ID_SELECTCOMPUTERNAME, CDlgSelectComputer::OnComputerNameUpdated )
    EVT_COMBOBOX( ID_SELECTCOMPUTERNAME, CDlgSelectComputer::OnComputerNameUpdated )

////@end CDlgSelectComputer event table entries

END_EVENT_TABLE()

/*!
 * CDlgSelectComputer constructors
 */

CDlgSelectComputer::CDlgSelectComputer( )
{
}

CDlgSelectComputer::CDlgSelectComputer( wxWindow* parent, bool required, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, required, id, caption, pos, size, style);
}

/*!
 * CDlgSelectComputer creator
 */

bool CDlgSelectComputer::Create( wxWindow* parent, bool required, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDlgSelectComputer member initialisation
    m_ComputerNameCtrl = NULL;
    m_ComputerPasswordCtrl = NULL;
////@end CDlgSelectComputer member initialisation

    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
        wxASSERT(pSkinAdvanced);
        wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        strCaption.Printf(_("%s - Select Computer"), pSkinAdvanced->GetApplicationName().c_str());
    }

////@begin CDlgSelectComputer creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, strCaption, pos, size, style );

    CreateControls(required);
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end CDlgSelectComputer creation
    return TRUE;
}

/*!
 * Control creation for CDlgSelectComputer
 */

void CDlgSelectComputer::CreateControls(bool required)
{
////@begin CDlgSelectComputer content construction
    CDlgSelectComputer* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    if (required) {
        CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
        wxASSERT(pSkinAdvanced);
        wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        wxString strInfo = wxEmptyString;
        strInfo.Printf(
            _("Another instance of %s is already running \non this computer.  Please select a client to monitor."),
            pSkinAdvanced->GetApplicationName().c_str()
        );
        wxStaticText* itemStaticText1 = new wxStaticText;
        itemStaticText1->Create( itemDialog1, wxID_STATIC, strInfo, wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer2->Add(itemStaticText1, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText;
    itemStaticText6->Create( itemDialog1, wxID_STATIC, _("Host name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxString* m_ComputerNameCtrlStrings = NULL;
    m_ComputerNameCtrl = new wxComboBox;
    m_ComputerNameCtrl->Create( itemDialog1, ID_SELECTCOMPUTERNAME, _T(""), wxDefaultPosition, wxSize(250, -1), 0, m_ComputerNameCtrlStrings, wxCB_DROPDOWN );
    itemFlexGridSizer5->Add(m_ComputerNameCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText;
    itemStaticText8->Create( itemDialog1, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ComputerPasswordCtrl = new wxTextCtrl;
    m_ComputerPasswordCtrl->Create( itemDialog1, ID_SELECTCOMPUTERPASSWORD, _T(""), wxDefaultPosition, wxSize(250, -1), wxTE_PASSWORD );
    itemFlexGridSizer5->Add(m_ComputerPasswordCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton11 = new wxButton;
    itemButton11->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton11->SetDefault();
    itemBoxSizer10->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton;
    itemButton12->Create( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Set validators
    // Under wxCocoa 3.0, wxGenericValidator doesn't work right here
    m_ComputerNameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strComputerName) );
    m_ComputerPasswordCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_strComputerPassword) );

#ifdef __WXMAC__
    // Set keyboard shortcuts - why is this necessary?
    m_Shortcuts[0].Set(wxACCEL_CTRL, 'x', wxID_CUT);
    m_Shortcuts[1].Set(wxACCEL_CTRL, 'c', wxID_COPY);
    m_Shortcuts[2].Set(wxACCEL_CTRL, 'v', wxID_PASTE);
    m_pAccelTable = new wxAcceleratorTable(3, m_Shortcuts);
    m_ComputerPasswordCtrl->SetAcceleratorTable(*m_pAccelTable);

    // This does not work for m_ComputerNameCtrl under wxCocoa 3.0.0
    // because of a known bug: http://trac.wxwidgets.org/ticket/14953
    m_ComputerNameCtrl->SetAcceleratorTable(*m_pAccelTable);
 #endif


////@end CDlgSelectComputer content construction
}

/*!
 * Should we show tooltips?
 */

bool CDlgSelectComputer::ShowToolTips(){
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDlgSelectComputer::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CDlgSelectComputer bitmap retrieval
    return wxNullBitmap;
////@end CDlgSelectComputer bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDlgSelectComputer::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CDlgSelectComputer icon retrieval
    return wxNullIcon;
////@end CDlgSelectComputer icon retrieval
}

/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SELECTCOMPUTERNAME
 */

void CDlgSelectComputer::OnComputerNameUpdated( wxCommandEvent& WXUNUSED(event) )
{
    wxString       strPassword = wxEmptyString;
    CMainDocument* pDoc        = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    wxString name = m_ComputerNameCtrl->GetValue();
    if (pDoc->IsComputerNameLocal(name)) {
        if (pDoc->m_pNetworkConnection->GetLocalPassword(strPassword) == 0) {
            m_ComputerPasswordCtrl->SetValue(strPassword);
        }
    }
}

