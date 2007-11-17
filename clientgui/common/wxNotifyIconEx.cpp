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