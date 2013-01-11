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

#ifndef _BOINCHTMLLBOX_H_
#define _BOINCHTMLLBOX_H_

#include "wx/htmllbox.h"               // base class

extern const wxChar BOINCHtmlListBoxNameStr[];

// ----------------------------------------------------------------------------
// CBOINCHtmlListBox
// ----------------------------------------------------------------------------

// small border always added to the cells:
#define CELL_BORDER 2

class CBOINCHtmlListBox : public wxHtmlListBox
{
    DECLARE_ABSTRACT_CLASS(CBOINCHtmlListBox)
public:
    // constructors and such
    // ---------------------

    // default constructor, you must call Create() later
    CBOINCHtmlListBox();

    // normal constructor which calls Create() internally
    CBOINCHtmlListBox(wxWindow *parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = 0,
                  const wxString& name = BOINCHtmlListBoxNameStr);

    // destructor cleans up whatever resources we use
    virtual ~CBOINCHtmlListBox();
    
    int OnGetItemHeight(size_t i) { return (int)OnMeasureItem(i); }
    int GetItemHeight(size_t i) { return (int)OnMeasureItem(i); }
    
    void GetItemRect(size_t item, wxRect& rect)
    {
        rect.height = GetItemHeight(item);
        rect.width = GetSize().GetWidth() - 4;
    }

protected:
    // The following overrides of methods in wxHtmlListBox and wxVListBox make 
    // it appear as if we don't allow the user to select entries in the notices.
    wxColour GetSelectedTextColour(const wxColour& colFg) const { return colFg; }
    wxColour GetSelectedTextBgColour(const wxColour& colBg) const { return colBg; }
    void OnDrawBackground(wxDC&, const wxRect&, size_t) const {}

    // The following overrides of methods in wxHtmlListBox 
    // reduce CPU usage and avoid crashes
    void OnMouseMove(wxMouseEvent&) {}
    void OnInternalIdle() {}

private:
     virtual wxHtmlOpeningStatus OnHTMLOpeningURL(wxHtmlURLType type,
                                                 const wxString& url,
                                                 wxString *redirect) const;
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(CBOINCHtmlListBox)
};

#endif // _BOINCHTMLLBOX_H_

