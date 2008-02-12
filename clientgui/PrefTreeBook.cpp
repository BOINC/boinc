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
//
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "PrefTreeBook.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "PrefTreeBook.h"
#include "PrefNodeBase.h"
#include "PrefNodeGeneral.h"
#include "PrefNodeProcessor.h"
#include "Events.h"

DEFINE_EVENT_TYPE(PREF_EVT_HELP_CMD)

IMPLEMENT_DYNAMIC_CLASS(CPrefTreeBook, wxPanel)

BEGIN_EVENT_TABLE(CPrefTreeBook, wxPanel)
    EVT_TIMER(wxID_ANY, CPrefTreeBook::OnTimer)
    EVT_TREE_SEL_CHANGING(wxID_ANY, CPrefTreeBook::OnTreeSelectionChanging)
    PREF_EVT_HELP(wxID_ANY, CPrefTreeBook::OnHelp)
END_EVENT_TABLE()


CPrefTreeBook::CPrefTreeBook(wxWindow* parent) : wxPanel(parent) {

    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);

    pDoc->rpc.get_global_prefs_working_struct(m_preferences);

    m_helpSource = 0;
    m_helpSourceFocus = 0;
    m_helpSourceMouse = 0;

    // Treeview preferences - ratio 1:2
    wxBoxSizer* contentRow = new wxBoxSizer(wxHORIZONTAL);

    // Content placeholder
    m_content = new wxPanel(this);

    m_helpTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxSize(-1, 75), wxTE_MULTILINE | wxTE_READONLY);

    m_helpTextCtrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    m_helpTextCtrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

    wxBoxSizer* contentBox = new wxBoxSizer(wxVERTICAL);

    contentBox->Add(m_content, 4, wxEXPAND);
    contentBox->Add(m_helpTextCtrl, 1, wxEXPAND);

    m_tree = new wxTreeCtrl(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxSize(200, -1),
        wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT
    );
    wxTreeItemId root = m_tree->AddRoot(_("Preferences"));
    m_tree->AppendItem(root, _("Presets"), -1, -1, new CPrefNodeType(Presets));
    m_tree->AppendItem(root, _("General Options"), -1, -1, new CPrefNodeType(General));
    wxTreeItemId proc = m_tree->AppendItem(root, _("Processor Usage"), -1, -1, new CPrefNodeType(Processor));
    m_tree->AppendItem(proc, _("Custom Times"), -1, -1, new CPrefNodeType(ProcessorTimes));
    wxTreeItemId net = m_tree->AppendItem(root, _("Network Usage"), -1, -1, new CPrefNodeType(Network));
    m_tree->AppendItem(net, _("Custom Times"), -1, -1, new CPrefNodeType(NetworkTimes));
    m_tree->AppendItem(root, _("Memory Usage"), -1, -1, new CPrefNodeType(Memory));
    m_tree->AppendItem(root, _("Disk Usage"), -1, -1, new CPrefNodeType(Disk));


    contentRow->Add(m_tree, 1, wxALL | wxEXPAND, 4);
    contentRow->Add(contentBox, 2, wxALL | wxEXPAND, 4);

    SetSizerAndFit(contentRow);

	RestoreState();

    m_helpTimer = new wxTimer(this);
    m_helpTimer->Start(250);
}


CPrefTreeBook::~CPrefTreeBook() {
    m_helpTimer->Stop();
	SaveState();
}


void CPrefTreeBook::OnTimer(wxTimerEvent& WXUNUSED(event)) {

    wxWindow* win = 0;
    wxPoint p = wxGetMousePosition();
    if (p != m_mouse) {
        m_mouse = p;
        wxRect r = m_content->GetScreenRect();

        if (r.Contains(p)) {
            if (m_content->IsKindOf(CLASSINFO(CPrefNodeBase))) {
                CPrefNodeBase* page = (CPrefNodeBase*) m_content;
                win = page->GetHelpAtPoint(p);
            }
        }
        PrefHelpEvent e(PREF_EVT_HELP_CMD, GetId());
        e.SetTrigger(PrefHelpEvent::Mouse);
        e.SetEventObject(win);
        GetEventHandler()->ProcessEvent(e);
    }
}


void CPrefTreeBook::OnTreeSelectionChanging(wxTreeEvent& event) {

    if (! Validate()) {
        event.Veto();
    }
    else {
        TransferDataFromWindow();

        // Create new page
        wxTreeItemId id = event.GetItem();
        if (id.IsOk()) {
            Freeze();
            CPrefNodeType* nodeType = (CPrefNodeType*) m_tree->GetItemData(id);
            CPrefNodeBase* page = CPrefNodeBase::Create(nodeType->GetNodeType(), this, &m_preferences);

            if (page && m_content) {
                // Swap pages
                wxSizer* s = m_content->GetContainingSizer();
                s->Replace(m_content, page);
                m_content->Destroy();
                TransferDataToWindow();
                s->Layout();

                m_content = page;
            }
            Thaw();
        }

        PrefHelpEvent e(PREF_EVT_HELP_CMD, GetId());
        e.SetTrigger(PrefHelpEvent::Focus);
        e.SetEventObject(0);
        GetEventHandler()->ProcessEvent(e);
    }
}


// Use a stack for help text, to allow mouse movement to override help derived from focus.
void CPrefTreeBook::OnHelp(PrefHelpEvent& event) {

    wxObject* eo = event.GetEventObject();
    wxWindow* source = 0;

    if (eo) {
        wxASSERT(eo->IsKindOf(CLASSINFO(wxWindow)));
        source = wxDynamicCast(eo, wxWindow);
    }

    if (event.GetTrigger() == PrefHelpEvent::Focus) {
        m_helpSourceFocus = source;
        m_helpSourceMouse = 0;
        ShowHelpText(source);        
    } else {
        if (!source && m_helpSourceFocus) {
            ShowHelpText(m_helpSourceFocus);
        } else {
            ShowHelpText(source);
        }
        m_helpSourceMouse = source;
    }
}


void CPrefTreeBook::ShowHelpText(wxWindow* source) {

    if (m_helpSource != source) {
        if (source) {
            m_helpTextCtrl->SetValue(source->GetHelpText());
        } else {
            m_helpTextCtrl->SetValue(wxEmptyString);
        }
    }
    m_helpSource = source;
    m_mouse = wxGetMousePosition();
}

bool CPrefTreeBook::SaveState() {

    wxString        strBaseConfigLocation = wxString(wxT("/PrefTreeBook/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);

    wxASSERT(pConfig);
	if (!pConfig) return false;

    CPrefNodeType* n = (CPrefNodeType*) m_tree->GetItemData(m_tree->GetSelection());

	pConfig->SetPath(strBaseConfigLocation);
	pConfig->Write(wxT("CurrentNode"), n->GetNodeType());

	return true;
}


bool CPrefTreeBook::RestoreState() {

    wxString        strBaseConfigLocation = wxString(wxT("/PrefTreeBook/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);
	int				node;

	wxASSERT(pConfig);
    if (!pConfig) return false;

	pConfig->SetPath(strBaseConfigLocation);

	pConfig->Read(wxT("CurrentNode"), &node, General);
	
    PrefNodeType selNode = (PrefNodeType) node;

    wxTreeItemId root = m_tree->GetRootItem();
    wxTreeItemId result;

    if (Find(root, result, selNode)) {
        m_tree->SelectItem(result);
    }

    m_tree->SetFocus();

	return true;
}


// Recursively search for a node of the specified type.
bool CPrefTreeBook::Find(const wxTreeItemId& root, wxTreeItemId& result, PrefNodeType nodeType) {

    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_tree->GetFirstChild(root, cookie);
    while (child.IsOk()){

        CPrefNodeType* n = (CPrefNodeType*) m_tree->GetItemData(child);

        if (n->GetNodeType() == nodeType) {
            result = child;
            return true;
        }
        // Depth first traversal
        if (Find(child, result, nodeType)) {
            return true;
        }
        child = m_tree->GetNextChild(root, cookie);
    }
    return false;
}

void CPrefTreeBook::SavePreferences() {

    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);

	pDoc->rpc.set_global_prefs_override_struct(m_preferences);		
	pDoc->rpc.read_global_prefs_override();

}