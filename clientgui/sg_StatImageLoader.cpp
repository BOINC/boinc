#include "stdwx.h"

#include "BOINCGUIApp.h"
#include "sg_StatImageLoader.h" 


BEGIN_EVENT_TABLE(StatImageLoader, wxWindow) 
        EVT_PAINT(StatImageLoader::OnPaint) 
		EVT_LEFT_DOWN(StatImageLoader::PopUpMenu)
END_EVENT_TABLE() 

StatImageLoader::StatImageLoader(wxWindow* parent) : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER) 
{ 
   appSkin = SkinClass::Instance();
   CreateMenu();
}

void StatImageLoader::PopUpMenu(wxMouseEvent& event) 
{ 
	// pop up menu
	bool menuPoped = PopupMenu(statPopUpMenu);
}

void StatImageLoader::CreateMenu() 
{ 
	CMainDocument* pDoc      = wxGetApp().GetDocument();
 
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    // i = project id
    // j = vector of menu items
    //pDoc->state.projects[0]->gui_urls[0];

	// create pop up menu
	statPopUpMenu = new wxMenu(wxSIMPLE_BORDER);
	//
	wxMenuItem *itmTellFriend = new wxMenuItem(statPopUpMenu, -1,_T("Tell a Friend"));
#ifdef __WXMSW__
	itmTellFriend->SetBackgroundColour(appSkin->GetAppBgCol());
#endif
	wxBitmap  *btmTellFriend = new wxBitmap();
	btmTellFriend->LoadFile("skins/default/graphic/micnTellFriend.png",wxBITMAP_TYPE_PNG);
    itmTellFriend->SetBitmap(*btmTellFriend);
	//
	wxMenuItem *itmHome = new wxMenuItem(statPopUpMenu, -1,_T("Home"));
#ifdef __WXMSW__
	itmHome->SetBackgroundColour(appSkin->GetAppBgCol());
#endif
	//
	wxMenuItem *itmAbout = new wxMenuItem(statPopUpMenu, -1,_T("About Us"));
#ifdef __WXMSW__
	itmAbout->SetBackgroundColour(appSkin->GetAppBgCol());
#endif
	//
	wxMenuItem *itmResearch = new wxMenuItem(statPopUpMenu, -1,_T("Research"));
#ifdef __WXMSW__
	itmResearch->SetBackgroundColour(appSkin->GetAppBgCol());
#endif
	//
	wxMenuItem *itmForums = new wxMenuItem(statPopUpMenu, -1,_T("Forums"));
#ifdef __WXMSW__
	itmForums->SetBackgroundColour(appSkin->GetAppBgCol());
#endif
	//
	wxMenuItem *itmStatistics = new wxMenuItem(statPopUpMenu, -1,_T("Statistics"));
#ifdef __WXMSW__
	itmStatistics->SetBackgroundColour(appSkin->GetAppBgCol());
#endif
	//
	wxMenuItem *itmMyGrid = new wxMenuItem(statPopUpMenu, -1,_T("My Grid"));
#ifdef __WXMSW__
	itmMyGrid->SetBackgroundColour(appSkin->GetAppBgCol());
#endif
	wxBitmap  *btmMyGrid = new wxBitmap();
	btmMyGrid->LoadFile("skins/default/graphic/micnMyGrid.png",wxBITMAP_TYPE_PNG);
	itmMyGrid->SetBitmap(*btmMyGrid);
	//
	statPopUpMenu->Append(itmTellFriend);
	statPopUpMenu->Append(itmHome);
	statPopUpMenu->Append(itmAbout);
	statPopUpMenu->Append(itmResearch);
	statPopUpMenu->Append(itmForums);
	statPopUpMenu->Append(itmStatistics);
	statPopUpMenu->Append(itmMyGrid);
}
void StatImageLoader::LoadImage(const wxImage& image) 
{ 
	Bitmap = wxBitmap();//delete existing bitmap since we are loading newone
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
