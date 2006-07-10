#include "stdwx.h"
#include <vector>
#include "BOINCGUIApp.h"
#include "sg_StatImageLoader.h" 
#include "BOINCBaseFrame.h"

enum{
	WEBSITE_URL_MENU_ID = 34500,
};


BEGIN_EVENT_TABLE(StatImageLoader, wxWindow) 
        EVT_PAINT(StatImageLoader::OnPaint) 
		EVT_LEFT_DOWN(StatImageLoader::PopUpMenu)
		EVT_MENU(WEBSITE_URL_MENU_ID,StatImageLoader::OnMenuLinkClicked)
END_EVENT_TABLE() 

StatImageLoader::StatImageLoader(wxWindow* parent, std::string url) : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER) 
{
	m_parent = parent;
    appSkin = SkinClass::Instance();
    prjUrl = url;
    CreateMenu();
}

void StatImageLoader::PopUpMenu(wxMouseEvent& event) 
{ 
	// pop up menu
	bool menuPoped = PopupMenu(statPopUpMenu);
}

void StatImageLoader::CreateMenu() 
{ 
	CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);

	PROJECT* project = pDoc->state.lookup_project(prjUrl);
	int urlCount = project->gui_urls.size();
	
	// create pop up menu
	statPopUpMenu = new wxMenu(wxSIMPLE_BORDER);

	for(int i = 0; i < urlCount; i++){
		wxMenuItem *urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID + i,wxString(project->gui_urls[i].name.c_str(), wxConvUTF8));
		#ifdef __WXMSW__
			urlItem->SetBackgroundColour(appSkin->GetAppBgCol());
		#endif
	    Connect( WEBSITE_URL_MENU_ID + i,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
 
		statPopUpMenu->Append(urlItem);
	}
	//
	/*
	wxBitmap  *btmTellFriend = new wxBitmap();
	btmTellFriend->LoadFile("skins/default/graphic/micnTellFriend.png",wxBITMAP_TYPE_PNG);
    itmTellFriend->SetBitmap(*btmTellFriend);
	*/
}
void StatImageLoader::OnMenuLinkClicked(wxCommandEvent& event) 
{ 
     wxObject *m_wxBtnObj = event.GetEventObject();
	 int menuIDevt =  event.GetId();
	 int menuId = menuIDevt - WEBSITE_URL_MENU_ID;
     
	 CMainDocument* pDoc = wxGetApp().GetDocument();
     wxASSERT(pDoc);

	 PROJECT* project = pDoc->state.lookup_project(prjUrl);
	 project->gui_urls[menuId].name.c_str();
     
	 CBOINCBaseFrame* pFrame = wxDynamicCast(m_parent->GetParent(),CBOINCBaseFrame);
     wxASSERT(pFrame);
     wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
	 pFrame->ExecuteBrowserLink(project->gui_urls[menuId].url.c_str());
	 int re = 4;
  
} 

void StatImageLoader::LoadImage(const wxImage& image) 
{ 
	Bitmap = wxBitmap();//delete existing bitmap since we are loading new one
	int width = image.GetWidth(); 
	int height = image.GetHeight(); 
	Bitmap = wxBitmap(image); 
	SetSize(width, height); 
} 

void StatImageLoader::OnPaint(wxPaintEvent& event) 
{ 
        wxPaintDC dc(this); 
        if(Bitmap.Ok()) 
        { 
                dc.DrawBitmap(Bitmap, 0, 0); 
        } 
} 
