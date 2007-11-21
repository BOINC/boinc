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
#pragma implementation "PrefNodeNetworkTimes.h"
#endif

#include "stdwx.h"
#include "prefs.h"
#include "ValidateNumber.h"
#include "PrefNodeBase.h"
#include "PrefNodeNetworkTimes.h"


IMPLEMENT_DYNAMIC_CLASS(CPrefNodeNetworkTimes, CPrefNodeBase)

BEGIN_EVENT_TABLE(CPrefNodeNetworkTimes, CPrefNodeBase)
    EVT_BUTTON(ID_PREF_COPY_TIMES, CPrefNodeNetworkTimes::OnCopyTimes)
END_EVENT_TABLE()

CPrefNodeNetworkTimes::CPrefNodeNetworkTimes(wxWindow* parent, GLOBAL_PREFS* preferences)
: CPrefNodeBase(parent, preferences), m_preferences(preferences) {

    CPrefGroup* copy = AddGroup(_("Copy Settings"));

    copy->AddPreference(new CPrefValueButton(this, wxEmptyString,
        _("Copy the network time settings from the processor time settings."),
        _("Copy"),
        _("If you want to use the same settings for networking and processing, copy them here."),
        ID_PREF_COPY_TIMES
        ));

    CPrefGroup* restrict = AddGroup(_("Time Restrictions"));
    m_time = new CPrefValueTime(this,
        _("Allow BOINC to perform network activity during these times:"),
        _("BOINC will only perform normal network activity at the specified times. Some user "
        "actions override this setting. If you set the start time after "
        "the end time, then BOINC will perform network activity during the "
        "night. Times must be specified in HH:MM format. Default: no restrictions."),
        &preferences->net_times);

    restrict->AddPreference(m_time);
        
    CPrefGroup* sched = AddGroup(_("Weekly Schedule"));

    m_week = new CPrefValueWeek(this,
        _("Override specific days with these times:"),
        _("Permitted values are 'Always', 'Never', or a time period in the "
        "form HH:MM - HH:MM. Default: no restrictions."),
        &preferences->net_times);

    sched->AddPreference(m_week);

    m_time->Connect(
        PREF_EVT_CMD_UPDATE,
        wxCommandEventHandler(CPrefNodeBase::CPrefValueWeek::OnUpdateUI),
        NULL,
        m_week
        );
}

// Handler for copy button.
void CPrefNodeNetworkTimes::OnCopyTimes(wxCommandEvent& WXUNUSED(event)) {

    m_preferences->net_times = m_preferences->cpu_times;

    m_time->Update();
    m_week->Update();
}