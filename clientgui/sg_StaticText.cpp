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
#pragma implementation "sg_StaticText.h"
#endif

#include "stdwx.h"
#include "sg_StaticText.h" 

BEGIN_EVENT_TABLE(CStaticText, wxStaticText) 
        EVT_ERASE_BACKGROUND(CStaticText::OnEraseBackground)
        EVT_PAINT(CStaticText::OnPaint) 
END_EVENT_TABLE() 

CStaticText::CStaticText(wxWindow* parent,wxPoint coord,wxSize size) : wxStaticText(parent, wxID_ANY, wxT("Static Text"),coord, size, wxST_NO_AUTORESIZE) 
{ 
}
void CStaticText::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
        wxPaintDC dc(this); 
		//dc.SetBackgroundMode(wxSOLID);
		DrawVerticalGradient(dc, GetRect());
		//
		dc.SetFont(this->GetFont()); 
		dc.DrawText(this->GetLabel(), 0, 0); 
} 
void CStaticText::DrawVerticalGradient(wxDC &dc, const wxRect &rect )
{
    // gradient fill from colour 1 to colour 2 with left to right

    if(rect.height < 1 || rect.width < 1)
        return;

    int size = rect.width;

    dc.SetPen(*wxTRANSPARENT_PEN);

    // calculate gradient coefficients
    wxColour col2 = wxColour(51,102,102),
             col1 = wxColour(51,102,100);

    double rstep = double((col2.Red() -   col1.Red())) / double(size), rf = 0,
           gstep = double((col2.Green() - col1.Green())) / double(size), gf = 0,
           bstep = double((col2.Blue() -  col1.Blue())) / double(size), bf = 0;

    wxColour currCol;
    for(int y = rect.y; y < rect.y + size; y++)
    {
        currCol.Set(
            (unsigned char)(col1.Red() + rf),
            (unsigned char)(col1.Green() + gf),
            (unsigned char)(col1.Blue() + bf)
        );
        dc.SetBrush( wxBrush( currCol, wxSOLID ) );
        dc.DrawRectangle( rect.x, rect.y + (y - rect.y), rect.width, size );
        //currCol.Set(currCol.Red() + rstep, currCol.Green() + gstep, currCol.Blue() + bstep);
        rf += rstep; gf += gstep; bf += bstep;
    }
}
void CStaticText::OnEraseBackground(wxEraseEvent& event){
  event.Skip(true);
}
