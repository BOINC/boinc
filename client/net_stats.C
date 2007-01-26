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
#include "config.h"
#include <cstring>
#include <cmath>
#endif

#include "parse.h"
#include "time.h"
#include "util.h"
#include "error_numbers.h"

#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"

#include "net_stats.h"

#define EXP_DECAY_RATE (1./3600)

NET_STATUS net_status;

NET_STATS::NET_STATS() {
    last_time = 0;
    memset(&up, 0, sizeof(up));
    memset(&down, 0, sizeof(down));
}

void NET_INFO::update(double dt, double nb, bool active) {
    //msg_printf(NULL, MSG_INFO, "dt %f nb %f active %d", dt, nb, active);
    if (active) {
        delta_t += dt;
        delta_nbytes += nb-last_bytes;
    }
    last_bytes = nb;
}

double NET_INFO::throughput() {
    double x, tp, new_tp=0;
    if (starting_throughput > 0) {
        if (delta_t > 0) {
            x = exp(-delta_t*EXP_DECAY_RATE);
            tp = delta_nbytes/delta_t;
            new_tp = x*starting_throughput + (1-x)*tp;
        } else {
            new_tp = starting_throughput;
        }
    } else if (delta_t > 0) {
        new_tp = delta_nbytes/delta_t;
    } else {
    }
#if 0
    msg_printf(NULL, MSG_INFO, "start %f delta_t %f delta_nb %f new_tp %f",
        starting_throughput, delta_t, delta_nbytes, new_tp
    );
#endif
    starting_throughput = new_tp;
    delta_nbytes = delta_t = 0;
    return new_tp;
}

void NET_STATS::poll(FILE_XFER_SET& fxs, HTTP_OP_SET& hops) {
    double dt;
    bool upload_active, download_active;

    if (last_time == 0) {
        dt = 0;
    } else {
        dt = gstate.now - last_time;
    }
    last_time = gstate.now;

    fxs.check_active(upload_active, download_active);
    up.update(dt, hops.bytes_up, upload_active);
    down.update(dt, hops.bytes_down, download_active);

	if (net_status.need_to_contact_reference_site && gstate.gui_http.state==GUI_HTTP_STATE_IDLE) {
		net_status.contact_reference_site();
	}
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
        } else {
            if (log_flags.unparsed_xml) {
                msg_printf(NULL, MSG_INFO,
                    "[unparsed_xml] Unrecognized network statistics line: %s", buf
                );
            }
        }
    }
    return ERR_XML_PARSE;
}

// Return:
// 0 if we have network connections open
// 1 if we need a physical connection
// 2 if we don't have any connections, and don't need any
// 3 if a website lookup is pending (try again later)
//
// There's a 10-second slop factor;
// if we've done network comm in the last 10 seconds,
// we act as if we're doing it now.
// (so that polling mechanisms have a chance to start other xfers,
// in the case of a modem connection waiting to be closed by the mgr)
//
int NET_STATUS::network_status() {
	int retval;

    if (gstate.http_ops->nops()) {
        last_comm_time = gstate.now;
    }
	if (gstate.lookup_website_op.error_num == ERR_IN_PROGRESS) {
        retval = NETWORK_STATUS_LOOKUP_PENDING;
	} else if (gstate.now - last_comm_time < 10) {
        //msg_printf(0, MSG_INFO, "nops %d; return 0", http_ops->nops());
        retval = NETWORK_STATUS_ONLINE;
    } else if (need_physical_connection) {
        //msg_printf(0, MSG_INFO, "need phys conn; return 1");
        retval = NETWORK_STATUS_WANT_CONNECTION;
    } else if (gstate.active_tasks.want_network()) {
        retval = NETWORK_STATUS_WANT_CONNECTION;
	} else {
		have_sporadic_connection = false;
    //msg_printf(0, MSG_INFO, "returning 2");
		retval = NETWORK_STATUS_WANT_DISCONNECT;
	}
	if (log_flags.network_status_debug) {
		msg_printf(NULL, MSG_INFO, "[network_status_debug] status: %s", network_status_string(retval));
	}
	return retval;
}

// There's now a network connection, after some period of disconnection.
// Do all communication that we can.
//
void NET_STATUS::network_available() {
    unsigned int i;

    have_sporadic_connection = true;
    for (i=0; i<gstate.pers_file_xfers->pers_file_xfers.size(); i++) {
        PERS_FILE_XFER* pfx = gstate.pers_file_xfers->pers_file_xfers[i];
        pfx->next_request_time = 0;
    }
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->min_rpc_time = 0;
    }

    // tell active tasks that network is available (for Folding@home)
    //
    gstate.active_tasks.network_available();
}

// An HTTP operation failed;
// it could be because there's no physical network connection.
// Find out for sure by trying to contact google
//
void NET_STATUS::got_http_error() {
    if ((gstate.lookup_website_op.error_num != ERR_IN_PROGRESS)
        && !need_physical_connection
    ) {
		need_to_contact_reference_site = true;
    }
}

void NET_STATUS::contact_reference_site() {
    std::string url = "http://www.google.com";
	if (log_flags.network_status_debug) {
		msg_printf(0, MSG_INFO,
			"[network_status_debug] need_phys_conn %d; trying google", need_physical_connection
		);
	}
    gstate.lookup_website_op.do_rpc(url);
	need_to_contact_reference_site = false;
}

int LOOKUP_WEBSITE_OP::do_rpc(string& url) {
    int retval;

    msg_printf(0, MSG_INFO, "Project communication failed: attempting access to reference site");
    retval = gstate.gui_http.do_rpc(this, url, LOOKUP_WEBSITE_FILENAME);
    if (retval) {
        error_num = retval;
        net_status.need_physical_connection = true;
		net_status.last_comm_time = 0;
        msg_printf(0, MSG_USER_ERROR,
            "Access to reference web site failed - check network connection or proxy configuration."
        );
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void LOOKUP_WEBSITE_OP::handle_reply(int http_op_retval) {
    error_num = http_op_retval;

    // if we couldn't contact a reference web site,
    // we can assume there's a problem that requires user attention
    // (usually no physical network connection).
    // Set a flag that will signal the Manager to that effect
    //
    if (http_op_retval) {
        net_status.need_physical_connection = true;
		net_status.last_comm_time = 0;
        msg_printf(0, MSG_USER_ERROR,
            "Access to reference site failed - check network connection or proxy configuration."
        );
    } else {
        msg_printf(0, MSG_INFO,
            "Access to reference site succeeded - project servers may be temporarily down."
        );
    }
}

const char *BOINC_RCSID_733b4006f5 = "$Id$";
