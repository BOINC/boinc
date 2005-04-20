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

#ifndef _BOINCTASKCTRL_H_
#define _BOINCTASKCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCTaskCtrl.cpp"
#endif


class CBOINCBaseView;

class CBOINCTaskCtrl : public wxPanel {
    DECLARE_DYNAMIC_CLASS( CBOINCTaskCtrl )

public:
    CBOINCTaskCtrl();
    CBOINCTaskCtrl( CBOINCBaseView* pView, wxWindowID iTaskWindowID, int iTaskWindowFlags );

    ~CBOINCTaskCtrl();

    wxInt32 CreateTaskControls();

    virtual bool OnSaveState( wxConfigBase* pConfig );
    virtual bool OnRestoreState( wxConfigBase* pConfig );

private:

    CBOINCBaseView* m_pParent;

    wxBoxSizer*     m_pBoxSizer;
};


#endif

