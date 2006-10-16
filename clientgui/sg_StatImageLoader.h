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
		~StatImageLoader(); 
        void LoadImage(std::string project_icon, wxBitmap* defaultImage); 
		void OnMenuLinkClicked(wxCommandEvent& event);
		void OnProjectDetach();
		void PopUpMenu(wxMouseEvent& event); 
        void OnPaint(wxPaintEvent& event);
		void RebuildMenu();
		void UpdateInterface();
private: 
        //private memb 
        wxBitmap Bitmap; 
		std::string projectIcon;
		int numReloadTries;
		size_t urlCount;
		double project_files_downloaded_time;
		double project_last_rpc_time;
		void LoadStatIcon(wxBitmap& image);
		void ReloadProjectSpecificIcon();
		void BuildUserStatToolTip();
		void AddMenuItems();
        DECLARE_EVENT_TABLE() 
}; 

#endif 

