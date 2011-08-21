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
    if (!cpu_time) {
        log_messages.printf(MSG_NORMAL, "unexpected zero CPU time\n");
        return ERR_XML_PARSE;
    }

    DB_HOST host;
    int retval = host.lookup_id(msg.hostid);
    if (retval) return retval;
    HOST old_host = host;

    double cpu_flops_sec = host.p_fpops;

    // sanity checks - customize as needed
    //
    if (cpu_flops_sec < 0) cpu_flops_sec = 1e9;
    if (cpu_flops_sec > 2e10) cpu_flops_sec = 2e10;
    double credit = cpu_time_to_credit(cpu_time, cpu_flops_sec);
    grant_credit(host, dtime()-86400, credit);

    // update the host's credit fields
    //
    retval = host.update_diff_validator(old_host);

    return 0;
}
