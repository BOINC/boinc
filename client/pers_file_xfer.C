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
// A set of URL is given.

// For download, the object attempts to download the file
// from any of the URLs.
// If one fails or is not available, try another,
// using an exponential backoff policy to avoid flooding servers.

// For upload, try to upload the file to the first URL;
// if that fails try the others.

int PERS_FILE_XFER::init(FILE_INFO&, bool is_upload) {
}

// 
void PERS_FILE_XFER::try() {
}

void PERS_FILE_XFER::poll(unsigned int now) {
    if (fxp) {
        if (fxp->file_xfer_done) {
            if (fxp->file_xfer_retval == 0) {
            } else {
                // file xfer failed.
                // See if it's time to give up on the persistent file xfer
                //
                diff = now - fip->last_xfer_time;
                if (diff > PERS_GIVEUP) {
                    pers_xfer_done = true;
                    pers_file_xfer_retval = ERR_GIVEUP;
                }
            }
        }
    } else {
        // No file xfer is active.
        // We must be waiting after a failure.
        // See if it's time to try again.
        //
        if (now > retry_time) {
            try();
        }
    }
}

int PERS_FILE_XFER_SET::poll() {
    unsigned int ;
    PERS_FILE_XFER* pfxp;
    bool action = false;
    int now = time(0);

    for (i=0; i<pers_file_xfers.size(); i++) {
        pers_file_xfers[i]->poll(now);
    }
}
