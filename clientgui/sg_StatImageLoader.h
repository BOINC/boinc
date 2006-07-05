#ifndef _STATIMAGELOADER_H_
#define _STATIMAGELOADER_H_ 


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
		void PopUpMenu(wxMouseEvent& event); 
        void OnPaint(wxPaintEvent& event); 
private: 
        //private memb 
	    wxWindow *m_parent;
        wxBitmap Bitmap; 
        DECLARE_EVENT_TABLE() 
}; 

#endif 

