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
