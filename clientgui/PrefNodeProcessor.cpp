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
#pragma implementation "PrefNodeProcessor.h"
#endif

#include "stdwx.h"
#include "prefs.h"
#include "ValidateBool.h"
#include "ValidateNumber.h"
#include "PrefNodeBase.h"
#include "PrefNodeProcessor.h"


IMPLEMENT_DYNAMIC_CLASS(CPrefNodeProcessor, CPrefNodeBase)


CPrefNodeProcessor::CPrefNodeProcessor(wxWindow* parent, GLOBAL_PREFS* preferences)
: CPrefNodeBase(parent, preferences) {

    CPrefGroup* limits = AddGroup(_("CPU Limits"));

    limits->AddPreference(new CPrefValueText(this,
        _("cpu_usage_limit"),
        _("Use no more than"),
        _("% of processor time"),
        _("BOINC has a built-in throttle that can limit the CPU time used. "
        "This is commonly used to reduce the risk of overheating. The throttle "
        "is coarse-grained; you may see a sawtooth profile if you graph "
        "CPU usage. Default 100%."),
        CValidateNumber<double>(&m_preferences->cpu_usage_limit, 0, 100))
    );

    limits->AddPreference(new CPrefValueText(this,
        _("max_cpus"),
        _("On multiprocessor systems, at most use"),
        _("processors"),
        _("This limit specifies the number of processors or individual "
        "processor cores that BOINC will use. Many projects will run one "
        "process on each permitted core. Default 16."),
        CValidateNumber<int>(&m_preferences->max_cpus, 1, 0xFFFF, false))
    );

    CPrefGroup* restrict = AddGroup(_("Processing Restrictions"));

    // WARNING! Prompt is opposite sense.
    CPrefValueBase* run_if_user_active = new CPrefValueBool(this,
        _("run_if_user_active"),
        _("Suspend while computer is in use"),
        _("Use this option if you want BOINC to behave like a screensaver, "
        "only working when you are away from your computer. Default false."),
        CValidateBoolInverse(&m_preferences->run_if_user_active));

    m_idleTime = new CPrefValueText(this,
        _("idle_time_to_run"),
        _("Resume if computer is idle for"),
        _("minutes"),
        _("This option prevents BOINC from starting if you are only "
        "away from your computer briefly. Default 3 minutes."),
        CValidateNumber<double>(&m_preferences->idle_time_to_run));



    run_if_user_active->Connect(
        wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(CPrefNodeProcessor::OnRunIdleChanged),
        NULL,
        this
        );

    m_idleTime->Enable(! m_preferences->run_if_user_active);

    restrict->AddPreference(run_if_user_active);
    restrict->AddPreference(m_idleTime);

    // WARNING! Prompt is opposite sense.
    restrict->AddPreference(new CPrefValueBool(this,
        _("run_on_batteries"),
        _("Suspend while computer is running on batteries"),
        _("Generally, laptop users don't want BOINC to run while "
        "on battery power. Default true."),
        CValidateBoolInverse(&m_preferences->run_on_batteries))
    );
}


void CPrefNodeProcessor::OnRunIdleChanged(wxCommandEvent& event) {

    m_idleTime->Enable(event.IsChecked());
}

