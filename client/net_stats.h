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

// keep track of the network performance of this host,
// namely exponentially weighted averages of upload and download speeds
//

#ifndef _NET_STATS_
#define _NET_STATS_

#ifndef _WIN32
#include <cstdio>
#endif

#include "miofile.h"

// there's one of these each for upload and download
//
struct NET_INFO {
    double delta_t;         // elapsed time of file transfer activity
                            // in this session of client
    double delta_nbytes;    // bytes transferred in this session
    double last_bytes;
    double starting_throughput; // throughput at start of session

    void update(double dt, double nb, bool active);
    double throughput();
};

class NET_STATS {
public:
    double last_time;
    NET_INFO up;
    NET_INFO down;

    NET_STATS();
    void poll(FILE_XFER_SET&, HTTP_OP_SET&);

    int write(MIOFILE&);
    int parse(MIOFILE&);
};

class NET_STATUS {
public:
    bool need_physical_connection;
        // client wants to do network comm and no physical connection exists.
        // Initially false; set whenever a Curl operation
        // returns CURLE_COULDNT_RESOLVE_HOST,
        // and a subsequent request to a highly-available site
        // also returns CURLE_COULDNT_RESOLVE_HOST.
        // cleared whenever we transfer data,
        // or an operation returns some other value
        //
    bool have_sporadic_connection;
        // we have a network connection, but it's likely to go away soon,
        // so do as much network comm as possible
        // (e.g. report completed results)
        //
    int network_status();
    void network_available();
    void got_http_error();
    NET_STATUS() {
        need_physical_connection = false;
        have_sporadic_connection = false;
    }
};

extern NET_STATUS net_status;

#endif
