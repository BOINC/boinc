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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgItemProperties.h"
#endif

#include "stdwx.h"
#include "DlgItemProperties.h"
#include "Events.h"

IMPLEMENT_DYNAMIC_CLASS(CDlgItemProperties, wxDialog)

BEGIN_EVENT_TABLE(CDlgItemProperties, wxDialog)
	//buttons
	EVT_BUTTON(wxID_OK,CDlgItemProperties::OnOK)
END_EVENT_TABLE()

/* Constructor */
CDlgItemProperties::CDlgItemProperties(wxWindow* parent) : CDlgItemPropertiesBase(parent,ID_ANYDIALOG) {
}

void CDlgItemProperties::OnOK(wxCommandEvent& ev) {
	ev.Skip();
}

CDlgItemProperties::~CDlgItemProperties() {
}

void CDlgItemProperties::renderInfos(PROJECT* project) {
	wxString html = wxEmptyString;	
	std::string name;
	project->get_name(name);
	html.append(wxT("<html><body><h3>"));
	html.append(_("project infos for "));
	html.append(wxString(name.c_str(),wxConvUTF8));
	html.append(wxT("</h3></body></html>"));
	this->m_html->SetPage(html);
}

void CDlgItemProperties::renderInfos(RESULT* result) {
}

