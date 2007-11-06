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
#pragma implementation "PrefNodeNetwork.h"
#endif

#include "stdwx.h"
#include "prefs.h"
#include "ValidateNumber.h"
#include "PrefNodeBase.h"
#include "PrefNodeNetwork.h"


IMPLEMENT_DYNAMIC_CLASS(CPrefNodeNetwork, CPrefNodeBase)

CPrefNodeNetwork::CPrefNodeNetwork(wxWindow* parent, GLOBAL_PREFS* preferences)
: CPrefNodeBase(parent, preferences) {

    CPrefGroup* connect = AddGroup(_("Connection"));

    connect->AddPreference(new CPrefValueBool(this,
        _("confirm_before_connecting"),
        _("Confirm before connecting to Internet"),
        _("BOINC will only try and get an Internet connection when it needs one. "
        "Default true."),
        CValidateBool(&m_preferences->confirm_before_connecting))
    );

    connect->AddPreference(new CPrefValueBool(this,
        _("hangup_if_dialed"),
        _("Disconnect when done"),
        _("BOINC will only disconnect if it initiated the Internet connection. "
        "Default false."),
        CValidateBool(&m_preferences->hangup_if_dialed))
    );

    connect->AddPreference(new CPrefValueText(this,
        _("work_buf_min_days"),
        _("Connect about every"),
        _("days"),
        _("BOINC will use this as a hint for buffering work between connections. "
        "BOINC will still use the Internet more frequently if a connection "
        "is available. Default 0.1 days."),
        CValidateNumber<double>(&m_preferences->work_buf_min_days))
    );

    CPrefGroup* limits = AddGroup(_("Bandwidth Limits"));

    limits->AddPreference(new CPrefValueText(this,
        _("max_bytes_sec_up"),
        _("Maximum upload rate"),
        _("Kbytes/sec"),
        _("Zero means upload is unrestricted. Default unrestricted."),
        CValidateNumber<double>(&m_preferences->max_bytes_sec_up))
    );

    limits->AddPreference(new CPrefValueText(this,
        _("max_bytes_sec_down"),
        _("Maximum download rate"),
        _("Kbytes/sec"),
        _("Zero means download is unrestricted. Default unrestricted."),
        CValidateNumber<double>(&m_preferences->max_bytes_sec_down))
    );

    CPrefGroup* errors = AddGroup(_("Error Checking"));

    errors->AddPreference(new CPrefValueBool(this,
        _("dont_verify_images"),
        _("Skip image file verification"),
        _("Some dialup Internet Service Providers compress image downloads on the fly. "
        "If you can't use a better ISP, use this option to ignore the modified images "
        "until you can switch to a better ISP. Default false."),
        CValidateBool(&m_preferences->dont_verify_images))
    );
}




