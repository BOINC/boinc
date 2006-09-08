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

#ifndef _PROJECTSCOMPONENT_H_
#define _PROJECTSCOMPONENT_H_


#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_ProjectsComponent.cpp"
#endif

class SkinClass;
class StatImageLoader;
class CSimpleFrame;
class ImageLoader;
class CStaticLine;

class CProjectsComponent : public wxPanel {
    DECLARE_DYNAMIC_CLASS( CProjectsComponent )

public:
	
	int clientRunMode;
	//Skin Class
    SkinClass *appSkin;
	char defaultIcnPath[256];
	char urlDirectory[256];
	std::string dirProjectGraphic;
	std::string projectIconName;
	// projects vector
	std::vector<StatImageLoader*> m_statProjects; // vector of all project icons created for GUI
	wxImage *g_statIcn;
	//static content
	wxStaticText *stMyProj;
	CStaticLine *lnMyProjTop;
	CStaticLine *lnMyProjBtm;
	// proj icon
	wxString toolTipTxt;
	wxString userCredit;
	// default icon
	wxImage *g_statIcnDefault;
	// spacer
	wxImage *g_spacer;
	ImageLoader *i_spacer;
	// btns
	// arrows
    wxImage *g_arwLeft;
	wxImage *g_arwRight;
	wxImage *g_arwLeftClick;
	wxImage *g_arwRightClick;
	wxImage *g_addProj;
	wxImage *g_addProjClick;
	wxImage *g_messages;
	wxImage *g_messagesClick;
	wxImage *g_alertMessages;
	wxImage *g_alertMessagesClick;
	wxImage *g_pause;
	wxImage *g_pauseClick;
	wxImage *g_resume;
	wxImage *g_resumeClick;
	wxImage *g_pref;
	wxImage *g_prefClick;
	wxImage *g_advView;
	wxImage *g_advViewClick;
	wxBitmap btmpArwL; 
    wxBitmap btmpArwR; 
    wxBitmap btmpArwLC; 
    wxBitmap btmpArwRC; 
	wxBitmap btmpAddProj;
	wxBitmap btmpAddProjC;
	wxBitmap btmpMessages;
	wxBitmap btmpMessagesC;
	wxBitmap btmpAlertMessages;
	wxBitmap btmpAlertMessagesC;
	wxBitmap btmpPause;
	wxBitmap btmpPauseC;
	wxBitmap btmpResume;
	wxBitmap btmpResumeC;
	wxBitmap btmpPref;
	wxBitmap btmpPrefC;
	wxBitmap btmpAdvView;
	wxBitmap btmpAdvViewC;
    wxBitmapButton *btnArwLeft;
	wxBitmapButton *btnArwRight;
	wxBitmapButton *btnAddProj;
	wxBitmapButton *btnMessages;
	wxBitmapButton *btnAlertMessages;
	wxBitmapButton *btnPause;
	wxBitmapButton *btnResume;
	wxBitmapButton *btnPreferences;
	wxBitmapButton *btnAdvancedView;
    // bg
	wxBitmap *btmpComponentBg;
	wxBitmap fileImgBuf[1];
	
    CProjectsComponent();
    CProjectsComponent(CSimpleFrame* parent,wxPoint coord);
    ~CProjectsComponent();

	void LoadSkinImages();
    void CreateComponent();
	void RemoveProject(std::string prjUrl);
	void UpdateInterface();
	void ReskinInterface();
    void OnBtnClick(wxCommandEvent& event);
	void OnPaint(wxPaintEvent& event); 

	DECLARE_EVENT_TABLE()

protected:
	int m_maxNumOfIcons;
	int m_projCnt;
	int m_leftIndex;
	int m_rightIndex;

	void OnEraseBackground(wxEraseEvent& event);
    void DrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz);

private:
	wxTimer* checkForMessagesTimer;
	bool receivedErrorMessage;
	bool alertMessageDisplayed;
	static size_t lastMessageId;
	void CheckForErrorMessages(wxTimerEvent& WXUNUSED(event));
   	void MessagesViewed();
};

#endif

