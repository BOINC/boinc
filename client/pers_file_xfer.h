// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

// PERS_FILE_XFER represents a persistent file transfer.
// A set of URL is given in the FILE_INFO.

// For download, the object attempts to download the file
// from any of the URLs.
// If one fails or is not available, try another,
// using an exponential backoff policy to avoid flooding servers.

// For upload, try to upload the file to the first URL;
// if that fails try the others.

#define PERS_RETRY_DELAY_MIN    60
#define PERS_RETRY_DELAY_MAX    (256*60)
#define PERS_GIVEUP             (3600*24*7)
    // give up on xfer if this time elapses since last byte xferred

class PERS_FILE_XFER {
    int url_index;      // which URL to try next
    int nretry;         // # of retries so far
    FILE_INFO* fip;
    bool is_upload;
    FILE_XFER* fxp;     // nonzero if file xfer in progress
    int retry_time;     // don't retry until this time

    int init(FILE_INFO&, bool is_upload);
};

class PERS_FILE_XFER_SET {
    vector<PERS_FILE_XFER>pers_file_xfers;
    FILE_XFER_SET* file_xfers;
public:
    PERS_FILE_XFER_SET(FILE_XFER_SET*);
    int insert(PERS_FILE_XFER*);
    int remove(PERS_FILE_XFER*);
    int poll();
};
