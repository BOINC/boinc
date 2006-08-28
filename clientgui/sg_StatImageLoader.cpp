#include "stdwx.h"
#include <vector>
#include "BOINCGUIApp.h"
#include "sg_StatImageLoader.h" 
#include "BOINCBaseFrame.h"
#include "sg_ProjectsComponent.h" 
#include "app_ipc.h"

#define ID_CHECKFORPROJECTICONDOWNLOADED  13001

enum{
	WEBSITE_URL_MENU_ID = 34500,
	WEBSITE_URL_MENU_ID_REMOVE_PROJECT = 34550,
};


BEGIN_EVENT_TABLE(StatImageLoader, wxWindow) 
        EVT_PAINT(StatImageLoader::OnPaint) 
		EVT_LEFT_DOWN(StatImageLoader::PopUpMenu)
		EVT_MENU(WEBSITE_URL_MENU_ID,StatImageLoader::OnMenuLinkClicked)
		EVT_MENU(WEBSITE_URL_MENU_ID_REMOVE_PROJECT,StatImageLoader::OnMenuLinkClicked)
		EVT_TIMER(ID_CHECKFORPROJECTICONDOWNLOADED, StatImageLoader::CheckForProjectIconDownloaded)
END_EVENT_TABLE() 

StatImageLoader::StatImageLoader(wxWindow* parent, std::string url) : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(40,40), wxNO_BORDER) 
{
    m_prjUrl = url;
    CreateMenu();
}

void StatImageLoader::PopUpMenu(wxMouseEvent& WXUNUSED(event)) 
{ 
	// pop up menu
	bool menuPoped = PopupMenu(statPopUpMenu);
}

void StatImageLoader::CreateMenu() 
{ 
	CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);

	appSkin = SkinClass::Instance();

	PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
	size_t urlCount = project->gui_urls.size();
	
	// create pop up menu
	statPopUpMenu = new wxMenu(wxSIMPLE_BORDER);

	for(unsigned int i = 0; i < urlCount; i++){
		wxMenuItem *urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID + i,wxString(project->gui_urls[i].name.c_str(), wxConvUTF8));
		#ifdef __WXMSW__
			urlItem->SetBackgroundColour(appSkin->GetAppBgCol());
		#endif
	    Connect( WEBSITE_URL_MENU_ID + i,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
 
		statPopUpMenu->Append(urlItem);
	}
    
	statPopUpMenu->AppendSeparator();
	wxMenuItemList menuList = statPopUpMenu->GetMenuItems();
	//wxMenuItem* separ = statPopUpMenu->FindItemByPosition(i);
	#ifdef __WXMSW__
		menuList[statPopUpMenu->GetMenuItemCount()-1]->SetBackgroundColour(wxColour("RED"));
	#endif

	wxMenuItem *urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID_REMOVE_PROJECT,wxT("Remove Project"));
	#ifdef __WXMSW__
		urlItem->SetBackgroundColour(appSkin->GetAppBgCol());
	#endif
	Connect( WEBSITE_URL_MENU_ID_REMOVE_PROJECT,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
	statPopUpMenu->Append(urlItem);
	//
	/*
	wxBitmap  *btmTellFriend = new wxBitmap();
	btmTellFriend->LoadFile("skins/default/graphic/micnTellFriend.png",wxBITMAP_TYPE_PNG);
    itmTellFriend->SetBitmap(*btmTellFriend);
	*/
}
void StatImageLoader::OnMenuLinkClicked(wxCommandEvent& event) 
{ 
	 CMainDocument* pDoc = wxGetApp().GetDocument();
     wxASSERT(pDoc);
     wxObject *m_wxBtnObj = event.GetEventObject();
	 int menuIDevt =  event.GetId();

	 if(menuIDevt == WEBSITE_URL_MENU_ID_REMOVE_PROJECT){
		 //call detach project function	
         OnProjectDetach();
	 }else{
         int menuId = menuIDevt - WEBSITE_URL_MENU_ID;
	     PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
		 project->gui_urls[menuId].name.c_str();
     
	     CBOINCBaseFrame* pFrame = wxDynamicCast(m_parent->GetParent(),CBOINCBaseFrame);
         wxASSERT(pFrame);
         wxASSERT(wxDynamicCast(pFrame, CBOINCBaseFrame));
	     pFrame->ExecuteBrowserLink(project->gui_urls[menuId].url.c_str());
	 }
  
} 
void StatImageLoader::OnProjectDetach() {
    wxInt32  iAnswer        = 0; 
    std::string strProjectName;
    wxString strMessage     = wxEmptyString;
    CMainDocument* pDoc     = wxGetApp().GetDocument();

    CProjectsComponent* pComp      = wxDynamicCast(GetParent(), CProjectsComponent);

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pComp);

    if (!pDoc->IsUserAuthorized())
        return;

	int indexOfProj = -1;
	int prjCount = pDoc->GetProjectCount();
	for(int m = 0; m < prjCount; m++){
		PROJECT* project = pDoc->project(m);
		project->get_name(strProjectName);
		if(project->master_url == m_prjUrl){
			indexOfProj = m;
			break;
		}
	}
    // PROJECT* project = pDoc->project(m_ProjIconIndex);
    //project->get_name(strProjectName);

    strMessage.Printf(
        _("Are you sure you want to detach from project '%s'?"), 
        strProjectName.c_str()
    );

    iAnswer = ::wxMessageBox(
        strMessage,
        _("Detach from Project"),
        wxYES_NO | wxICON_QUESTION,
        this
    );

    if (wxYES == iAnswer) {
        pDoc->ProjectDetach(indexOfProj);
		pComp->RemoveProject(m_prjUrl);
    }
}

void StatImageLoader::LoadStatIcon(const wxImage& image) {
	Bitmap = wxBitmap();//delete existing bitmap since we are loading new one
	int width = image.GetWidth(); 
	int height = image.GetHeight(); 
	Bitmap = wxBitmap(image); 
	SetSize(width, height); 
}


void StatImageLoader::LoadImage(std::string project_icon, const wxImage& defaultImage) 
{ 
	char defaultIcnPath[256];
	bool defaultUsed = true;
	if(boinc_resolve_filename(project_icon.c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
		wxImage* g_statIcn = new wxImage();
		if ( g_statIcn->LoadFile(defaultIcnPath, wxBITMAP_TYPE_PNG) ) {
			LoadStatIcon(g_statIcn);
			defaultUsed = false;
		} else {
			// TODO - need to set a timer to switch this to load the real graphics once they are downloaded
			LoadStatIcon(defaultImage);
		}
	}else{
		LoadStatIcon(defaultImage);
	}

	if ( defaultUsed ) {
		projectIcon = project_icon;
		numReloadTries = 80; // check for 10 minutes
		attemptToReloadTimer = new wxTimer(this, ID_CHECKFORPROJECTICONDOWNLOADED);
		attemptToReloadTimer->Start(7500);
	}
} 

void StatImageLoader::CheckForProjectIconDownloaded(wxTimerEvent& WXUNUSED(event)) {
	char defaultIcnPath[256];
	bool success = false;
	if(boinc_resolve_filename(projectIcon.c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
		wxImage* g_statIcn = new wxImage();
		if ( g_statIcn->LoadFile(defaultIcnPath, wxBITMAP_TYPE_PNG) ) {
			LoadStatIcon(g_statIcn);
			success = true;
			Refresh();
			Update();
		}
	}
	// Try numReloadTries times (set in constructor) or until success
	if ( success || numReloadTries-- <=0 ) {
		attemptToReloadTimer->Stop();
	}
}

void StatImageLoader::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
        wxPaintDC dc(this); 
        if(Bitmap.Ok()) 
        { 
                dc.DrawBitmap(Bitmap, 0, 0); 
        } 
} 
