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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// scheduler code related to homogeneous redundancy

#include "config.h"
#include <ctime>
#include <cstdio>
#include <stdlib.h>

#include "error_numbers.h"

#include "server_types.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "main.h"
#include "hr.h"
#include "sched_hr.h"


#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#else
#define FCGI_ToFILE(x) (x)
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
bool already_sent_to_different_platform_quick(
    SCHEDULER_REQUEST& sreq, WORKUNIT& wu, APP& app
) {
    if (wu.hr_class && (hr_class(sreq.host, app_hr_type(app)) != wu.hr_class)) {
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
bool already_sent_to_different_platform_careful(
    SCHEDULER_REQUEST& sreq, WORK_REQ& wreq, WORKUNIT& workunit, APP& app
) {
    DB_WORKUNIT db_wu;
    int retval, wu_hr_class;
    char buf[256];

    // reread hr_class field from DB in case it's changed
    //
    db_wu.id = workunit.id;
    retval = db_wu.get_field_int("hr_class", wu_hr_class);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL, "can't get hr_class for %d: %d\n",
            db_wu.id, retval
        );
        return true;
    }
    wreq.hr_reject_temp = false;
    int host_hr_class = hr_class(sreq.host, app_hr_type(app));
    if (wu_hr_class) {
        if (host_hr_class != wu_hr_class) {
            wreq.hr_reject_temp = true;
        }
    } else {
        sprintf(buf, "hr_class=%d", host_hr_class);
        db_wu.update_field(buf);
    }
    return wreq.hr_reject_temp;
}

const char *BOINC_RCSID_4196d9a5b4="$Id$";
