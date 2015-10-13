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

// A trickle handler that grants credit based on run time
//
// message format:
//
// <runtime>x</runtime>
//
// The credit granted is based on the host's CPU benchmarks,
// and assumes a single-threaded CPU app.
//
// Required cmdline arg:
//
// --max_runtime X     Cap runtime at X
//
// This should match the frequency with which your app
// sends trickle-up messages

#include "error_numbers.h"
#include "util.h"

#include "credit.h"
#include "miofile.h"
#include "parse.h"
#include "sched_msgs.h"
#include "trickle_handler.h"

double flops_50_percentile;
    // default if host value is <= 0
double flops_95_percentile;
    // limit for cheat-proofing
double max_runtime = 0;

int handle_trickle_init(int argc, char** argv) {
    int retval;
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "--max_runtime")) {
            max_runtime = atof(argv[++i]);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown arg %s\n", argv[i]);
            return ERR_XML_PARSE;
        }
    }
    if (!max_runtime) {
        log_messages.printf(MSG_CRITICAL, "missing --max_runtime arg\n");
        return ERR_NULL;
    }

    DB_HOST host;
    retval = host.fpops_percentile(50, flops_50_percentile);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "fpops_percentile failed: %d\n", retval);
        return retval;
    }
    retval = host.fpops_percentile(95, flops_95_percentile);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "fpops_percentile failed: %d\n", retval);
        return retval;
    }

    log_messages.printf(MSG_NORMAL, "default FLOPS: %f\n", flops_50_percentile);
    log_messages.printf(MSG_NORMAL, "max FLOPS: %f\n", flops_95_percentile);
    log_messages.printf(MSG_NORMAL, "max runtime: %f\n", max_runtime);
    return 0;
}

int handle_trickle(MSG_FROM_HOST& msg) {
    double runtime = 0;
    MIOFILE mf;

    mf.init_buf_read(msg.xml);
    XML_PARSER xp(&mf);

    while (!xp.get_tag()) {
        if (xp.parse_double("runtime", runtime)) break;
        log_messages.printf(MSG_NORMAL, "unexpected tag: %s\n", xp.parsed_tag);
    }
    if (runtime <= 0) {
        log_messages.printf(MSG_NORMAL,
            "unexpected nonpositive runtime: %f\n", runtime
        );
        return ERR_XML_PARSE;
    }

    DB_HOST host;
    int retval = host.lookup_id(msg.hostid);
    if (retval) return retval;
    HOST old_host = host;

    double flops_sec = host.p_fpops;

    // sanity checks - customize as needed
    //
    if (runtime > max_runtime) {
        log_messages.printf(MSG_NORMAL,
            "Reported runtime exceeds bound: %f>%f\n", runtime, max_runtime
        );
        runtime = max_runtime;
    }
    if (flops_sec < 0) {
        log_messages.printf(MSG_NORMAL,
            "host CPU speed %f < 0.  Using %f instead\n",
            flops_sec, flops_50_percentile
        );
        flops_sec = flops_50_percentile;
    }
    if (flops_sec > flops_95_percentile) {
        log_messages.printf(MSG_NORMAL,
            "host CPU speed %f exceeds %f.  Using %f instead\n",
            flops_sec, flops_95_percentile, flops_95_percentile
        );
        flops_sec = flops_95_percentile;
    }
    double credit = cpu_time_to_credit(runtime, flops_sec);
    grant_credit(host, dtime()-86400, credit);
    log_messages.printf(MSG_DEBUG,
        "granting %f credit to host %lu\n", credit, host.id
    );

    // update the host's credit fields
    //
    retval = host.update_diff_validator(old_host);

    return 0;
}
