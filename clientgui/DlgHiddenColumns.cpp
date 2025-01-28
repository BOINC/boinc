// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "DlgHiddenColumns.h"
#endif

#include "stdwx.h"
#include "util.h"
#include "DlgHiddenColumns.h"
#include "BOINCGUIApp.h"
#include "Events.h"
#include "error_numbers.h"
#include "gui_rpc_client.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCBaseView.h"
#include "BOINCListCtrl.h"

const int dlgHiddenColumnsInitialWidth = 480;
const int dlgHiddenColumnsInitialHeight = 480;
const int dlgHiddenColumnsMinWidth = 400;
const int dlgHiddenColumnsMinHeight = 400;

IMPLEMENT_DYNAMIC_CLASS(CDlgHiddenColumns, wxDialog)

BEGIN_EVENT_TABLE(CDlgHiddenColumns, wxDialog)
    EVT_SIZE(CDlgHiddenColumns::OnSize)
    EVT_BUTTON(wxID_OK,CDlgHiddenColumns::OnOK)
    EVT_BUTTON(ID_DEFAULTSBTN,CDlgHiddenColumns::OnSetDefaults)
    EVT_CHECKBOX(wxID_ANY, CDlgHiddenColumns::OnCheckboxClick )


END_EVENT_TABLE()

/* Constructor */
CDlgHiddenColumns::CDlgHiddenColumns(wxWindow* parent) :
    wxDialog( parent, ID_ANYDIALOG, wxEmptyString, wxDefaultPosition,
                wxSize(dlgHiddenColumnsInitialWidth, dlgHiddenColumnsInitialHeight),
                wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER
            ) {

    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    wxString title;
    title.Printf(
        _("%s Column Selection"),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );

    SetTitle(title);

    SetSizeHints(dlgHiddenColumnsMinWidth, dlgHiddenColumnsMinHeight);
    SetExtraStyle( GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );
    m_headingSizer = new wxGridSizer( 1 );

    m_headingText.Printf(
        _("Select which columns %s should show."),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );

    m_heading = new wxStaticText(this, wxID_ANY, m_headingText);

    m_headingSizer->Add(m_heading, 1, wxLEFT | wxRIGHT, 25);

    bSizer1->AddSpacer(7);
    bSizer1->Add( m_headingSizer, 0, wxEXPAND | wxALL, 5 );
    bSizer1->AddSpacer(7);

    m_scrolledWindow = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    m_scrolledWindow->SetScrollRate( 5, 5 );

    m_scrolledSizer = new wxBoxSizer(wxVERTICAL);

    CreateCheckboxes();

    bSizer1->Add( m_scrolledWindow, 1, wxEXPAND | wxALL, 5 );

    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    m_btnOK = new wxButton( this, wxID_OK, _("&Save"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnOK->SetToolTip( _("Save all values and close the dialog") );
    buttonSizer->Add( m_btnOK, 0, wxALL, 5 );

    wxButton* btnDefaults = new wxButton( this, ID_DEFAULTSBTN, _("Defaults"), wxDefaultPosition, wxDefaultSize, 0 );
    btnDefaults->SetToolTip( _("Restore default settings") );
    buttonSizer->Add( btnDefaults, 0, wxALL, 5 );

    wxButton* btnCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    btnCancel->SetToolTip( _("Close the dialog without saving") );
    buttonSizer->Add( btnCancel, 0, wxALL, 5 );

    btnCancel->SetDefault();
    bSizer1->Add( buttonSizer, 0, wxALIGN_RIGHT | wxALL, 15 );

    SetSizer( bSizer1 );

    RestoreState();
    Layout();
    Center( wxBOTH );

#if defined(__WXMSW__) || defined(__WXGTK__)
    SetDoubleBuffered(true);
#endif
}

// destructor
CDlgHiddenColumns::~CDlgHiddenColumns() {
    unsigned int i, count;

    SaveState();

    count = m_checkbox_list.size();
    for (i=0; i<count; ++i) {
        std::vector <wxCheckBox*> *checkbox_list = m_checkbox_list[i];
        checkbox_list->clear();
        delete checkbox_list;
    }
    m_checkbox_list.clear();
    m_pBOINCBaseView.clear();
}


void CDlgHiddenColumns::CreateCheckboxes() {
    int i, stdCount, actualCount;
    bool val;
    CAdvancedFrame* pFrame = (CAdvancedFrame*)GetParent();
    wxASSERT(wxDynamicCast(pFrame, CAdvancedFrame));

    wxNotebook* pNotebook = pFrame->GetNotebook();
    wxASSERT(pNotebook);

    actualCount = m_checkbox_list.size();
    for (i=0; i<actualCount; ++i) {
        std::vector <wxCheckBox*> *checkbox_list = m_checkbox_list[i];
        checkbox_list->clear();
        delete checkbox_list;
    }
    m_checkbox_list.clear();

    wxWindow*       pwndNotebookPage = NULL;
    CBOINCBaseView* pView = NULL;
    long            iIndex;
    long            iPageCount;

    iPageCount = (long)pNotebook->GetPageCount();

    for (iIndex = 0; iIndex < iPageCount; iIndex++) {
        pwndNotebookPage = pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(pView);

        CBOINCListCtrl* listPane = pView->GetListCtrl();
        if (!listPane) continue;

        m_pBOINCBaseView.push_back(pView);

        std::vector <wxCheckBox*> *checkbox_list = new std::vector <wxCheckBox*>;

        wxStaticBox* tabStaticBox = new wxStaticBox(m_scrolledWindow, -1, pView->GetViewDisplayName());
        wxStaticBoxSizer* tabStaticBoxSizer = new wxStaticBoxSizer( tabStaticBox, wxVERTICAL );
        wxGridSizer* checkboxSizer = new wxGridSizer(2, wxSize(0,3));

        CBOINCListCtrl* listCtrl = pView->GetListCtrl();
        wxInt32 iShownColumnCount = listCtrl->GetColumnCount();
        wxArrayInt aOrder(iShownColumnCount);

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
        aOrder = listCtrl->GetColumnsOrder();
#else
        for (i = 0; i < iShownColumnCount; ++i) {
            aOrder[i] = i;
        }
#endif

        stdCount = pView->m_aStdColNameOrder->GetCount();

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
        // Create checkboxes for shown columns in current column order
        for (i=0; i<iShownColumnCount; ++i) {
            wxString columnLabel = pView->m_aStdColNameOrder->Item(pView->m_iColumnIndexToColumnID[aOrder[i]]);
            wxCheckBox* ckbox = new wxCheckBox(tabStaticBox, wxID_ANY, columnLabel);
            checkboxSizer->Add(ckbox, 0, wxLEFT, 25);
            val = false;
            ckbox->SetValue(true);
            checkbox_list->push_back(ckbox);
        }

        // Create checkboxes for hidden columns
        for (i=0; i<stdCount; ++i) {
            bool found = false;
            for (int j = 0; j < iShownColumnCount; ++j) {
                if (pView->m_iColumnIndexToColumnID[aOrder[j]] == i) {
                    found = true;
                    break;
                }
            }
            if (found) continue;

            wxString columnLabel = pView->m_aStdColNameOrder->Item(i);
            wxCheckBox* ckbox = new wxCheckBox(tabStaticBox, wxID_ANY, columnLabel);
            checkboxSizer->Add(ckbox, 0, wxLEFT, 25);
            ckbox->SetValue(false);
            checkbox_list->push_back(ckbox);
        }
#else
        for (i=0; i<stdCount; ++i) {
            wxString columnLabel = pView->m_aStdColNameOrder->Item(i);
            wxCheckBox* ckbox = new wxCheckBox(tabStaticBox, wxID_ANY, columnLabel);
            checkboxSizer->Add(ckbox, 0, wxLEFT, 25);
            val = false;
            if (pView->m_iColumnIDToColumnIndex[i] >= 0) val = true;
            ckbox->SetValue(val);
            checkbox_list->push_back(ckbox);
        }
#endif

        m_checkbox_list.push_back(checkbox_list);

        tabStaticBoxSizer->Add(checkboxSizer, 0, wxEXPAND, 1 );
        m_scrolledSizer->Add(tabStaticBoxSizer, 0, wxEXPAND, 1 );
    }
    m_scrolledWindow->SetSizer( m_scrolledSizer );
    m_scrolledWindow->Layout();
    m_scrolledSizer->Fit( m_scrolledWindow );
}


/* saves dialog size and (on Mac) position */
bool CDlgHiddenColumns::SaveState() {
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);
    if (!pConfig) return false;

    pConfig->SetPath("/DlgHiddenColumns/");
    pConfig->Write(wxT("Width"), GetSize().GetWidth());
    pConfig->Write(wxT("Height"), GetSize().GetHeight());

    pConfig->Flush();

    return true;
}

/* restores former dialog size and (on Mac) position */
bool CDlgHiddenColumns::RestoreState() {
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    int                iWidth, iHeight;

    wxASSERT(pConfig);
    if (!pConfig) return false;

    pConfig->SetPath("/DlgHiddenColumns/");

    pConfig->Read(wxT("Width"), &iWidth, dlgHiddenColumnsInitialWidth);
    pConfig->Read(wxT("Height"), &iHeight, dlgHiddenColumnsInitialHeight);

    // Guard against a rare situation where registry values are zero
    if ((iWidth < 50) && (iWidth != wxDefaultCoord)) iWidth = dlgHiddenColumnsInitialWidth;
    if ((iHeight < 50) && (iHeight != wxDefaultCoord)) iHeight = dlgHiddenColumnsInitialHeight;

    // Set size to saved values or defaults if no saved values
    SetSize(std::max(iWidth, dlgHiddenColumnsMinWidth), std::max(iHeight, dlgHiddenColumnsMinHeight));

    return true;
}


void CDlgHiddenColumns::OnSize(wxSizeEvent& event) {
    m_heading->SetLabel(m_headingText);
    m_heading->Wrap(m_headingSizer->GetSize().GetWidth()-50);
    m_headingSizer->Fit(m_heading);
    Layout();
    SaveState();
    Refresh();

    event.Skip();
}


void CDlgHiddenColumns::OnCheckboxClick(wxCommandEvent& event){
    bool bAllOffInGroup, bEnableOK = true;

    size_t actualCount = m_checkbox_list.size();
    for (size_t i=0; i<actualCount; ++i) {
        std::vector <wxCheckBox*> *checkbox_list = m_checkbox_list[i];
        bAllOffInGroup = true;
        for (size_t j=0; j<checkbox_list->size(); ++j) {
            wxCheckBox* ckbox = (*checkbox_list)[j];
            if (ckbox->GetValue()) {
                bAllOffInGroup = false;
                break;
            }
        }
        if (bAllOffInGroup) {
            bEnableOK =  false;
            break;
        }
    }
    m_btnOK->Enable(bEnableOK);

    event.Skip();
}


void CDlgHiddenColumns::OnOK(wxCommandEvent& event) {
    size_t actualCount = m_checkbox_list.size();
    wxASSERT (m_pBOINCBaseView.size() == actualCount);

    for (size_t i=0; i<actualCount; ++i) {
        CBOINCBaseView* pView = m_pBOINCBaseView[i];
        std::vector <wxCheckBox*> *checkbox_list = m_checkbox_list[i];
        wxArrayString orderArray;
        for (size_t j=0; j<checkbox_list->size(); ++j) {
            wxCheckBox* ckbox = (*checkbox_list)[j];
            if (ckbox->GetValue()) {
                wxString name = ckbox->GetLabel();
                orderArray.Add(name);
            }
        }

        CBOINCListCtrl* listPane = pView->GetListCtrl();
        listPane->SetListColumnOrder(orderArray);

        // Write the new column configuration to the registry
        wxConfigBase* pConfig = wxConfigBase::Get(false);
        pConfig->SetPath(wxT("/") + pView->GetViewName());
        listPane->OnSaveState(pConfig);
    }

    event.Skip();
}


void CDlgHiddenColumns::OnSetDefaults(wxCommandEvent& ) {
    wxMessageDialog* dlg = new wxMessageDialog(this, _("Are you sure you want to reset all list columns to the default configurations?"),
                    _("Confirm defaults"), wxOK|wxCANCEL|wxCENTRE, wxDefaultPosition);

    if (dlg->ShowModal() != wxID_OK) return;

    int count = m_pBOINCBaseView.size();
    for (int i=0; i<count; ++i) {
        CBOINCBaseView* pView = m_pBOINCBaseView[i];
        CBOINCListCtrl* listPane = pView->GetListCtrl();

        listPane->SetListColumnOrder(*(pView->m_aStdColNameOrder));
        listPane->SetDefaultColumnDisplay();

        // Write the new column configuration to the registry
        wxConfigBase* pConfig = wxConfigBase::Get(false);
        pConfig->SetPath(wxT("/") + pView->GetViewName());
        listPane->OnSaveState(pConfig);
    }

    EndModal(0);
}
