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

#include "stdwx.h"
#include "Events.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "sg_ProjectPanel.h"
#include "sg_ProjectWebSitesPopup.h"


IMPLEMENT_DYNAMIC_CLASS(CSimpleProjectWebSitesPopupButton, CTransparentButton)

BEGIN_EVENT_TABLE(CSimpleProjectWebSitesPopupButton, CTransparentButton)
    EVT_LEFT_DOWN(CSimpleProjectWebSitesPopupButton::OnProjectWebSitesMouseDown)
	EVT_MENU(WEBSITE_URL_MENU_ID,CSimpleProjectWebSitesPopupButton::OnMenuLinkClicked)
	EVT_MENU(WEBSITE_URL_MENU_ID_REMOVE_PROJECT,CSimpleProjectWebSitesPopupButton::OnMenuLinkClicked)
END_EVENT_TABLE()

CSimpleProjectWebSitesPopupButton::CSimpleProjectWebSitesPopupButton() {
}

CSimpleProjectWebSitesPopupButton::CSimpleProjectWebSitesPopupButton(wxWindow* parent, wxWindowID id,
        const wxString& label, const wxPoint& pos, const wxSize& size,
        long style, const wxValidator& validator, const wxString& name) :
        CTransparentButton(parent, id, label, pos, size, style, validator, name)
    {

    m_ProjectWebSitesPopUpMenu = new wxMenu();
    Connect(
        id,
        wxEVT_BUTTON,
        (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &CSimpleProjectWebSitesPopupButton::OnProjectWebSitesKeyboardNav
    );
}


CSimpleProjectWebSitesPopupButton::~CSimpleProjectWebSitesPopupButton() {
    delete m_ProjectWebSitesPopUpMenu;
}


void CSimpleProjectWebSitesPopupButton::AddMenuItems()
{
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    ProjectSelectionData*  selData = ((CSimpleProjectPanel*)GetParent())->GetProjectSelectionData();
    if (selData == NULL) return;
    char* ctrl_url = selData->project_url;
    PROJECT* project = pDoc->state.lookup_project(ctrl_url);
    if (!project) return;

	size_t urlCount = project->gui_urls.size();

	// Add the home page link
    wxMenuItem *urlItem = new wxMenuItem(m_ProjectWebSitesPopUpMenu, WEBSITE_URL_MENU_ID_HOMEPAGE, wxString("Home page", wxConvUTF8));
	Connect( WEBSITE_URL_MENU_ID_HOMEPAGE,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CSimpleProjectWebSitesPopupButton::OnMenuLinkClicked) );
	m_ProjectWebSitesPopUpMenu->Append(urlItem);


	// Add any GUI urls
	for(unsigned int i = 0; i < urlCount; i++){
        urlItem = new wxMenuItem(m_ProjectWebSitesPopUpMenu, WEBSITE_URL_MENU_ID + i, wxGetTranslation(wxString(project->gui_urls[i].name.c_str(), wxConvUTF8)));
	    Connect( WEBSITE_URL_MENU_ID + i,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CSimpleProjectWebSitesPopupButton::OnMenuLinkClicked) );

		m_ProjectWebSitesPopUpMenu->Append(urlItem);
	}
}


void CSimpleProjectWebSitesPopupButton::RebuildMenu() {
	for(int i=(int) m_ProjectWebSitesPopUpMenu->GetMenuItemCount()-1; i>=0; --i){
		wxMenuItem* item = m_ProjectWebSitesPopUpMenu->FindItemByPosition(i);
		m_ProjectWebSitesPopUpMenu->Delete(item);
	}
	AddMenuItems();
}


void CSimpleProjectWebSitesPopupButton::OnProjectWebSitesMouseDown(wxMouseEvent&) {
    ShowProjectWebSitesMenu(ScreenToClient(wxGetMousePosition()));
}


void CSimpleProjectWebSitesPopupButton::OnProjectWebSitesKeyboardNav(wxCommandEvent&) {
    ShowProjectWebSitesMenu(wxPoint(GetSize().GetWidth()/2, GetSize().GetHeight()/2));
}


void CSimpleProjectWebSitesPopupButton::ShowProjectWebSitesMenu(wxPoint pos) {
#ifdef __WXMAC__
    // Disable tooltips on Mac while menus are popped up because they cover menus
    wxToolTip::Enable(false);
#endif

	PopupMenu(m_ProjectWebSitesPopUpMenu, pos.x, pos.y);
}


void CSimpleProjectWebSitesPopupButton::OnMenuLinkClicked(wxCommandEvent& event) {
	 CMainDocument* pDoc = wxGetApp().GetDocument();
     wxASSERT(pDoc);
	 int menuIDevt =  event.GetId();

    ProjectSelectionData*  selData = ((CSimpleProjectPanel*)GetParent())->GetProjectSelectionData();
    if (selData == NULL) return;
    char* ctrl_url = selData->project_url;

     if (menuIDevt == WEBSITE_URL_MENU_ID_HOMEPAGE ) {
         wxLaunchDefaultBrowser(wxString(ctrl_url, wxConvUTF8));
     } else{
         int menuId = menuIDevt - WEBSITE_URL_MENU_ID;
         PROJECT* project = pDoc->state.lookup_project(ctrl_url);

         wxLaunchDefaultBrowser(wxString(project->gui_urls[menuId].url.c_str(),wxConvUTF8));
	 }
}
