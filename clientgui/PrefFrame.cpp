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
#pragma implementation "PrefFrame.h"
#endif

#include "stdwx.h"
#include "wx/treectrl.h"
#include "PrefFrame.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "SkinManager.h"
#include "hyperlink.h"
#include "Events.h"
#include "PrefLocationManager.h"

IMPLEMENT_DYNAMIC_CLASS(CPrefFrame, wxDialog)

BEGIN_EVENT_TABLE(CPrefFrame, wxDialog)
	EVT_BUTTON(wxID_OK, CPrefFrame::OnOK)
	EVT_BUTTON(wxID_HELP, CPrefFrame::OnHelp)
    EVT_BUTTON(ID_LOCATIONMANAGER, CPrefFrame::OnLocationManager)
END_EVENT_TABLE()

CPrefFrame::CPrefFrame(wxWindow* parent) : wxDialog(parent, ID_ANYDIALOG, _("Preferences"),
    wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxBoxSizer* vShape = new wxBoxSizer(wxVERTICAL);

    // Venue controls at the top.
    wxBoxSizer* headerRow = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* locationText = new wxStaticText(this, wxID_ANY, _("Location:"));

    wxChoice* locationChoice = new wxChoice(this, wxID_ANY);
    locationChoice->AppendString(_("Default"));
    locationChoice->SetSelection(0);

    wxButton* locationManager = new wxButton(this, ID_LOCATIONMANAGER, _("Manage locations..."));

    headerRow->Add(locationText, 0, wxALL | wxCENTER, PREF_DLG_MARGIN);
    headerRow->Add(locationChoice, 1, wxALL | wxCENTER, PREF_DLG_MARGIN);
    headerRow->Add(locationManager, 0, wxALL | wxCENTER, PREF_DLG_MARGIN);

    // Treeview preferences
    m_treeBook = new CPrefTreeBook(this);

    // Dialog buttons - right aligned.
    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);

    m_buttonOkay = new wxButton(this, wxID_OK, _("Okay"));
    m_buttonCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
    m_buttonHelp = new wxButton(this, wxID_HELP, _("Help"));

    buttonRow->AddStretchSpacer();
    buttonRow->Add(m_buttonOkay, 0, wxALL, PREF_DLG_MARGIN);
    buttonRow->Add(m_buttonCancel, 0, wxALL, PREF_DLG_MARGIN);
    buttonRow->Add(m_buttonHelp, 0, wxALL, PREF_DLG_MARGIN);
    
    vShape->Add(headerRow, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, PREF_DLG_MARGIN);
    vShape->Add(m_treeBook, 1, wxLEFT | wxRIGHT | wxEXPAND, PREF_DLG_MARGIN);
    vShape->Add(buttonRow, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, PREF_DLG_MARGIN);

    SetSizerAndFit(vShape);

	RestoreState();
}


CPrefFrame::~CPrefFrame() {
	SaveState();
}


bool CPrefFrame::SaveState() {
    wxString        strBaseConfigLocation = wxString(wxT("/PrefFrame/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);

	if (!pConfig) return false;

	pConfig->SetPath(strBaseConfigLocation);
	pConfig->Write(wxT("Width"), GetSize().GetWidth());
	pConfig->Write(wxT("Height"), GetSize().GetHeight());

	return true;
}


bool CPrefFrame::RestoreState() {
    wxString        strBaseConfigLocation = wxString(wxT("/PrefFrame/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);
	int				w, h;

    if (!pConfig) return false;

	pConfig->SetPath(strBaseConfigLocation);
	pConfig->Read(wxT("Width"), &w, -1);
	pConfig->Read(wxT("Height"), &h, -1);
	this->SetSize(w, h);	

	return true;
}

// OK button handler.
void CPrefFrame::OnOK(wxCommandEvent& ev) {

    if (Validate() && TransferDataFromWindow())
    {
        m_treeBook->SavePreferences();

        if (IsModal()) {
            EndModal(wxID_OK);
        } else {
            SetReturnCode(wxID_OK);
            this->Show(false);
        }
    }
	ev.Skip();
}


// Help button handler.
void CPrefFrame::OnHelp(wxCommandEvent& ev) {
	//wxString url = wxGetApp().GetSkinManager()->GetAdvanced()->GetCompanyWebsite();
	//url += wxT("/prefs.php");//this seems not the right url, but which instead ?
	//wxHyperLink::ExecuteLink(url);
	ev.Skip();
}


void CPrefFrame::OnLocationManager(wxCommandEvent& ev) {
	CPrefLocationManager dlg(this);
	dlg.ShowModal();
}