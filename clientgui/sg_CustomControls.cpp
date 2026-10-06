// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#include "stdwx.h"
#include "sg_CustomControls.h"


IMPLEMENT_DYNAMIC_CLASS (CTransparentStaticLine, wxPanel)

CTransparentStaticLine::CTransparentStaticLine() {}

CTransparentStaticLine::CTransparentStaticLine(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    Create(parent, id, pos, size, style, name);
}


bool CTransparentStaticLine::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    bool bRetVal = wxPanel::Create(parent, id, pos, size, style|wxTRANSPARENT_WINDOW, name);

    SetBackgroundColour(parent->GetBackgroundColour());
    SetForegroundColour(parent->GetForegroundColour());
    SetLineColor(GetForegroundColour());

    Bind(wxEVT_PAINT, [this](wxPaintEvent& event){ OnPaint(event); });
    return bRetVal;
}


void CTransparentStaticLine::OnPaint(wxPaintEvent& /*event*/) {
    wxPaintDC dc(this);
    wxPen pen = wxPen(GetLineColor(), 1);
    dc.SetPen(pen);
    dc.DrawLine(0, 0, GetSize().GetWidth(), 0);
}


IMPLEMENT_DYNAMIC_CLASS (CTransparentStaticText, wxStaticText)


CTransparentStaticText::CTransparentStaticText() {}

CTransparentStaticText::CTransparentStaticText(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& WXUNUSED(name) ) {
    // Set name same as label for accessibility on Windows
    Create(parent, id, label, pos, size, style, label);
}


bool CTransparentStaticText::Create(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    bool bRetVal = wxStaticText::Create(parent, id, label, pos, size, style|wxTRANSPARENT_WINDOW, name);

#ifndef __WXGTK__
    SetBackgroundColour(parent->GetBackgroundColour());
    SetForegroundColour(parent->GetForegroundColour());
#endif
    Bind(wxEVT_ERASE_BACKGROUND, [this](wxEraseEvent& event){ OnEraseBackground(event); });
    return bRetVal;
}


IMPLEMENT_DYNAMIC_CLASS (CTransparentButton, wxButton)

CTransparentButton::CTransparentButton() {}

CTransparentButton::CTransparentButton(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name )
{
    Create(parent, id, label, pos, size, style, validator, name);
}


bool CTransparentButton::Create(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name )
{
#ifdef __WXMAC__
    SetBackgroundStyle(wxBG_STYLE_TRANSPARENT); // Must come before wxButton::Create()
#endif
    bool bRetVal = wxButton::Create(parent, id, label, pos, size, style|wxTRANSPARENT_WINDOW, validator, name);

#ifndef __WXGTK__
    SetBackgroundColour(parent->GetBackgroundColour());
    SetForegroundColour(parent->GetForegroundColour());
#endif

    Bind(wxEVT_ERASE_BACKGROUND, [this](wxEraseEvent& event){ OnEraseBackground(event); });
    return bRetVal;
}


void CTransparentButton::SetLabel(const wxString& label)
{
    wxButton::SetLabel(label);
}


void CTransparentButton::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
}


IMPLEMENT_DYNAMIC_CLASS (CTransparentHyperlinkCtrl, wxHyperlinkCtrl)

CTransparentHyperlinkCtrl::CTransparentHyperlinkCtrl(wxWindow *parent,
                    wxWindowID id,
                    const wxString& label, const wxString& url,
                    const wxPoint& pos,
                    const wxSize& size,
                    long style,
                    const wxString& name,
                    wxBitmap** parentsBgBmp)
{
    Create(parent, id, label, url, pos, size, style, name, parentsBgBmp);
}


bool CTransparentHyperlinkCtrl::Create(wxWindow *parent,
                    wxWindowID id,
                    const wxString& label, const wxString& url,
                    const wxPoint& pos,
                    const wxSize& size,
                    long style,
                    const wxString& name,
                    wxBitmap** parentsBgBmp)
{
    bool bRetVal = wxHyperlinkCtrl::Create(parent, id, label, url, pos, size, style, name);

    m_pParentsBgBmp = parentsBgBmp;

#if !defined(__WXMAC__) && !defined(__WXGTK__)
    SetBackgroundColour(parent->GetBackgroundColour());
    SetForegroundColour(wxColour(0, 100, 225));
    wxFont myFont = GetFont();
    myFont.SetUnderlined(true);
    SetFont(myFont);
#endif
#ifndef __WXMAC__
    Bind(wxEVT_ERASE_BACKGROUND, [this](wxEraseEvent& event){ OnEraseBackground(event); });
    Bind(wxEVT_PAINT, [this](wxPaintEvent& event){ OnPaint(event); });
#endif

    return bRetVal;
}


#ifndef __WXMAC__
void CTransparentHyperlinkCtrl::OnEraseBackground(wxEraseEvent& event)
{
    if (m_pParentsBgBmp && *m_pParentsBgBmp) {
        wxMemoryDC memDC(**m_pParentsBgBmp);
        wxSize sz = GetClientSize();
        wxDC *dc = event.GetDC();
        wxCoord x=0, y=0, xx, yy;
        wxWindow* win = this;
        do {
            win->GetPosition(&xx, &yy);
            x += xx;
            y += yy;
            win = win->GetParent();
        } while (!win->IsTopLevel());
        dc->Blit(0, 0, sz.GetWidth(), sz.GetHeight(), &memDC, x, y, wxCOPY);
    } else {
        event.Skip();
    }
}
#endif


IMPLEMENT_DYNAMIC_CLASS (CTransparentStaticTextAssociate, wxPanel)

CTransparentStaticTextAssociate::CTransparentStaticTextAssociate(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    Create(parent, id, label, pos, size, style, name);
}


bool CTransparentStaticTextAssociate::Create(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    bool bRetVal = wxPanel::Create(parent, id, pos, size, style|wxTRANSPARENT_WINDOW, name);

    m_pWnd = NULL;

    SetLabel(label);
    SetFont(GetFont());

    SetBackgroundColour(parent->GetBackgroundColour());
    SetForegroundColour(parent->GetForegroundColour());

    Bind(wxEVT_ERASE_BACKGROUND, [this](wxEraseEvent& event){ OnEraseBackground(event); });
    Bind(wxEVT_PAINT, [this](wxPaintEvent& event){ OnPaint(event); });
    Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event){ OnMouse(event); });
    Bind(wxEVT_LEFT_UP, [this](wxMouseEvent& event){ OnMouse(event); });
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


#ifdef __WXMSW__
IMPLEMENT_DYNAMIC_CLASS (CTransparentStaticBitmap, wxStaticText)

CTransparentStaticBitmap::CTransparentStaticBitmap() {}

CTransparentStaticBitmap::CTransparentStaticBitmap(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxString& WXUNUSED(name) ) {
    Create(parent, id, bitmap, pos, size, style);
}


bool CTransparentStaticBitmap::Create(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
    bool bRetVal = wxPanel::Create(parent, id, pos, size, style|wxTRANSPARENT_WINDOW, name);

    SetBackgroundColour(parent->GetBackgroundColour());
    SetForegroundColour(parent->GetForegroundColour());

    m_bitMap = wxBitmap(bitmap);

    Bind(wxEVT_ERASE_BACKGROUND, [this](wxEraseEvent& event){ OnEraseBackground(event); });
    Bind(wxEVT_PAINT, [this](wxPaintEvent& event){ OnPaint(event); });

    return bRetVal;
}


void CTransparentStaticBitmap::OnPaint(wxPaintEvent& /*event*/) {
    wxPaintDC dc(this);
    dc.DrawBitmap(m_bitMap, 0, 0, true);
}
#endif


IMPLEMENT_DYNAMIC_CLASS (CTransparentCheckBox, wxCheckBox)

CTransparentCheckBox::CTransparentCheckBox(wxWindow *parent, wxWindowID id, const wxString& label,
            const wxPoint& pos,
            const wxSize& size, long style,
            const wxValidator& validator,
            const wxString& name,
            wxBitmap** parentsBgBmp
        ) {
        Create(parent, id, label, pos, size, style, validator, name, parentsBgBmp);
}


bool CTransparentCheckBox::Create(wxWindow *parent, wxWindowID id, const wxString& label,
            const wxPoint& pos,
            const wxSize& size, long style,
            const wxValidator& validator,
            const wxString& name,
            wxBitmap** parentsBgBmp
            ) {
    wxString spacedLabel = wxT(" ") + label;
    bool bRetVal = wxCheckBox::Create(parent, id, spacedLabel, pos, size, style, validator, name);

    m_pParentsBgBmp = parentsBgBmp;

#if !defined(__WXMAC__) && !defined(__WXGTK__)
    SetBackgroundColour(parent->GetBackgroundColour());
    SetForegroundColour(parent->GetForegroundColour());
#endif
#ifndef __WXMAC__
    Bind(wxEVT_ERASE_BACKGROUND, [this](wxEraseEvent& event){ OnEraseBackground(event); });
#endif
    return bRetVal;
}


#ifndef __WXMAC__
void CTransparentCheckBox::OnEraseBackground(wxEraseEvent& event)
{
    if (m_pParentsBgBmp && *m_pParentsBgBmp) {
        wxMemoryDC memDC(**m_pParentsBgBmp);
        wxSize sz = GetClientSize();
        wxDC *dc = event.GetDC();
        wxCoord x=0, y=0, xx, yy;
        wxWindow* win = this;
        do {
            win->GetPosition(&xx, &yy);
            x += xx;
            y += yy;
            win = win->GetParent();
        } while (!win->IsTopLevel());
        dc->Blit(0, 0, sz.GetWidth(), sz.GetHeight(), &memDC, x, y, wxCOPY);
    } else {
        event.Skip();
    }
}
#endif


IMPLEMENT_DYNAMIC_CLASS (CLinkButton, wxBitmapButton)

CLinkButton::CLinkButton() {}

CLinkButton::CLinkButton(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name ) {
    Create(parent, id, bitmap, pos, size, style, validator, name);
}


bool CLinkButton::Create(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name ) {
    bool bRetVal = wxBitmapButton::Create(parent, id, bitmap, pos, size, style, validator, name);

    m_HandCursor = wxCursor(wxCURSOR_HAND);

    Bind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent& event){ CLinkButton::OnMouse(event); });
    Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& event){ CLinkButton::OnMouse(event); });
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
