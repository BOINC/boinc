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

// keep track of the network performance of this host,
// namely exponentially weighted averages of upload and download speeds
//

#ifndef _NET_STATS_
#define _NET_STATS_

#include <deque>
using std::deque;

#ifndef _WIN32
#include <cstdio>
#endif

#include "miofile.h"

class FILE_XFER_SET;
class HTTP_OP_SET;

// there's one of these each for upload and download

struct NET_INFO {
    double max_rate;
        // estimate of max transfer rate; computed as an average of
        // the rates of recent file transfers, weighted by file size.
        // This ignores concurrency of transfers.
    double avg_rate;
        // recent average transfer rate
    double avg_time;
        // when avg_rate was last updated
    void update(double nbytes, double dt);
        // updates the above vars

};

class NET_STATS {
public:
    NET_INFO up;
    NET_INFO down;

    NET_STATS();

    int write(MIOFILE&);
    int parse(MIOFILE&);
};

class NET_STATUS {
public:
	bool need_to_contact_reference_site;
		// contact the reference site as soon as GUI_HTTP is idle
		// polled from NET_STATS::poll(), for want of a better place
	void contact_reference_site();
    bool show_ref_message;
    bool need_physical_connection;
        // client wants to do network comm and no physical connection exists.
        // Initially false; set whenever a Curl operation
        // returns CURLE_COULDNT_RESOLVE_HOST,
        // and a subsequent request to a highly-available site
        // also returns CURLE_COULDNT_RESOLVE_HOST.
        // cleared whenever we transfer data,
        // or an operation returns some other value
    bool have_sporadic_connection;
        // we have a network connection, but it's likely to go away soon,
        // so do as much network comm as possible
        // (e.g. report completed results)
	double last_comm_time;

    int network_status();
    void network_available();
    void got_http_error();
    NET_STATUS() {
        need_physical_connection = false;
        have_sporadic_connection = false;
		need_to_contact_reference_site = false;
        show_ref_message = false;
		last_comm_time = 0;
    }
    void poll();
};

// This is used to access a reference website (like yahoo or google)
// that is assumed to be 100% available.
// It is used ONLY from the HTTP code, when a transaction fails

struct LOOKUP_WEBSITE_OP: public GUI_HTTP_OP {
    int error_num;

    virtual ~LOOKUP_WEBSITE_OP(){}
    int do_rpc(std::string&);
    virtual void handle_reply(int http_op_retval);
    LOOKUP_WEBSITE_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
};

struct DAILY_XFER {
    int when;
    double up;
    double down;
    DAILY_XFER() {
        when = 0;
        up = down = 0;
    }
    int parse(XML_PARSER&);
    void write(FILE*);
};

struct DAILY_XFER_HISTORY {
    deque<DAILY_XFER> daily_xfers;
    bool dirty;

    DAILY_XFER* today();
    void add(size_t, bool upload);
    void init();
    void poll();
    void totals(int ndays, double& up, double& down);
    void write_state();
    void write_scheduler_request(MIOFILE&, int ndays);
    DAILY_XFER_HISTORY() {
        dirty = false;
    }
};

extern NET_STATUS net_status;
extern DAILY_XFER_HISTORY daily_xfer_history;

#endif
