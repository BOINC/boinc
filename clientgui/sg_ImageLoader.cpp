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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "sg_ImageLoader.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "sg_ImageLoader.h" 

BEGIN_EVENT_TABLE(ImageLoader, wxWindow) 
        EVT_PAINT(ImageLoader::OnPaint) 
END_EVENT_TABLE() 

ImageLoader::ImageLoader(wxWindow* parent, bool center) :
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER) 
{ 
	centerOnParent = center;
}


void ImageLoader::LoadImage(wxBitmap image) 
{ 
        int width = image.GetWidth(); 
        int height = image.GetHeight(); 
        Bitmap = image; 
        SetSize(width, height); 
		if ( centerOnParent ) {
			CentreOnParent();
		}
} 


void ImageLoader::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
        wxPaintDC dc(this); 
        if(Bitmap.Ok()) 
        { 
                dc.DrawBitmap(Bitmap, 0, 0); 
        } 
} 
