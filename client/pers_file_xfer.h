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

#ifndef _PERS_FILE_XFER_H
#define _PERS_FILE_XFER_H

#include "client_types.h"
#include "file_xfer.h"


// PERS_FILE_XFER represents a persistent file transfer.
// A set of URL is given in the FILE_INFO.

// For download, the object attempts to download the file
// from any of the URLs.
// If one fails or is not available, try another,
// using an exponential backoff policy to avoid flooding servers.

// For upload, try to upload the file to the first URL;
// if that gets transient failure, try the others.

// Default values for exponential backoff
#define PERS_RETRY_DELAY_MIN    60                // 1 minute
#define PERS_RETRY_DELAY_MAX    (60*60*4)         // 4 hours
#define PERS_GIVEUP             (60*60*24*7*2)    // 2 weeks
    // give up on xfer if this time elapses since last byte xferred

class PERS_FILE_XFER {
    int nretry;                // # of retries so far
    double first_request_time;    // time of first transfer request

public:
    bool is_upload;
    double next_request_time;     // UNIX time to next retry the file request
    double time_so_far;
        // Total time there's been an active FILE_XFER for this PFX
        // Currently not used for anything;  not meaningful for throughput
        // because could include repeated transfer
    double last_time;
        // when the above was last updated.
        // Defined only while a transfer is active
    bool pers_xfer_done;
    FILE_XFER* fxp;     // nonzero if file xfer in progress
    FILE_INFO* fip;

    PERS_FILE_XFER();
    ~PERS_FILE_XFER();
    int init(FILE_INFO*, bool is_file_upload);
    bool poll(double);
    void handle_xfer_failure();
    void retry_or_backoff();
    void check_giveup(char*);
    void abort();
    int write(MIOFILE& fout);
    int parse(MIOFILE& fin);
    int start_xfer();
    void suspend();
};

class PERS_FILE_XFER_SET {
    FILE_XFER_SET* file_xfers;
public:
    std::vector<PERS_FILE_XFER*>pers_file_xfers;

    PERS_FILE_XFER_SET(FILE_XFER_SET*);
    int insert(PERS_FILE_XFER*);
    int remove(PERS_FILE_XFER*);
    bool poll(double);
    void suspend();
};

#endif
