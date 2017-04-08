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

#ifndef BOINC_BOINCTASKCTRL_H
#define BOINC_BOINCTASKCTRL_H

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

#ifdef __WXMSW__
    void OnChildFocus(wxChildFocusEvent& event);
#endif

    void EllipseStringIfNeeded( wxString& s );

private:

    CBOINCBaseView*  m_pParent;

    wxBoxSizer*      m_pSizer;

#ifdef __WXMSW__
    DECLARE_EVENT_TABLE()
#endif
};


#endif

