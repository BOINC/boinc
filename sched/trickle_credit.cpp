// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// A trickle handler that grants credit based on CPU time
//
// message format:
//
// <cpu_time>x</cpu_time>
//
// NOTE: there is no cheat-prevention mechanism here; add your own.
// NOTE: doesn't work for GPU apps

// sanity-check bounds:
#define MIN_FLOPS 1e9
    // if host FLOPS is < 0, set it to this
#define MAX_FLOPS 2e10
    // cap host FLOPS at this
#define MAX_RUNTIME 3600.
    // this corresponds to the --trickle arg to vboxwrapper

#include "error_numbers.h"
#include "util.h"

#include "credit.h"
#include "miofile.h"
#include "parse.h"
#include "sched_msgs.h"
#include "trickle_handler.h"

int handle_trickle(MSG_FROM_HOST& msg) {
    double cpu_time = 0;
    MIOFILE mf;

    mf.init_buf_read(msg.xml);
    XML_PARSER xp(&mf);

    while (!xp.get_tag()) {
        if (xp.parse_double("cpu_time", cpu_time)) break;
        log_messages.printf(MSG_NORMAL, "unexpected tag: %s\n", xp.parsed_tag);
    }
    if (cpu_time <= 0) {
        log_messages.printf(MSG_NORMAL, "unexpected nonpositive CPU time: %f\n", cpu_time);
        return ERR_XML_PARSE;
    }

    DB_HOST host;
    int retval = host.lookup_id(msg.hostid);
    if (retval) return retval;
    HOST old_host = host;

    double cpu_flops_sec = host.p_fpops;

    // sanity checks - customize as needed
    //
    if (cpu_time > MAX_RUNTIME) {
        log_messages.printf(MSG_NORMAL,
            "Reported runtime exceeds bound: %f>%f\n", cpu_time, MAX_RUNTIME
        );
        return 0;
    }
    if (cpu_flops_sec < 0) {
        log_messages.printf(MSG_NORMAL,
            "host CPU speed %f < 0.  Using %f instead\n",
            cpu_flops_sec, MIN_FLOPS
        );
        cpu_flops_sec = MIN_FLOPS;
    }
    if (cpu_flops_sec > 2e10) {
        log_messages.printf(MSG_NORMAL,
            "host CPU speed %f exceeds %f.  Using %f instead\n",
            cpu_flops_sec, MAX_FLOPS, MAX_FLOPS
        );
        cpu_flops_sec = 2e10;
    }
    double credit = cpu_time_to_credit(cpu_time, cpu_flops_sec);
    grant_credit(host, dtime()-86400, credit);
    log_messages.printf(MSG_DEBUG,
        "granting %f credit to host %d\n", credit, host.id
    );

    // update the host's credit fields
    //
    retval = host.update_diff_validator(old_host);

    return 0;
}
