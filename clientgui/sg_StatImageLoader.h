#ifndef _STATIMAGELOADER_H_
#define _STATIMAGELOADER_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_StatImageLoader.cpp"
#endif

#include "sg_SkinClass.h"

class StatImageLoader : public wxWindow 
{ 
public: 
	    //members
        wxMenu *statPopUpMenu;
        //Skin Class
        SkinClass *appSkin;
		std::string m_prjUrl;
	    /// Constructors
		StatImageLoader(wxWindow* parent, std::string url); 
        void LoadImage(std::string project_icon, const wxImage& defaultImage); 
		void CreateMenu();
		void OnMenuLinkClicked(wxCommandEvent& event);
		void OnProjectDetach();
		void PopUpMenu(wxMouseEvent& event); 
        void OnPaint(wxPaintEvent& event); 
private: 
        //private memb 
        wxBitmap Bitmap; 
		std::string projectIcon;
		int numReloadTries;
		wxTimer* attemptToReloadTimer;
		void LoadStatIcon(const wxImage& image);
		void CheckForProjectIconDownloaded(wxTimerEvent& WXUNUSED(event));
        DECLARE_EVENT_TABLE() 
}; 

#endif 

