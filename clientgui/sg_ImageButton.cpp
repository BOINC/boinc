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
#pragma implementation "sg_ImageButton.h"
#endif

#include "stdwx.h"
#include "sg_ImageButton.h" 
#include "sg_ViewTabPage.h"

BEGIN_EVENT_TABLE(CImageButton, wxPanel) 
        EVT_PAINT(CImageButton::OnPaint) 
		EVT_LEFT_UP(CImageButton::OnLeftUp)
		EVT_ERASE_BACKGROUND(CImageButton::OnEraseBackground)
END_EVENT_TABLE() 

CImageButton::CImageButton(wxWindow* parent,wxBitmap bg, wxPoint coord,wxSize size,bool drawText = false) : wxPanel(parent, wxID_ANY, coord, size, wxNO_BORDER) 
{ 
	btnBG = bg;
	m_drawText = drawText;
}

void CImageButton::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
        wxPaintDC dc(this); 
		if(m_drawText)
		{
			dc.SetTextForeground(wxColour(*wxWHITE));
			dc.SetFont(wxFont(7,74,90,90,0,wxT("Arial")));
			wxString text = wxString(wxT("> Show graphics"));
			wxCoord width, height;
			dc.GetTextExtent(text, &width, &height);
			int x, y;
			GetSize(&x,&y);
			dc.DrawText(text, wxPoint(x-width-7,y-height-4));
		}
} 
void CImageButton::SetImage(wxBitmap bg) 
{ 
      btnBG = bg;  
} 
void CImageButton::OnEraseBackground(wxEraseEvent& event){

	event.Skip(false);
	wxDC *dc;
	dc=event.GetDC();
	dc->SetBackground(wxBrush(this->GetBackgroundColour(),wxSOLID));
	dc->Clear();
	if(btnBG.Ok()) 
    { 
		dc->DrawBitmap(btnBG, 0, 0); 
    } 
	
}
void CImageButton::OnLeftUp(wxMouseEvent& event)
{
	CViewTabPage* pView = wxDynamicCast(GetParent(), CViewTabPage);
    wxASSERT(pView);
	pView->OnImageButton();
	
}

void CImageButton::SetShowText(bool show) {
	if ( show != m_drawText )  {
		m_drawText = show;
		Refresh();
	}
}
 