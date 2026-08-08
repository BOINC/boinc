// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#ifndef BOINC_SG_CUSTOMCONTROLS_H
#define BOINC_SG_CUSTOMCONTROLS_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_CustomControls.cpp"
#endif


class CTransparentStaticLine : public wxPanel
{
    DECLARE_DYNAMIC_CLASS (CTransparentStaticLine)

public:
    CTransparentStaticLine();
    CTransparentStaticLine(
        wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name= wxStaticTextNameStr
    );

    bool Create(
        wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name= wxStaticTextNameStr
    );

    wxColour GetLineColor() const { return m_LineColor ; }
    void     SetLineColor(wxColour value) { if (value != wxNullColour) m_LineColor = value ; }

    virtual bool HasTransparentBackground() { return true; };

    void     OnPaint(wxPaintEvent& event);

    DECLARE_EVENT_TABLE()

private:
    wxColour m_LineColor;
};


class CTransparentStaticText : public wxStaticText
{
    DECLARE_DYNAMIC_CLASS (CTransparentStaticText)

public:
    CTransparentStaticText();
    CTransparentStaticText(
        wxWindow* parent,
        wxWindowID id,
        const wxString& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name= wxStaticTextNameStr
    );

    bool Create(
        wxWindow* parent,
        wxWindowID id,
        const wxString& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name= wxStaticTextNameStr
    );

    virtual bool HasTransparentBackground() { return true; };

    virtual void OnEraseBackground(wxEraseEvent& /*event*/) {};

    DECLARE_EVENT_TABLE()
};


class CTransparentButton : public wxButton
{
    DECLARE_DYNAMIC_CLASS (CTransparentButton)
    DECLARE_EVENT_TABLE()

public:
    CTransparentButton();
    CTransparentButton(
        wxWindow* parent,
        wxWindowID id,
        const wxString& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxButtonNameStr
    );

    bool Create(
        wxWindow* parent,
        wxWindowID id,
        const wxString& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxButtonNameStr
    );

    virtual bool HasTransparentBackground() { return true; };

    virtual void SetLabel(const wxString& label);
    virtual void OnEraseBackground(wxEraseEvent& event);
};


class CTransparentHyperlinkCtrl : public wxHyperlinkCtrl
{
    DECLARE_DYNAMIC_CLASS (CTransparentHyperlinkCtrl)

public:
    CTransparentHyperlinkCtrl();
    CTransparentHyperlinkCtrl(wxWindow *parent,
                    wxWindowID id,
                    const wxString& label, const wxString& url,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxHL_DEFAULT_STYLE,
                    const wxString& name = wxHyperlinkCtrlNameStr,
                    wxBitmap** parentsBgBmp = NULL
                    );

    // Creation function (for two-step construction).
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& label, const wxString& url,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxHL_DEFAULT_STYLE,
                const wxString& name = wxHyperlinkCtrlNameStr,
                wxBitmap** parentsBgBmp = NULL
                );

private:
    wxBitmap** m_pParentsBgBmp;

#ifndef __WXMAC__
    public:
    virtual void OnEraseBackground(wxEraseEvent& event);

    DECLARE_EVENT_TABLE()
#endif
};


class CTransparentStaticTextAssociate : public wxPanel
{
    DECLARE_DYNAMIC_CLASS (CTransparentStaticTextAssociate)

public:
    CTransparentStaticTextAssociate();
    CTransparentStaticTextAssociate(
        wxWindow* parent,
        wxWindowID id,
        const wxString& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name= wxStaticTextNameStr
    );

    bool Create(
        wxWindow* parent,
        wxWindowID id,
        const wxString& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name= wxStaticTextNameStr
    );

    virtual bool HasTransparentBackground() { return true; };
    virtual bool SetFont(const wxFont& font);

    virtual bool AssociateWindow(wxWindow* pWnd);

    virtual void OnEraseBackground(wxEraseEvent& /*event*/) {};
    virtual void OnPaint(wxPaintEvent& event);
    virtual void OnMouse(wxMouseEvent& event);

    DECLARE_EVENT_TABLE()
private:
    wxWindow*   m_pWnd;
};


#ifndef __WXMSW__
#define CTransparentStaticBitmap wxStaticBitmap
#else
class CTransparentStaticBitmap : public wxPanel
{
    DECLARE_DYNAMIC_CLASS (CTransparentStaticBitmap)

public:
    CTransparentStaticBitmap();
    CTransparentStaticBitmap(
        wxWindow* parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name= wxStaticBitmapNameStr
    );

    bool Create(
        wxWindow* parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name= wxStaticBitmapNameStr
    );

    virtual bool HasTransparentBackground() { return true; };

    virtual void OnEraseBackground(wxEraseEvent& /*event*/) {};
    virtual void OnPaint(wxPaintEvent& event);

    DECLARE_EVENT_TABLE()
private:
    wxBitmap   m_bitMap;
};
#endif


class CTransparentCheckBox : public wxCheckBox
{
    DECLARE_DYNAMIC_CLASS (CTransparentCheckBox)

public:
    CTransparentCheckBox();
    CTransparentCheckBox(wxWindow *parent, wxWindowID id, const wxString& label,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxCheckBoxNameStr,
            wxBitmap** parentsBgBmp = NULL
            );

    bool Create(wxWindow *parent, wxWindowID id, const wxString& label,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxCheckBoxNameStr,
            wxBitmap** parentsBgBmp = NULL
            );

private:
    wxBitmap** m_pParentsBgBmp;

#ifndef __WXMAC__
public:
    virtual void OnEraseBackground(wxEraseEvent& event);

    DECLARE_EVENT_TABLE()
#endif
};


class CLinkButton : public wxBitmapButton
{
    DECLARE_DYNAMIC_CLASS (CLinkButton)

public:
    CLinkButton();
    CLinkButton(
        wxWindow* parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name= wxButtonNameStr
    );

    bool Create(
        wxWindow* parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name= wxButtonNameStr
    );

    virtual void OnMouse(wxMouseEvent& event);

    DECLARE_EVENT_TABLE()
private:
    wxCursor m_HandCursor;
};

#endif
