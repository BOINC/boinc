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

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "miofile.h"
#include "SkinManager.h"
#include "sg_PanelBase.h"

#define RECTANGLERADIUS 15.0

IMPLEMENT_DYNAMIC_CLASS(CSimplePanelBase, wxPanel)

BEGIN_EVENT_TABLE(CSimplePanelBase, wxPanel)
    	EVT_PAINT(CSimplePanelBase::OnPaint)
END_EVENT_TABLE()


CSimplePanelBase::CSimplePanelBase() {
}


CSimplePanelBase::CSimplePanelBase( wxWindow* parent ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN | wxBORDER_NONE )
{
#ifdef __WXMSW__
    SetDoubleBuffered(true);
#endif

#ifdef __WXMAC__
    // Tell accessibility aids to ignore this panel (but not its contents)
    HIObjectSetAccessibilityIgnored((HIObjectRef)GetHandle(), true);
#endif
}


CSimplePanelBase::~CSimplePanelBase()
{
}


void CSimplePanelBase::ReskinInterface() {
    m_GotBGBitMap = false;
}

// Create a background bitmap simulating partial transparency
void CSimplePanelBase::MakeBGBitMap() {
    wxRect r;
    wxBitmap rawBmp;
    wxBitmap whiteBmp;
    wxImage bgImage;
    wxImage whiteImage;
    register unsigned char *bgImagePixels;
    register unsigned char *whitePixels;
    register int i, j, k;
    CSimpleGUIPanel* backgroundPanel = (CSimpleGUIPanel*)GetParent();
    wxPen bgPen(*wxWHITE, 1, wxTRANSPARENT);
    wxBrush bgBrush(*wxWHITE);
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    int white_weight = pSkinSimple->GetPanelOpacity();
    int image_weight = MAX_OPACITY - white_weight;

// Workaround for CSimpleGUIPanel not reliably getting 
// Paint or EraseBackground events under Linux
#if (!(defined(__WXMSW_) || defined(__WXMAC__)))
    backgroundPanel->SetBackgroundBitmap();
#endif
    
    GetPosition(&r.x, &r.y);
    GetSize(&r.width, &r.height);
    wxBitmap *bgBmp(backgroundPanel->GetBackgroundBitMap());
    wxRect bgRect(0, 0, bgBmp->GetWidth(), bgBmp->GetHeight());
    if (bgRect.Contains(r)) {
        rawBmp = bgBmp->GetSubBitmap(r);
    } else {
        fprintf(stderr, "SimpleGUI background image is too small\n");
        rawBmp = wxBitmap(r.width, r.height);
        wxMemoryDC dc(rawBmp);
        wxPen rawPen(*wxBLACK, 1, wxTRANSPARENT);
        wxBrush rawBrush(*wxBLACK);
        dc.SetBackgroundMode(wxSOLID);
        dc.SetPen(rawPen);
        dc.SetBrush(rawBrush);
        dc.DrawRectangle(0, 0, r.width, r.height);
    }
    
    whiteBmp = wxBitmap(r.width, r.height);
    wxMemoryDC dc(whiteBmp);
    dc.SetBackgroundMode(wxSOLID);
    dc.SetPen(bgPen);
    dc.SetBrush(bgBrush);
    dc.DrawRoundedRectangle(0, 0, r.width, r.height, RECTANGLERADIUS);

    bgImage = rawBmp.ConvertToImage();
    bgImagePixels = bgImage.GetData(); // RGBRGBRGB...
    whiteImage = whiteBmp.ConvertToImage();
    whitePixels = whiteImage.GetData(); // RGBRGBRGB...

    for (i=0; i<r.width; ++i) {
        for (j=0; j<r.height; ++j) {
            if (*whitePixels) {
                k = (((unsigned int)*bgImagePixels * image_weight) + ((unsigned int)*whitePixels++ * white_weight));
                *bgImagePixels++ = k / MAX_OPACITY;
                k = (((unsigned int)*bgImagePixels * image_weight) + ((unsigned int)*whitePixels++ * white_weight));
                *bgImagePixels++ = k / MAX_OPACITY;
                k = (((unsigned int)*bgImagePixels * image_weight) + ((unsigned int)*whitePixels++ * white_weight));
                *bgImagePixels++ = k / MAX_OPACITY;
            } else {
                bgImagePixels += 3;
                whitePixels += 3;
            }
        }
    }

    m_TaskPanelBGBitMap = wxBitmap(bgImage);
    m_GotBGBitMap = true;
}


// Linux does not reliably generate EraseBackground 
// events here, so use Paint events
void CSimplePanelBase::OnPaint(wxPaintEvent& /*event*/) {
    wxPaintDC dc(this);

    if (!m_GotBGBitMap) {
        MakeBGBitMap();
    }

    dc.DrawBitmap(m_TaskPanelBGBitMap, 0, 0);
    wxPen oldPen= dc.GetPen();
    wxBrush oldBrush = dc.GetBrush();
    int oldMode = dc.GetBackgroundMode();
    wxCoord w, h;
    wxPen bgPen(*wxLIGHT_GREY, 3);
    wxBrush bgBrush(*wxLIGHT_GREY, wxTRANSPARENT);

    dc.SetBackgroundMode(wxSOLID);
    dc.SetPen(bgPen);
    dc.SetBrush(bgBrush);
    dc.GetSize(&w, &h);
    dc.DrawRoundedRectangle(0, 0, w, h, RECTANGLERADIUS);

#ifdef __WXMAC__
    // Mac progress bar can be hard to see on a colored 
    // background, so put it on a white background
    wxRect* progressRect = GetProgressRect();
    if (progressRect) {
        dc.SetPen(*wxBLACK_PEN);
        dc.SetBrush(*wxWHITE_BRUSH);
        dc.DrawRoundedRectangle(progressRect->x, progressRect->y, progressRect->width, progressRect->height, 2);
    }
#endif

    // Restore Mode, Pen and Brush 
    dc.SetBackgroundMode(oldMode);
    dc.SetPen(oldPen);
    dc.SetBrush(oldBrush);
}


wxBitmap CSimplePanelBase::GetBackgroundBmp() {
    if (!m_GotBGBitMap) {
        MakeBGBitMap();
    }

    return m_TaskPanelBGBitMap;
}


void CSimplePanelBase::UpdateStaticText(CTransparentStaticText **whichText, wxString s) {
    EllipseStringIfNeeded(s, *whichText);
    if ((*whichText)->GetLabel() != s) {
        (*whichText)->SetLabel(s);
        (*whichText)->SetName(s);   // For accessibility on Windows
    }
}


void CSimplePanelBase::EllipseStringIfNeeded(wxString& s, wxWindow *win) {
    int x, y;
    int w, h;
    wxSize sz = GetSize();
    win->GetPosition(&x, &y);
    int maxWidth = sz.GetWidth() - x - SIDEMARGINS;
    
    win->GetTextExtent(s, &w, &h);
    
    // Adapted from ellipis code in wxRendererGeneric::DrawHeaderButtonContents()
    if (w > maxWidth) {
        int ellipsisWidth;
        win->GetTextExtent( wxT("..."), &ellipsisWidth, NULL);
        if (ellipsisWidth > maxWidth) {
            s.Clear();
            w = 0;
        } else {
            do {
                s.Truncate( s.length() - 1 );
                win->GetTextExtent( s, &w, &h);
            } while (((w + ellipsisWidth) > maxWidth) && s.length() );
            s.append( wxT("...") );
            w += ellipsisWidth;
        }
    }
}
