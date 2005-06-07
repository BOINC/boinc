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

// NET_STATS estimates average network throughput,
// i.e. the average total throughput in both the up and down directions.
// Here's how it works: NET_STATS::poll() is called every second or so.
// If there are any file transfers active,
// it increments elapsed time and byte counts,
// and maintains an exponential average of throughput.

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cstring>
#include <cmath>
#endif

#include "parse.h"
#include "time.h"

#include "util.h"
#include "error_numbers.h"
#include "client_msgs.h"
#include "client_state.h"

#include "net_stats.h"

#define EXP_DECAY_RATE (1./SECONDS_PER_DAY)

NET_STATS::NET_STATS() {
    last_time = 0;
    memset(&up, 0, sizeof(up));
    memset(&down, 0, sizeof(down));
}

void NET_INFO::update(double dt, double nb, bool active) {
    if (active) {
        delta_t += dt;
        delta_nbytes += nb-last_bytes;
    }
    last_bytes = nb;
}

double NET_INFO::throughput() {
    double x, tp;
    if (starting_throughput > 0) {
        if (delta_t > 0) {
            x = exp(delta_t*EXP_DECAY_RATE);
            tp = delta_nbytes/delta_t;
            return x*starting_throughput + (1-x)*tp;
        } else {
            return starting_throughput;
        }
    } else if (delta_t > 0) {
        return delta_nbytes/delta_t;
    }
    return 0;
}

void NET_STATS::poll(NET_XFER_SET& nxs) {
    double dt;
    bool upload_active, download_active;

    if (last_time == 0) {
        dt = 0;
    } else {
        dt = gstate.now - last_time;
    }
    last_time = gstate.now;

    nxs.check_active(upload_active, download_active);
    up.update(dt, nxs.bytes_up, upload_active);
    down.update(dt, nxs.bytes_down, download_active);
}

// Write XML based network statistics
//
int NET_STATS::write(MIOFILE& out) {
    out.printf(
        "<net_stats>\n"
        "    <bwup>%g</bwup>\n"
        "    <bwdown>%g</bwdown>\n"
        "</net_stats>\n",
        up.throughput(),
        down.throughput()
    );
    return 0;
}

// Read XML based network statistics
//
int NET_STATS::parse(MIOFILE& in) {
    char buf[256];
    double bwup, bwdown;

    memset(this, 0, sizeof(NET_STATS));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</net_stats>")) return 0;
        else if (parse_double(buf, "<bwup>", bwup)) {
            up.starting_throughput = bwup;
            continue;
        }
        else if (parse_double(buf, "<bwdown>", bwdown)) {
            down.starting_throughput = bwdown;
            continue;
        }
        else msg_printf(NULL, MSG_ERROR, "NET_STATS::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

const char *BOINC_RCSID_733b4006f5 = "$Id$";
