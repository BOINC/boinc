// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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


#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

// return true if HR rules out sending any work to this host
//
bool hr_unknown_platform(HOST& host) {
    for (int i=0; i<ssp->napps; i++) {
        APP& app = ssp->apps[i];
        if (!hr_unknown_platform_type(host, app_hr_type(app))) return false;
    }
    return true;
}

// quick check for platform compatibility
//
bool already_sent_to_different_platform_quick(WORKUNIT& wu, APP& app) {
    if (wu.hr_class && (hr_class(g_request->host, app_hr_type(app)) != wu.hr_class)) {
        return true;
    }
    return false;
}

// If we've already sent a result of this WU to a different platform
//    return true
// else if we haven't sent a result to ANY platform
//    update WU with platform code
//    return false
//
// (where "platform" is os_name + p_vendor; may want to sharpen this for Unix)
//
// This is "careful" in that it rereads the WU from DB
//
bool already_sent_to_different_platform_careful(WORKUNIT& workunit, APP& app) {
    DB_WORKUNIT db_wu;
    int retval, wu_hr_class;
    char buf[256], buf2[256];

    // reread hr_class field from DB in case it's changed
    //
    db_wu.id = workunit.id;
    retval = db_wu.get_field_int("hr_class", wu_hr_class);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "can't get hr_class for [WU#%d]: %d\n", db_wu.id, retval
        );
        return true;
    }
    g_wreq->hr_reject_temp = false;
    int host_hr_class = hr_class(g_request->host, app_hr_type(app));
    if (wu_hr_class) {
        if (host_hr_class != wu_hr_class) {
            g_wreq->hr_reject_temp = true;
        }
    } else {
        // do a "careful update" to make sure the WU's hr_class hasn't
        // changed since we read it earlier
        //
        sprintf(buf, "hr_class=%d", host_hr_class);
        sprintf(buf2, "hr_class=%d", wu_hr_class);
        retval = db_wu.update_field(buf, buf2);
        if (retval) return true;
        if (boinc_db.affected_rows() != 1) return true;
    }
    return g_wreq->hr_reject_temp;
}

const char *BOINC_RCSID_4196d9a5b4="$Id$";
