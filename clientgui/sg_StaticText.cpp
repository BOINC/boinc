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


IMPLEMENT_DYNAMIC_CLASS (CTransparentStaticText, wxStaticText)

BEGIN_EVENT_TABLE(CTransparentStaticText, wxStaticText)
    EVT_PAINT(CTransparentStaticText::OnPaint)
END_EVENT_TABLE()


CTransparentStaticText::CTransparentStaticText() {}

CTransparentStaticText::CTransparentStaticText(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    Create(parent, id, label, pos, size, style, name);
}


bool CTransparentStaticText::Create(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) { 
    bool bRetVal = wxStaticText::Create(parent, id, label, pos, size, style, name);

    SetBackgroundColour(wxColour(255, 0, 255));
    SetForegroundColour(*wxBLACK);
    SetWindowStyle(GetWindowStyle()|wxTRANSPARENT_WINDOW);

    return bRetVal;
}


void CTransparentStaticText::OnPaint(wxPaintEvent& /*event*/) {
    wxPaintDC dc(this);
    dc.SetFont(GetFont());
    dc.DrawText(GetLabel(), 0, 0);
}
