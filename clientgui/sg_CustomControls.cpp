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
#pragma implementation "sg_CustomControls.h"
#endif

#include "stdwx.h"
#include "sg_CustomControls.h" 


IMPLEMENT_DYNAMIC_CLASS (CTransparentStaticLine, wxPanel)

BEGIN_EVENT_TABLE(CTransparentStaticLine, wxPanel)
    EVT_PAINT(CTransparentStaticLine::OnPaint)
END_EVENT_TABLE()


CTransparentStaticLine::CTransparentStaticLine() {}

CTransparentStaticLine::CTransparentStaticLine(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    Create(parent, id, pos, size, style, name);
}


bool CTransparentStaticLine::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) { 
    bool bRetVal = wxPanel::Create(parent, id, pos, size, style|wxTRANSPARENT_WINDOW, name);

    SetBackgroundColour(parent->GetBackgroundColour());
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetForegroundColour(parent->GetForegroundColour());

    return bRetVal;
}


void CTransparentStaticLine::OnPaint(wxPaintEvent& /*event*/) {
    wxPaintDC dc(this); 
    wxPen pen = wxPen(GetLineColor(), 1);
    dc.SetPen(pen);
    dc.DrawLine(0, 0, GetSize().GetWidth(), 0); 
}


IMPLEMENT_DYNAMIC_CLASS (CTransparentStaticText, wxStaticText)

BEGIN_EVENT_TABLE(CTransparentStaticText, wxStaticText)
#ifdef __WXMAC__
    EVT_ERASE_BACKGROUND(CTransparentStaticText::OnEraseBackground)
#endif
    EVT_PAINT(CTransparentStaticText::OnPaint)
END_EVENT_TABLE()


CTransparentStaticText::CTransparentStaticText() {}

CTransparentStaticText::CTransparentStaticText(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& WXUNUSED(name) ) {
    // Set name same as label for accessibility on Windows
    Create(parent, id, label, pos, size, style, label);
}


bool CTransparentStaticText::Create(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) { 
    bool bRetVal = wxStaticText::Create(parent, id, label, pos, size, style|wxTRANSPARENT_WINDOW, name);

    SetBackgroundColour(parent->GetBackgroundColour());
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetForegroundColour(parent->GetForegroundColour());

    return bRetVal;
}


#ifndef __WXMAC__
void CTransparentStaticText::SetLabel(const wxString& label) {
    wxStaticText::SetLabel(label);
	GetParent()->RefreshRect(GetRect());
}
#endif


void CTransparentStaticText::OnPaint(wxPaintEvent& /*event*/) {
    wxPaintDC dc(this);
    dc.SetFont(GetFont());
    dc.DrawText(GetLabel(), 0, 0);
}


IMPLEMENT_DYNAMIC_CLASS (CTransparentStaticTextAssociate, wxPanel)

BEGIN_EVENT_TABLE(CTransparentStaticTextAssociate, wxPanel)
    EVT_ERASE_BACKGROUND(CTransparentStaticTextAssociate::OnEraseBackground)
    EVT_PAINT(CTransparentStaticTextAssociate::OnPaint)
    EVT_MOUSE_EVENTS(CTransparentStaticTextAssociate::OnMouse)
END_EVENT_TABLE()


CTransparentStaticTextAssociate::CTransparentStaticTextAssociate() {}

CTransparentStaticTextAssociate::CTransparentStaticTextAssociate(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    Create(parent, id, label, pos, size, style, name);
}


bool CTransparentStaticTextAssociate::Create(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) { 
    bool bRetVal = wxPanel::Create(parent, id, pos, size, style|wxTRANSPARENT_WINDOW, name);

    m_pWnd = NULL;

    SetLabel(label);
    SetFont(GetFont());

    SetBackgroundColour(parent->GetBackgroundColour());
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetForegroundColour(parent->GetForegroundColour());

    return bRetVal;
}


bool CTransparentStaticTextAssociate::SetFont(const wxFont& font) {

    bool ret = wxPanel::SetFont(font);

    InvalidateBestSize();

    wxCoord width, height;
    wxClientDC dc(this);
    dc.SetFont(font);
    dc.GetMultiLineTextExtent(GetLabel(), &width, &height);

    CacheBestSize(wxSize(width, height));

    return ret;
}


// Due to the nature of how wxWidgets handles some controls when Windows XP
//   themes are enabled, it is easier to make a simulated CheckBox or
//   RadioButton by linking two windows together. So when a mouse event
//   happens with this window it forwards the event to the associated
//   window.
bool CTransparentStaticTextAssociate::AssociateWindow(wxWindow* pWnd) {
    m_pWnd = pWnd;
    return true;
}


void CTransparentStaticTextAssociate::OnPaint(wxPaintEvent& /*event*/) {
    wxPaintDC dc(this);
    dc.SetFont(GetFont());
    dc.DrawText(GetLabel(), 1, 1);
}


void CTransparentStaticTextAssociate::OnMouse(wxMouseEvent& event) {
    if (m_pWnd) {
        wxMouseEvent evtAssociate(event);
        evtAssociate.SetId(m_pWnd->GetId());
        m_pWnd->GetEventHandler()->ProcessEvent(event);
    }

    // If we get the left button up event and we already had focus, that must
    //   mean the user clicked on the static text, So change the associated
    //   control so that it has been clicked.
    if (event.GetEventType() == wxEVT_LEFT_UP) {
        wxCheckBox* pCheckBox = wxDynamicCast(m_pWnd, wxCheckBox);
        if (pCheckBox) {
            // Send the updated click event
            wxCommandEvent evtCheckBox(wxEVT_COMMAND_CHECKBOX_CLICKED, pCheckBox->GetId());
            evtCheckBox.SetEventObject(pCheckBox);
            if (pCheckBox->IsChecked()) {
                evtCheckBox.SetInt(wxCHK_UNCHECKED);
            } else {
                evtCheckBox.SetInt(wxCHK_CHECKED);
            }
            pCheckBox->Command(evtCheckBox);
        }
    }
    event.Skip();
}


IMPLEMENT_DYNAMIC_CLASS (CLinkButton, wxBitmapButton)

BEGIN_EVENT_TABLE(CLinkButton, wxBitmapButton)
    EVT_MOUSE_EVENTS(CLinkButton::OnMouse)
END_EVENT_TABLE()


CLinkButton::CLinkButton() {}

CLinkButton::CLinkButton(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name ) {
    Create(parent, id, bitmap, pos, size, style, validator, name);
}


bool CLinkButton::Create(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name ) { 
    bool bRetVal = wxBitmapButton::Create(parent, id, bitmap, pos, size, style, validator, name);

    m_HandCursor = wxCursor(wxCURSOR_HAND);

    return bRetVal;
}


void CLinkButton::OnMouse(wxMouseEvent& event) {
    if (event.Entering()) {
        SetCursor(m_HandCursor);
    } else if (event.Leaving()) {
        SetCursor(*wxSTANDARD_CURSOR);
    }
    event.Skip();
}
