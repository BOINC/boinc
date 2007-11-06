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
#pragma implementation "PrefNodeDisk.h"
#endif

#include "stdwx.h"
#include "prefs.h"
#include "ValidateNumber.h"
#include "PrefNodeBase.h"
#include "PrefNodeDisk.h"


IMPLEMENT_DYNAMIC_CLASS(CPrefNodeDisk, CPrefNodeBase)

CPrefNodeDisk::CPrefNodeDisk(wxWindow* parent, GLOBAL_PREFS* preferences)
: CPrefNodeBase(parent, preferences) {

    CPrefGroup* quotas = AddGroup(_("Disk Quotas"));

    quotas->AddPreference(new CPrefValueText(this,
        _("disk_max_used_gb"),
        _("Use no more than"),
        _("Gigabytes of disk space"),
        _("This option is combined with the other disk restrictions to "
        "determine the maximum space BOINC may use for project application data. "
        "Default 10 Gigabytes."),
        CValidateNumber<double>(&m_preferences->disk_max_used_gb))
    );

    quotas->AddPreference(new CPrefValueText(this,
        _("disk_min_free_gb"),
        _("Leave at least"),
        _("Gigabytes of disk space free"),
        _("This option is combined with the other disk restrictions to "
        "determine the maximum space BOINC may use for project application data. "
        "Default 50%."),
        CValidateNumber<double>(&m_preferences->disk_min_free_gb))
    );

    quotas->AddPreference(new CPrefValueText(this,
        _("disk_max_used_pct"),
        _("Use no more than"),
        _("% of total disk space"),
        _("This option is combined with the other disk restrictions to "
        "determine the maximum space BOINC may use for project application data. "
        "Default 0.1 Gigabytes."),
        CValidateNumber<double>(&m_preferences->disk_max_used_pct))
    );

    CPrefGroup* access = AddGroup(_("Disk Access"));

    access->AddPreference(new CPrefValueText(this,
        _("disk_interval"),
        _("Write to disk at most every"),
        _("seconds"),
        _("This setting is not observed by all projects. Use this setting to "
        "prevent the drive spinning up unnecessarily. Also refer to your power "
        "management settings. Default 60 seconds."),
        CValidateNumber<double>(&m_preferences->disk_interval))
    );

    CPrefGroup* vm = AddGroup(_("Virtual Memory"));

    vm->AddPreference(new CPrefValueText(this,
        _("disk_interval"),
        _("Use no more than"),
        _("% of page file"),
        _("Restrict the amount of virtual memory that BOINC may use. This "
        "setting should be used with care if your page file size is managed by "
        "the operating system. Default 75%."),
        CValidatePercent<double>(&m_preferences->vm_max_used_frac))
    );
}




