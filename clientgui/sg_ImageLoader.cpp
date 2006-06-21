#include <wx/wx.h> 
#include <wx/image.h> 

#include "sg_ImageLoader.h" 

BEGIN_EVENT_TABLE(ImageLoader, wxWindow) 
        EVT_PAINT(ImageLoader::OnPaint) 
END_EVENT_TABLE() 

ImageLoader::ImageLoader(wxWindow* parent) : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER) 
{ 
}
void ImageLoader::LoadImage(const wxImage& image) 
{ 
		Bitmap = wxBitmap();//delete existing bitmap since we are loading newone

        int width = image.GetWidth(); 
        int height = image.GetHeight(); 
        //double X_Ratio = (double) MaxWidth / (double) width; 
        //double Y_Ratio = (double) MaxHeight / (double) height; 
        //double Ratio = X_Ratio < Y_Ratio ? X_Ratio : Y_Ratio; 
        //wxImage Image = image.Scale((int)(Ratio * width), (int)(Ratio * height)); 
        Bitmap = wxBitmap(image); 
        //width = imageGetWidth(); 
        //height = image.GetHeight(); 
        SetSize(width, height); 
} 

void ImageLoader::OnPaint(wxPaintEvent& event) 
{ 
        wxPaintDC dc(this); 
        if(Bitmap.Ok()) 
        { 
                dc.DrawBitmap(Bitmap, 0, 0); 
        } 
} 
