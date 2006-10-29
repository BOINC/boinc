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
    bool bRetVal = wxPanel::Create(parent, id, pos, size, style, name);

    SetBackgroundColour(parent->GetBackgroundColour());
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetForegroundColour(parent->GetForegroundColour());
    SetWindowStyle(GetWindowStyle()|wxTRANSPARENT_WINDOW);

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
    EVT_PAINT(CTransparentStaticText::OnPaint)
END_EVENT_TABLE()


CTransparentStaticText::CTransparentStaticText() {}

CTransparentStaticText::CTransparentStaticText(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    Create(parent, id, label, pos, size, style, name);
}


bool CTransparentStaticText::Create(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) { 
    bool bRetVal = wxStaticText::Create(parent, id, label, pos, size, style|wxTRANSPARENT_WINDOW, name);

    SetBackgroundColour(parent->GetBackgroundColour());
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetForegroundColour(parent->GetForegroundColour());

    return bRetVal;
}


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
        m_pWnd->ProcessEvent(event);
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

