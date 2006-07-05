#ifndef _IMAGELOADER_H_
#define _IMAGELOADER_H_ 

class ImageLoader : public wxWindow 
{ 
public: 
	    /// Constructors
	    ImageLoader(wxWindow* parent); 
        void LoadImage(const wxImage& image); 
        void OnPaint(wxPaintEvent& event); 
private: 
        //static const int MaxWidth = 320; 
        //static const int MaxHeight = 240; 
        wxBitmap Bitmap; 
        DECLARE_EVENT_TABLE() 
}; 

#endif 

