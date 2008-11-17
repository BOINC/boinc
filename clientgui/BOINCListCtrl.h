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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _BOINCLISTCTRL_H_
#define _BOINCLISTCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCListCtrl.cpp"
#endif


class CBOINCBaseView;

class CBOINCListCtrl : public wxListView {
    DECLARE_DYNAMIC_CLASS(CBOINCListCtrl)

public:
    CBOINCListCtrl();
    CBOINCListCtrl(CBOINCBaseView* pView, wxWindowID iListWindowID, int iListWindowFlags);

    ~CBOINCListCtrl();

    virtual bool            OnSaveState(wxConfigBase* pConfig);
    virtual bool            OnRestoreState(wxConfigBase* pConfig);

private:
    
    virtual void            OnClick(wxCommandEvent& event);

    virtual wxString        OnGetItemText(long item, long column) const;
    virtual int             OnGetItemImage(long item) const;
    virtual wxListItemAttr* OnGetItemAttr(long item) const;

    bool                    m_bIsSingleSelection;

    CBOINCBaseView*         m_pParentView;

};


#endif

