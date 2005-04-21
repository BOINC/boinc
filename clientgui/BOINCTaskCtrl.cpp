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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCTaskCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCTaskCtrl.h"


IMPLEMENT_DYNAMIC_CLASS(CBOINCTaskCtrl, wxPanel)


CBOINCTaskCtrl::CBOINCTaskCtrl() {}


CBOINCTaskCtrl::CBOINCTaskCtrl(CBOINCBaseView* pView, wxWindowID iTaskWindowID, wxInt32 iTaskWindowFlags) :
    wxPanel(pView, iTaskWindowID, wxDefaultPosition, wxSize(175, -1), iTaskWindowFlags)
{
    m_pParent = pView;
    m_pBoxSizer = NULL;
}


CBOINCTaskCtrl::~CBOINCTaskCtrl() {}



wxInt32 CBOINCTaskCtrl::UpdateControls() {
    unsigned int        i;
    unsigned int        j;
    bool                bCreateMainSizer = false;
    CTaskItemGroup*     pGroup = NULL;
    CTaskItem*          pItem = NULL;

    bCreateMainSizer = (!GetSizer());
    if (bCreateMainSizer) { 
        m_pBoxSizer = new wxBoxSizer(wxVERTICAL);
        m_pBoxSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    }

    // Create static boxes and sizers if they don't exist
    for (i=0; i < m_pParent->m_TaskGroups.size(); i++) {
        pGroup = m_pParent->m_TaskGroups[i];
        if (!pGroup->m_pStaticBox) {
            pGroup->m_pStaticBox = new wxStaticBox(this, wxID_ANY, pGroup->m_strName);
            pGroup->m_pStaticBoxSizer = new wxStaticBoxSizer(pGroup->m_pStaticBox, wxVERTICAL);
            m_pBoxSizer->Add(pGroup->m_pStaticBoxSizer, 0, wxEXPAND|wxALL, 5);
            m_pBoxSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
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
                pItem->m_pButton->SetToolTip(pItem->m_strDescription);
                pGroup->m_pStaticBoxSizer->Add(pItem->m_pButton, 0, wxEXPAND|wxALL, 5);
            }
        }
    }

    if (bCreateMainSizer) {
        SetSizerAndFit(m_pBoxSizer);
    } else {
        Fit();
    }

    return 0;
}


bool CBOINCTaskCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;

    wxASSERT(NULL != pConfig);


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

    wxASSERT(NULL != pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    pConfig->SetPath(strBaseConfigLocation + wxT("TaskCtrl/"));

    //ReadCustomization(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    return true;
}


const char *BOINC_RCSID_125ef3d14d = "$Id$";
