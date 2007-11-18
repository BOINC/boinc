// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2007 University of California
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
#pragma implementation "common/wxNotifyIcon.h"
#endif

#include "common/wxNotifyIcon.h"

DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_CONTEXT_MENU )
DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_SELECT )
DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_KEY_SELECT )
DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_BALLOON_SHOW )
DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_BALLOON_HIDE )
DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_BALLOON_TIMEOUT )
DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_BALLOON_USERCLICK )
DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_POPUP_SHOW )
DEFINE_EVENT_TYPE( wxEVT_NOTIFYICON_POPUP_HIDE )

// Catch wxTaskBarIcon events and re-emit them as wxNotifyIconEvent.
bool wxNotifyIconBase::ProcessEvent(wxEvent& event) {

    wxEventType eventType = event.GetEventType();

    if (eventType == wxEVT_TASKBAR_MOVE
        || eventType == wxEVT_TASKBAR_LEFT_DOWN
        || eventType == wxEVT_TASKBAR_LEFT_UP
        || eventType == wxEVT_TASKBAR_RIGHT_DOWN
        || eventType == wxEVT_TASKBAR_RIGHT_UP
        || eventType == wxEVT_TASKBAR_LEFT_DCLICK
        || eventType == wxEVT_TASKBAR_RIGHT_DCLICK)
    {
        //wxASSERT(this->IsKindOf(wxNotifyIcon));
        wxNotifyIcon* notify = wxDynamicCast(this, wxNotifyIcon);

        wxNotifyIconEvent newEvent(eventType, notify);
        return wxTaskBarIconBase::ProcessEvent(newEvent);
    } else {
        return wxTaskBarIconBase::ProcessEvent(event);
    }
}