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

// scheduler code related to homogeneous redundancy

#include "config.h"
#include <ctime>
#include <cstdio>
#include <stdlib.h>
#include <cstring>
#include <string>

#include "error_numbers.h"
#include "sched_types.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_main.h"
#include "hr.h"
#include "sched_hr.h"
#include "boinc_stdio.h"

// return true if HR rules out sending any work to this host
//
bool hr_unknown_platform(HOST& host) {
    for (int i=0; i<ssp->napps; i++) {
        APP& app = ssp->apps[i];
        if (!hr_unknown_class(host, app_hr_type(app))) return false;
    }
    return true;
}

// check for HR compatibility
//
bool already_sent_to_different_hr_class(WORKUNIT& wu, APP& app) {
    g_wreq->hr_reject_temp = false;
    int host_hr_class = hr_class(g_request->host, app_hr_type(app));
    if (wu.hr_class && (host_hr_class != wu.hr_class)) {
        g_wreq->hr_reject_temp = true;
        return true;
    }
    return false;
}
