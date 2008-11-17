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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCTaskCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCTaskCtrl.h"


IMPLEMENT_DYNAMIC_CLASS(CBOINCTaskCtrl, wxScrolledWindow)


CBOINCTaskCtrl::CBOINCTaskCtrl() {}


CBOINCTaskCtrl::CBOINCTaskCtrl(CBOINCBaseView* pView, wxWindowID iTaskWindowID, wxInt32 iTaskWindowFlags) :
    wxScrolledWindow(pView, iTaskWindowID, wxDefaultPosition, wxSize(200, -1), iTaskWindowFlags)
{
    m_pParent = pView;
    m_pSizer = NULL;

    SetVirtualSize( 200, 1000 );
    EnableScrolling(false, true);
    SetScrollRate( 0, 10 );
}


CBOINCTaskCtrl::~CBOINCTaskCtrl() {}


wxInt32 CBOINCTaskCtrl::DeleteTaskGroupAndTasks( CTaskItemGroup* pGroup ) {
    unsigned int i;
    CTaskItem*   pItem = NULL;

    for (i=0; i < pGroup->m_Tasks.size(); i++) {
        pItem = pGroup->m_Tasks[i];
        DeleteTask(pGroup, pItem);
    }
    if (pGroup->m_pStaticBoxSizer) {
        m_pSizer->Detach(pGroup->m_pStaticBoxSizer);
        pGroup->m_pStaticBoxSizer->Detach(pGroup->m_pStaticBox);

        delete pGroup->m_pStaticBox;
        delete pGroup->m_pStaticBoxSizer;

        pGroup->m_pStaticBox = NULL;
        pGroup->m_pStaticBoxSizer = NULL;
    }

    return 0;
}


wxInt32 CBOINCTaskCtrl::DisableTaskGroupTasks( CTaskItemGroup* pGroup ) {
    unsigned int i;
    CTaskItem*   pItem = NULL;

    if (pGroup) {
        for (i=0; i < pGroup->m_Tasks.size(); i++) {
            pItem = pGroup->m_Tasks[i];
            DisableTask(pItem);
        }
    }

    return 0;
}


wxInt32 CBOINCTaskCtrl::EnableTaskGroupTasks( CTaskItemGroup* pGroup ) {
    unsigned int i;
    CTaskItem*   pItem = NULL;

    if (pGroup) {
        for (i=0; i < pGroup->m_Tasks.size(); i++) {
            pItem = pGroup->m_Tasks[i];
            EnableTask(pItem);
        }
    }

    return 0;
}


wxInt32 CBOINCTaskCtrl::DeleteTask( CTaskItemGroup* pGroup, CTaskItem* pItem ) {
    if (pItem->m_pButton) {
        pGroup->m_pStaticBoxSizer->Detach(pItem->m_pButton);
        delete pItem->m_pButton;
        pItem->m_pButton = NULL;
    }
    return 0;
}


wxInt32 CBOINCTaskCtrl::DisableTask( CTaskItem* pItem ) {
    if (pItem->m_pButton) {
        pItem->m_pButton->Disable();
    }
    return 0;
}


wxInt32 CBOINCTaskCtrl::EnableTask( CTaskItem* pItem ) {
    if (pItem->m_pButton) {
        pItem->m_pButton->Enable();
    }
    return 0;
}


wxInt32 CBOINCTaskCtrl::UpdateTask( CTaskItem* pItem, wxString strName, wxString strDescription ) {
    if (pItem->m_pButton) {
        pItem->m_strName = strName;
        pItem->m_strDescription = strDescription;

        pItem->m_pButton->SetLabel( strName );
        pItem->m_pButton->SetHelpText( strDescription );
#if wxUSE_TOOLTIPS
        pItem->m_pButton->SetToolTip( strDescription );
#endif
    }
    return 0;
}


wxInt32 CBOINCTaskCtrl::UpdateControls() {
    unsigned int        i;
    unsigned int        j;
    bool                bCreateMainSizer = false;
    CTaskItemGroup*     pGroup = NULL;
    CTaskItem*          pItem = NULL;


    bCreateMainSizer = !GetSizer();
    if (bCreateMainSizer) {
        SetAutoLayout(TRUE);
        m_pSizer = new wxBoxSizer( wxVERTICAL  );
        m_pSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    }


    // Create static boxes and sizers if they don't exist
    for (i=0; i < m_pParent->m_TaskGroups.size(); i++) {
        pGroup = m_pParent->m_TaskGroups[i];
        if (!pGroup->m_pStaticBoxSizer) {
            pGroup->m_pStaticBox = new wxStaticBox(this, wxID_ANY, pGroup->m_strName);
            pGroup->m_pStaticBoxSizer = new wxStaticBoxSizer(pGroup->m_pStaticBox, wxVERTICAL);
            m_pSizer->Add(pGroup->m_pStaticBoxSizer, 0, wxEXPAND|wxALL, 5);
        }
    }

    // Create buttons if they don't exist
    for (i=0; i < m_pParent->m_TaskGroups.size(); i++) {
        pGroup = m_pParent->m_TaskGroups[i];
        for (j=0; j < pGroup->m_Tasks.size(); j++) {
            pItem = pGroup->m_Tasks[j];
            if (!pItem->m_pButton) {
                pItem->m_pButton = new wxButton;
                pItem->m_pButton->Create(this, pItem->m_iEventID, pItem->m_strName, wxDefaultPosition, wxDefaultSize, 0);
                pItem->m_pButton->SetHelpText(pItem->m_strDescription);
#if wxUSE_TOOLTIPS
                pItem->m_pButton->SetToolTip(pItem->m_strDescription);
#endif
                pGroup->m_pStaticBoxSizer->Add(pItem->m_pButton, 0, wxEXPAND|wxALL, 5);
            }
        }
    }

    if (bCreateMainSizer) {
        SetSizer(m_pSizer);
    }

    // Force update layout and scrollbars, since nothing we do here
    // necessarily generates a size event which would do it for us.
    FitInside();

    return 0;
}


bool CBOINCTaskCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;

    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    pConfig->SetPath(strBaseConfigLocation + wxT("TaskCtrl/"));

    //WriteCustomization(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    return true;
}


bool CBOINCTaskCtrl::OnRestoreState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;

    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    pConfig->SetPath(strBaseConfigLocation + wxT("TaskCtrl/"));

    //ReadCustomization(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    return true;
}


const char *BOINC_RCSID_125ef3d14d = "$Id: BOINCTaskCtrl.cpp 13804 2007-10-09 11:35:47Z fthomas $";
