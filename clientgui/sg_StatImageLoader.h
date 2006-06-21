#ifndef _STATIMAGELOADER_H_
#define _STATIMAGELOADER_H_ 

#include "wx/menu.h"
#include "sg_SkinClass.h"

class StatImageLoader : public wxWindow 
{ 
public: 
	    //members
        wxMenu *statPopUpMenu;
        //Skin Class
        SkinClass *appSkin;
	    /// Constructors
	    StatImageLoader(wxWindow* parent); 
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

