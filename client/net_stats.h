// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

// keep track of the network performance of this host,
// namely exponentially weighted averages of upload and download speeds
//

#ifndef _WIN32
#include <stdio.h>
#endif

#include "net_xfer.h"
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
    void poll(NET_XFER_SET&);
    
    int write(MIOFILE&, bool to_server);
    int parse(MIOFILE&);
};
