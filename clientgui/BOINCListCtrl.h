// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//

#ifndef _BOINCLISTCTRL_H_
#define _BOINCLISTCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCListCtrl.cpp"
#endif


class CBOINCBaseView;

class CBOINCListCtrl : public wxListView
{
    DECLARE_DYNAMIC_CLASS( CBOINCListCtrl )

public:
    CBOINCListCtrl();
    CBOINCListCtrl( CBOINCBaseView* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags );

    ~CBOINCListCtrl();

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

private:
    
    virtual void            OnClick( wxCommandEvent& event );
    virtual void            OnSelected( wxListEvent& event );
    virtual void            OnDeselected( wxListEvent& event );

    virtual wxString        OnGetItemText( long item, long column ) const;
    virtual int             OnGetItemImage( long item ) const;
    virtual wxListItemAttr* OnGetItemAttr( long item ) const;

    bool                    m_bIsSingleSelection;

    CBOINCBaseView*         m_pParentView;

};


#endif

