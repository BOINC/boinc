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

// scheduler code related to sending work


#include <ctime>
#include <cstdio>
#include <stdlib.h>

#include "error_numbers.h"

#include "server_types.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_hr.h"


#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#else
#define FCGI_ToFILE(x) (x)
#endif

// modified by Pietro Cicotti
// Check that the two platform has the same architecture and operating system
// Architectures: AMD, Intel, Macintosh
// OS: Linux, Windows, Darwin, SunOS

const int unspec = 0;
const int nocpu = 1;
const int Intel = 2;
const int AMD = 3;
const int Macintosh = 4;

const int noos = 128;
const int Linux = 256;
const int Windows = 384;
const int Darwin = 512;
const int SunOS = 640;

inline
int OS(SCHEDULER_REQUEST& sreq){
    if ( strstr(sreq.host.os_name, "Linux") != NULL ) return Linux;
    else if( strstr(sreq.host.os_name, "Windows") != NULL ) return Windows;
    else if( strstr(sreq.host.os_name, "Darwin") != NULL ) return Darwin;
    else if( strstr(sreq.host.os_name, "SunOS") != NULL ) return SunOS;
    else return noos;
};

inline
int CPU(SCHEDULER_REQUEST& sreq){
    if ( strstr(sreq.host.p_vendor, "Intel") != NULL ) return Intel;
    else if( strstr(sreq.host.p_vendor, "AMD") != NULL ) return AMD;
    else if( strstr(sreq.host.p_vendor, "Macintosh") != NULL ) return Macintosh;
    else return nocpu;
};

#if 0
// old version, just in case
bool same_platform(DB_HOST& host, SCHEDULER_REQUEST& sreq) {
    return !strcmp(host.os_name, sreq.host.os_name)
        && !strcmp(host.p_vendor, sreq.host.p_vendor);
}
#endif

// return true if we've already sent a result of this WU to a different platform
// (where "platform" is os_name + p_vendor;
// may want to sharpen this for Unix)
//
bool already_sent_to_different_platform(
    SCHEDULER_REQUEST& sreq, WORKUNIT& workunit, WORK_REQ& wreq
) {
    DB_WORKUNIT db_wu;
    int retval, hr_class=0;
    char buf[256];

    // reread hr_class field from DB in case it's changed
    //
    db_wu.id = workunit.id;
    retval = db_wu.get_field_int("hr_class", hr_class);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL, "can't get hr_class for %d: %d\n",
            db_wu.id, retval
        );
        return true;
    }
    wreq.homogeneous_redundancy_reject = false;
    if (hr_class != unspec) {
        if (OS(sreq) + CPU(sreq) != hr_class) {
            wreq.homogeneous_redundancy_reject = true;
        }
    } else {
        hr_class = OS(sreq) + CPU(sreq);
        sprintf(buf, "hr_class=%d", hr_class);
        db_wu.update_field(buf);
    }
    return wreq.homogeneous_redundancy_reject;
}

const char *BOINC_RCSID_4196d9a5b4="$Id$";
