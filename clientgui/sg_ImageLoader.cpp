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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "sg_ImageLoader.h"
#endif

#include "stdwx.h"
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
