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
#ifndef _PREFLOCATIONMANAGER_H_
#define _PREFLOCATIONMANAGER_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "PrefLocationManager.cpp"
#endif


#include "prefs.h"
#include "PrefTreeBook.h"

#define PREF_DLG_MARGIN 4

class CPrefLocationManager : public wxDialog {

	DECLARE_DYNAMIC_CLASS(CPrefLocationManager)
    DECLARE_EVENT_TABLE()

public:
	CPrefLocationManager(wxWindow* parent=NULL);
	virtual ~CPrefLocationManager();

	void OnOK(wxCommandEvent& event);

protected:
    bool SaveState();
    bool RestoreState();

private:
    GLOBAL_PREFS        prefs;

    CPrefTreeBook*      m_treeBook;

    wxButton*           m_buttonOkay;
    wxButton*           m_buttonCancel;
    wxButton*           m_buttonHelp;
};

#endif // _PREFLOCATIONMANAGER_H_

