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
	PopupMenu(statPopUpMenu);
}

void StatImageLoader::RebuildMenu() {
	Freeze();
	delete statPopUpMenu;
	CreateMenu();
	Thaw();
}

void StatImageLoader::CreateMenu() 
{ 
	CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);

	appSkin = SkinClass::Instance();

	PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
	urlCount = project->gui_urls.size();
	
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
	#ifdef __WXMSW__
		menuList[statPopUpMenu->GetMenuItemCount()-1]->SetBackgroundColour(wxColour("RED"));
	#endif

	wxMenuItem *urlItem = new wxMenuItem(statPopUpMenu, WEBSITE_URL_MENU_ID_REMOVE_PROJECT,wxT("Remove Project"));
	#ifdef __WXMSW__
		urlItem->SetBackgroundColour(appSkin->GetAppBgCol());
	#endif
	Connect( WEBSITE_URL_MENU_ID_REMOVE_PROJECT,  wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(StatImageLoader::OnMenuLinkClicked) );
	statPopUpMenu->Append(urlItem);

}
void StatImageLoader::OnMenuLinkClicked(wxCommandEvent& event) 
{ 
	 CMainDocument* pDoc = wxGetApp().GetDocument();
     wxASSERT(pDoc);
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

void StatImageLoader::LoadStatIcon(wxBitmap& image) {
	int width = image.GetWidth(); 
	int height = image.GetHeight(); 
	Bitmap = image; 
	SetSize(width, height); 
}


void StatImageLoader::LoadImage(std::string project_icon, wxBitmap* defaultImage) 
{ 
	char defaultIcnPath[256];
	bool defaultUsed = true;
	if(boinc_resolve_filename(project_icon.c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
		wxBitmap* btmpStatIcn = new wxBitmap();
		if ( btmpStatIcn->LoadFile(defaultIcnPath, wxBITMAP_TYPE_ANY) ) {
			LoadStatIcon(*btmpStatIcn);
			defaultUsed = false;
		} else {
			LoadStatIcon(*defaultImage);
		}
	}else{
		LoadStatIcon(*defaultImage);
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
	// Check project icon downloaded
	if(boinc_resolve_filename(projectIcon.c_str(), defaultIcnPath, sizeof(defaultIcnPath)) == 0){
		wxBitmap* btmpStatIcn = new wxBitmap();
		if ( btmpStatIcn->LoadFile(defaultIcnPath, wxBITMAP_TYPE_ANY) ) {
			LoadStatIcon(*btmpStatIcn);
			success = true;
			Refresh();
			Update();
		}
	}

	CMainDocument* pDoc = wxGetApp().GetDocument();
	PROJECT* project = pDoc->state.lookup_project(m_prjUrl);
	if ( urlCount != project->gui_urls.size() ) {
		RebuildMenu();
	}

	// Try numReloadTries times (set in constructor) or until success
	if ( success || numReloadTries-- <=0 ) {
		attemptToReloadTimer->Stop();
	}
}

void StatImageLoader::UpdateInterface() {
	// TODO - need to update this so that the gui urls are checked here
}

void StatImageLoader::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
        wxPaintDC dc(this); 
        if(Bitmap.Ok()) 
        { 
                dc.DrawBitmap(Bitmap, 0, 0); 
        } 
} 
