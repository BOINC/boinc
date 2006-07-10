#ifndef _STATIMAGELOADER_H_
#define _STATIMAGELOADER_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_StatImageLoader.cpp"
#endif


#include "BOINCBaseView.h"
#include "sg_SkinClass.h"

class StatImageLoader : public wxWindow 
{ 
public: 
	    //members
        wxMenu *statPopUpMenu;
        //Skin Class
        SkinClass *appSkin;
		std::string prjUrl;
	    /// Constructors
		StatImageLoader(wxWindow* parent, std::string url); 
        void LoadImage(const wxImage& image); 
		void CreateMenu();
		void OnMenuLinkClicked(wxCommandEvent& event);
		void PopUpMenu(wxMouseEvent& event); 
        void OnPaint(wxPaintEvent& event); 
private: 
        //private memb 
	    wxWindow *m_parent;
        wxBitmap Bitmap; 
        DECLARE_EVENT_TABLE() 
}; 

#endif 

