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

#ifndef __sg_PanelBase__
#define __sg_PanelBase__

#include "sg_CustomControls.h" 
#include "sg_BoincSimpleFrame.h"


// TODO: Move these to events.h
enum{
    ID_CHANGE_SLIDE_TIMER = 14000,
	WEBSITE_URL_MENU_ID = 34500,
	WEBSITE_URL_MENU_ID_REMOVE_PROJECT = 34550,
	WEBSITE_URL_MENU_ID_HOMEPAGE = 34551,
};



///////////////////////////////////////////////////////////////////////////

#ifdef __WXMAC__
#define SMALL_FONT 12
#define MEDIUM_FONT 16
#define LARGE_FONT 20
#else
#define SMALL_FONT 9
#define MEDIUM_FONT 12
#define LARGE_FONT 16
#endif

#define SIDEMARGINS 30


///////////////////////////////////////////////////////////////////////////////
/// Class CSimplePanelBase
///////////////////////////////////////////////////////////////////////////////

#ifdef __WXMAC__
#include "MacBitmapComboBox.h"
#else
#include "BOINCBitmapComboBox.h"
#endif


class CSimplePanelBase : public wxPanel 
{
    DECLARE_DYNAMIC_CLASS( CSimplePanelBase )
    DECLARE_EVENT_TABLE()

	public:
        CSimplePanelBase();
		CSimplePanelBase( wxWindow* parent);
		~CSimplePanelBase();

    void ReskinInterface();
    virtual wxRect* GetProgressRect() { return NULL; }
    wxBitmap GetBackgroundBmp();
    void UpdateStaticText(CTransparentStaticText **whichText, wxString s);
    void EllipseStringIfNeeded(wxString& s, wxWindow *win);

	protected:
        void MakeBGBitMap();
        void OnPaint(wxPaintEvent& event);

        wxBitmap                    m_TaskPanelBGBitMap;
        bool                        m_GotBGBitMap;
};

#endif //__sg_PanelBase__
