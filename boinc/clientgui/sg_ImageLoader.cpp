// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

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
