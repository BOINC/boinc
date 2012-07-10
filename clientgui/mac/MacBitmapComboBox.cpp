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
#include "MacBitmapComboBox.h"

#define POPUPBUTTONCONTROLHEIGHT 20

// wxChoice uses CreatePopupButtonControl

const wxChar CBOINCBitmapChoiceNameStr[] = wxT("popup");
const wxChar CBOINCBitmapComboBoxNameStr[] = wxT("combo");

IMPLEMENT_DYNAMIC_CLASS(CBOINCBitmapChoice, wxChoice)

BEGIN_EVENT_TABLE(CBOINCBitmapChoice, wxChoice)
    EVT_LEFT_DOWN(CBOINCBitmapChoice::OnMouseDown)
END_EVENT_TABLE()

CBOINCBitmapChoice::CBOINCBitmapChoice() {}

CBOINCBitmapChoice::CBOINCBitmapChoice(wxWindow *parent, wxWindowID id,
            const wxString& value, 
            const wxPoint& pos,
            const wxSize& size,
            int n, const wxString choices[],
            long style,
            const wxValidator& validator,
            const wxString& name)
{
    Create(parent, id, pos, size, n, choices, style, validator, name);

    if (! value.IsEmpty()) {
        SetStringSelection(value);
    }
}

CBOINCBitmapChoice::~CBOINCBitmapChoice() {
}

void CBOINCBitmapChoice::SetItemBitmap(unsigned int n, const wxBitmap& bitmap) {
    MenuHandle mhandle = (MenuHandle) m_macPopUpMenuHandle;
    unsigned int index = n + 1;
    
    if ( mhandle == NULL || index == 0)
        return ;

    if ( bitmap.Ok() )
    {
        CGImageRef imageRef = (CGImageRef)( bitmap.CGImageCreate() ) ;
        SetMenuItemIconHandle( mhandle , index ,
                    kMenuCGImageRefType , (Handle) imageRef ) ;

#if 0// wxUSE_BMPBUTTON
        ControlButtonContentInfo info ;
        wxMacCreateBitmapButton( &info , bitmap ) ;
        if ( info.contentType != kControlNoContent )
        {
            if ( info.contentType == kControlContentIconRef )
                SetMenuItemIconHandle( mhandle , index ,
                    kMenuIconRefType , (Handle) info.u.iconRef ) ;
            else if ( info.contentType == kControlContentCGImageRef )
               SetMenuItemIconHandle( mhandle , index ,
                    kMenuCGImageRefType , (Handle) info.u.imageRef ) ;
        }
        wxMacReleaseBitmapButton( &info ) ;
#endif
    }
}
void CBOINCBitmapChoice::OnMouseDown(wxMouseEvent& event) {
    wxToolTip::Enable(false);
    event.Skip();
}





IMPLEMENT_DYNAMIC_CLASS(CBOINCBitmapComboBox, wxPanel)

BEGIN_EVENT_TABLE(CBOINCBitmapComboBox, wxPanel)
//	EVT_ERASE_BACKGROUND(CBOINCBitmapComboBox::OnEraseBackground)
    EVT_PAINT(CBOINCBitmapComboBox::OnPaint)
//    EVT_CHOICE(CBOINCBitmapComboBox::OnSelection)
END_EVENT_TABLE()

CBOINCBitmapComboBox::CBOINCBitmapComboBox() {}

CBOINCBitmapComboBox::CBOINCBitmapComboBox(wxWindow *parent, wxWindowID id,
            const wxString& value, 
            const wxPoint& pos,
            const wxSize& size,
            int n, const wxString choices[],
            long style,
            const wxValidator& validator,
            const wxString& name) :
            wxPanel( parent, id, pos, size, wxCLIP_CHILDREN | wxBORDER_NONE )
{
    int i;

    m_ChoiceControl = new CBOINCBitmapChoice(this, id, value, wxDefaultPosition, wxSize(size.x, POPUPBUTTONCONTROLHEIGHT), n, choices, style, validator);
    m_bHaveLargeBitmaps = (size.y > 0);
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
    int margin = m_bHaveLargeBitmaps ? (size.y - POPUPBUTTONCONTROLHEIGHT)/2 : 0;
	bSizer1->Add( m_ChoiceControl, 1, wxTOP | wxBOTTOM | wxEXPAND, margin);
	this->SetSizer( bSizer1 );
    Layout();
    if (m_bHaveLargeBitmaps) {
        for (i=0; i<n; ++i) {
            m_BitmapCache.push_back(wxNullBitmap);
        }
    }
        Connect(id, wxEVT_COMMAND_CHOICE_SELECTED,
        (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &CBOINCBitmapComboBox::OnSelection
    );

}

CBOINCBitmapComboBox::~CBOINCBitmapComboBox() {
    Clear();
}

void CBOINCBitmapComboBox::SetItemBitmap(unsigned int n, const wxBitmap& bitmap) {
    unsigned int cacheSize, i;

    if (m_bHaveLargeBitmaps) {
        cacheSize = m_BitmapCache.size();
        for (i=cacheSize; i<n; ++i) {
            m_BitmapCache.push_back(wxNullBitmap);
        }
#if 0
        if (m_BitmapCache.at(n) != NULL) {
            delete m_BitmapCache.at(n);
            m_BitmapCache.at(n) = NULL;
        }
#endif
        wxBitmap* bm = new wxBitmap(bitmap);
        m_BitmapCache.at(n) = *bm;
        delete bm;
    }
    
    m_ChoiceControl->SetItemBitmap(n, bitmap);
    if (n == (unsigned int)m_ChoiceControl->GetSelection()) {
        Refresh();
    }
}


void CBOINCBitmapComboBox::SetStringSelection(const wxString& text) {
    m_ChoiceControl->SetStringSelection(text);
    Refresh();
}


void CBOINCBitmapComboBox::SetSelection(int sel) {
    m_ChoiceControl->SetSelection(sel);
    Refresh();
}


int CBOINCBitmapComboBox::Append(const wxString& item, const wxBitmap& bitmap) {
    if (m_bHaveLargeBitmaps) {
        m_BitmapCache.push_back(bitmap);
    }
    
    int n = m_ChoiceControl->Append(item);
    SetItemBitmap(n, bitmap);
    return n;
}


int CBOINCBitmapComboBox::Append(const wxString& item, const wxBitmap& bitmap, void *clientData) {
    if (m_bHaveLargeBitmaps) {
        m_BitmapCache.push_back(bitmap);
    }

    int n = m_ChoiceControl->Append(item, clientData);
    SetItemBitmap(n, bitmap);
    return n;
}


int CBOINCBitmapComboBox::Insert(const wxString& item, const wxBitmap& bitmap, unsigned int pos) {
    if (m_bHaveLargeBitmaps) {
        std::vector<wxBitmap>::iterator insertionPoint = m_BitmapCache.begin();
        wxBitmap* bm = new wxBitmap(bitmap);
        m_BitmapCache.insert(insertionPoint + pos, *bm);
        delete bm;
    }
    int n = m_ChoiceControl->Insert(item, pos);
    SetItemBitmap(n, bitmap);
    return n;
}


int CBOINCBitmapComboBox::Insert(const wxString& item, const wxBitmap& bitmap, unsigned int pos, void *clientData) {
    if (m_bHaveLargeBitmaps) {
        std::vector<wxBitmap>::iterator insertionPoint = m_BitmapCache.begin();
        wxBitmap* bm = new wxBitmap(bitmap);
        m_BitmapCache.insert(insertionPoint + pos, *bm);
        delete bm;
    }
    
    int n = m_ChoiceControl->Insert(item, pos, clientData);
    SetItemBitmap(n, bitmap);
    return n;
}


void CBOINCBitmapComboBox::Delete(unsigned int n) {
    if (n < m_ChoiceControl->GetCount()) {
        if (m_bHaveLargeBitmaps) {
            std::vector<wxBitmap>::iterator deletionPoint = m_BitmapCache.begin();
            m_BitmapCache.erase(deletionPoint + n);
        }
        
        m_ChoiceControl->Delete(n);
        Refresh();
    }
}


void CBOINCBitmapComboBox::Clear() {
    m_BitmapCache.clear();
    int count = GetCount();
	for(int j = count-1; j >=0; --j) {
        wxASSERT(!m_ChoiceControl->GetClientData(j));
        m_ChoiceControl->SetClientData(j, NULL);
    }
    m_ChoiceControl->Clear();
}


void CBOINCBitmapComboBox::OnSelection(wxCommandEvent& event) {
    Refresh();      // To draw the bitmap
    event.Skip();
}


void CBOINCBitmapComboBox::OnPaint(wxPaintEvent& event) {
    int x, y;
	wxPaintDC myDC(this);
    unsigned int i = GetSelection();
    if (m_BitmapCache.size() <= i) {
        return;
    }
    
    wxPen oldPen = myDC.GetPen();
    wxBrush oldBrush = myDC.GetBrush();
    int oldMode = myDC.GetBackgroundMode();

    myDC.SetPen(*wxMEDIUM_GREY_PEN);
    myDC.SetBrush(*wxTRANSPARENT_BRUSH);
    myDC.SetBackgroundMode(wxSOLID);

    GetSize(&x, &y);
    if ((m_BitmapCache.at(i)).Ok()) {
        myDC.DrawBitmap(m_BitmapCache.at(i), 9, 1, false);
        myDC.DrawRectangle(8, 0, y, y);
    }
    
    // Restore Mode, Pen and Brush 
    myDC.SetBackgroundMode(oldMode);
    myDC.SetPen(oldPen);
    myDC.SetBrush(oldBrush);
}


void CBOINCBitmapComboBox::EmptyBitmapCache() {
#if 0
    unsigned int i, cacheSize;
    
    cacheSize = m_BitmapCache.size();
    for (i=0; i<cacheSize; i++) {
        if (m_BitmapCache.at(i) != NULL) {
            delete m_BitmapCache.at(i);
            m_BitmapCache.at(i) = NULL;
        }
    }
#endif
    m_BitmapCache.clear();
}


void CBOINCBitmapComboBox::SetToolTip(wxString& s) {
    m_ChoiceControl->SetToolTip(s);
}


void CBOINCBitmapComboBox::SetToolTip(wxToolTip* tip) {
    m_ChoiceControl->SetToolTip(tip);
}
