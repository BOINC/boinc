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
#ifndef _PREFTREEBOOK_H_
#define _PREFTREEBOOK_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "PrefTreeBook.cpp"
#endif

#include "PrefNodeBase.h"
#include "wx/treectrl.h"

// Data class to attach NodeType to TreeItem
class CPrefNodeType : public wxTreeItemData {
public:
    CPrefNodeType(PrefNodeType nodeType) : wxTreeItemData(), m_nodeType(nodeType) {};
    inline PrefNodeType GetNodeType() { return m_nodeType; };

private:
    PrefNodeType m_nodeType;
};


// Event class for extended help events.
class PrefHelpEvent: public wxCommandEvent {
public:
    PrefHelpEvent(wxEventType commandType = wxEVT_NULL, int id = 0) : wxCommandEvent(commandType, id) {}
    PrefHelpEvent(const PrefHelpEvent& event) : wxCommandEvent(event), m_trigger(event.m_trigger) {}

    enum Trigger {
        Focus,
        Mouse,
    };

    Trigger GetTrigger() { return m_trigger; }
    void SetTrigger(Trigger trigger) { m_trigger = trigger; }

    // required for sending with wxPostEvent()
    wxEvent* Clone() const { return new PrefHelpEvent(*this); }

private:
    Trigger   m_trigger;
};

DECLARE_EVENT_TYPE(PREF_EVT_HELP_CMD, -1)

typedef void (wxEvtHandler::*PrefHelpEventFunction)(PrefHelpEvent&);

#define PREF_EVT_HELP(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY(PREF_EVT_HELP_CMD, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
    wxStaticCastEvent(PrefHelpEventFunction, &fn), (wxObject *) NULL),




// Not using wxTreeBook for extra flexibility.

class CPrefTreeBook : public wxPanel {

	DECLARE_DYNAMIC_CLASS(CPrefTreeBook)
    DECLARE_EVENT_TABLE()

public:
	CPrefTreeBook(wxWindow* parent = NULL);
	virtual ~CPrefTreeBook();

    void SavePreferences();

protected:
    bool SaveState();
    bool RestoreState();
    void OnTreeSelectionChanging(wxTreeEvent& event);
    void OnHelp(PrefHelpEvent& event);
    void ShowHelpText(wxWindow* source);

private:
    void OnTimer(wxTimerEvent& event);
    bool Find(const wxTreeItemId& root, wxTreeItemId& result, PrefNodeType nodeType);

    wxTreeCtrl*     m_tree;
    wxTextCtrl*     m_helpTextCtrl;
    wxWindow*       m_helpSourceFocus;
    wxWindow*       m_helpSourceMouse;
    wxWindow*       m_helpSource;
    wxPoint         m_mouse;

    wxTimer*        m_helpTimer;
    wxWindow*       m_content;
    GLOBAL_PREFS    m_preferences;

};

#endif // _PREFTREEBOOK_H_

