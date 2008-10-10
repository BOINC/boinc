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

#ifndef _SGUILISTCTRL_H_
#define _SGUILISTCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_SGUIListControl.cpp"
#endif

#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL

class CPanelMessages;

class CSGUIListCtrl : public wxListView {
    DECLARE_DYNAMIC_CLASS(CSGUIListCtrl)

public:
    CSGUIListCtrl();
    CSGUIListCtrl(CPanelMessages* pView, wxWindowID iListWindowID, int iListWindowFlags);

private:
    
    virtual wxString        OnGetItemText(long item, long column) const;
    virtual int             OnGetItemImage(long item) const;
    virtual wxListItemAttr* OnGetItemAttr(long item) const;
    virtual wxColour        GetBackgroundColour();

    bool                    m_bIsSingleSelection;

    CPanelMessages*         m_pParentView;
};

#endif
