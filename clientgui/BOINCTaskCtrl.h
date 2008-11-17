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

#ifndef _BOINCTASKCTRL_H_
#define _BOINCTASKCTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCTaskCtrl.cpp"
#endif


class CTaskItem;
class CTaskItemGroup;
class CBOINCBaseView;

class CBOINCTaskCtrl : public wxScrolledWindow {
    DECLARE_DYNAMIC_CLASS( CBOINCTaskCtrl )

public:
    CBOINCTaskCtrl();
    CBOINCTaskCtrl( CBOINCBaseView* pView, wxWindowID iTaskWindowID, int iTaskWindowFlags );

    ~CBOINCTaskCtrl();

    wxInt32 DeleteTaskGroupAndTasks( CTaskItemGroup* pGroup );
    wxInt32 DisableTaskGroupTasks( CTaskItemGroup* pGroup );
    wxInt32 EnableTaskGroupTasks( CTaskItemGroup* pGroup );

    wxInt32 DeleteTask( CTaskItemGroup* pGroup, CTaskItem* pItem );
    wxInt32 DisableTask( CTaskItem* pItem );
    wxInt32 EnableTask( CTaskItem* pItem );
    wxInt32 UpdateTask( CTaskItem* pItem, wxString strName, wxString strDescription );

    wxInt32 UpdateControls();

    virtual bool OnSaveState( wxConfigBase* pConfig );
    virtual bool OnRestoreState( wxConfigBase* pConfig );

private:

    CBOINCBaseView*  m_pParent;

    wxBoxSizer*      m_pSizer;
};


#endif

