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
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#include <math.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "client_state.h"
#include "client_types.h"
#include "error_numbers.h"
#include "file_names.h"
#include "log_flags.h"
#include "parse.h"
#include "util.h"

// PERS_FILE_XFER represents a persistent file transfer.
// A set of URL is given.

// For download, the object attempts to download the file
// from any of the URLs.
// If one fails or is not available, try another,
// using an exponential backoff policy to avoid flooding servers.

// For upload, try to upload the file to the first URL;
// if that fails try the others.
//
int PERS_FILE_XFER::init(FILE_INFO* the_file, bool is_file_upload) {
    fxp = NULL;
    this->fip = the_file;
    nretry = 0;
    first_request_time = time(NULL);
    next_request_time = first_request_time;
    is_upload = is_file_upload;
    pers_xfer_retval = -1;
    xfer_done = false;

    return 0;
}

// Try to start the file transfer associated with this persistent file transfer.
// Returns true if transfer was successfully started, false otherwise
//
bool PERS_FILE_XFER::start_xfer() {
    FILE_XFER *file_xfer;
    int retval;

    // Decide whether to start a new file transfer
    if(gstate.start_new_file_xfer()) {
        // Create a new FILE_XFER object and initialize a
        // download or upload for the persistent file transfer
        file_xfer = new FILE_XFER;
        if (is_upload) {
            retval = file_xfer->init_upload(*fip);
        } else {
            retval = file_xfer->init_download(*fip);
        }
        if (!retval) {
            retval = gstate.insert_file_xfer(file_xfer);
        }
        if (retval) {
            fprintf(stderr, "couldn't start %s for %s: error %d\n",
                    (is_upload ? "upload" : "download"), fip->get_url(), retval);
        } else {
            fxp = file_xfer;
            if (log_flags.file_xfer) {
                printf("started %s of %s\n", (is_upload ? "upload" : "download"), fip->get_url());
            }
            return true;
        }
    }
    return false;
}

// Poll the status of this persistent file transfer.  If it's time to start it, then
// attempt to start it. If it has finished or failed, then deal with it appropriately
//
int PERS_FILE_XFER::poll(unsigned int now) {
    double exp_backoff;
    int retval;
    char pathname[256];
   
    if (fxp) {
        if (fxp->file_xfer_done) {
            if (fxp->file_xfer_retval == 0) {
                // The transfer finished with no errors.  We will clean up the
                // PERS_FILE_XFER object in garbage_collect() later
                //
                if (log_flags.file_xfer) {
                    printf( "file transfer done for %s; retval %d\n",
                            fip->get_url(), fxp->file_xfer_retval );
                }
                if (fip->generated_locally) {
                    // If the file was generated locally (for upload), update stats
                    // and delete the local copy of the file if needed
                    //
                    gstate.update_net_stats(true, fip->nbytes, fxp->elapsed_time());

                    // file has been uploaded - delete if not sticky
                    if (!fip->sticky) {
                        fip->delete_file();
                    }
                    fip->uploaded = true;
                } else {
                    // Otherwise we downloaded the file.  Update stats, verify
                    // the file with RSA or MD5, and change permissions
                    gstate.update_net_stats(false, fip->nbytes, fxp->elapsed_time());
                    get_pathname(fip, pathname);
                    retval = verify_downloaded_file(pathname, *fip);
                    if (retval) {
                        fprintf(stderr, "checksum or signature error for %s\n", fip->name);
                    } else {
                        if (log_flags.file_xfer_debug) {
                            printf("MD5 checksum validated for %s\n", pathname);
                        }
                        // Set the appropriate permissions depending on whether
                        // it's an executable or normal file
                        if (fip->executable) {
                            retval = chmod(pathname, S_IEXEC|S_IREAD|S_IWRITE);
                        } else {
                            get_pathname(fip, pathname);
                            retval = chmod(pathname, S_IREAD|S_IWRITE);
                        }
                        fip->file_present = true;
                    }
                }
                xfer_done = true;
                pers_xfer_retval = 0;
            } else {
                // file xfer failed.
                // Cycle to the next URL to try
                fip->current_url = (fip->current_url + 1)%fip->urls.size();

                // If we reach the URL that we started at, then we have tried all
                // servers without success
                if( fip->current_url == fip->start_url ) {
                    nretry++;
                    exp_backoff = exp(((double)rand()/(double)RAND_MAX)*nretry);
                    // Do an exponential backoff of e^nretry seconds, keeping within the bounds
                    // of PERS_RETRY_DELAY_MIN and PERS_RETRY_DELAY_MAX
                    next_request_time = now+(int)max(PERS_RETRY_DELAY_MIN,min(PERS_RETRY_DELAY_MAX,exp_backoff));
                }
                
                // See if it's time to give up on the persistent file xfer
                //
                if ((now - first_request_time) > gstate.giveup_after) {
                    xfer_done = true;
                    pers_xfer_retval = ERR_GIVEUP;
                }
            }
            // Disassociate the finished file transfer
            fxp = NULL;

            return true;
        }
    } else {
        // No file xfer is active.
        // We must be waiting after a failure.
        // See if it's time to try again.
        //
        if (now >= (unsigned int)next_request_time) {
            return start_xfer();
        }
    }

    return false;
}

// Parse XML information about a single persistent file transfer
//
int PERS_FILE_XFER::parse(FILE* fin) {
    char buf[256];
    if(fin==NULL) {
        fprintf(stderr, "error: PERS_FILE_XFER_SET.parse: unexpected NULL pointer fin\n");
        return ERR_NULL;
    }
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</persistent_file_xfer>")) return 0;
        else if (parse_int(buf, "<num_retries>", nretry)) continue;
        else if (parse_int(buf, "<first_request_time>", first_request_time)) continue;
        else if (parse_int(buf, "<next_request_time>", next_request_time)) continue;
        else fprintf(stderr, "PERS_FILE_XFER::parse(): unrecognized: %s\n", buf);
    }
    return -1;
}

// Write XML information about a particular persistent file transfer
//
int PERS_FILE_XFER::write(FILE* fout) {
    if(fout==NULL) {
        fprintf(stderr, "error: PERS_FILE_XFER_SET.write: unexpected NULL pointer fout\n");
        return ERR_NULL;
    }
    fprintf(fout,
            "    <persistent_file_xfer>\n"
            "        <num_retries>%d</num_retries>\n"
            "        <first_request_time>%d</first_request_time>\n"
            "        <next_request_time>%d</next_request_time>\n"
            "    </persistent_file_xfer>\n",
            nretry, first_request_time, next_request_time);
    return 0;
}

PERS_FILE_XFER_SET::PERS_FILE_XFER_SET(FILE_XFER_SET* p) {
    if(p==NULL) {
        fprintf(stderr, "error: PERS_FILE_XFER_SET: unexpected NULL pointer p\n");
    }
    file_xfers = p;
}

int PERS_FILE_XFER_SET::poll() {
    unsigned int i;
    bool action = false;
    int now = time(NULL);

    for (i=0; i<pers_file_xfers.size(); i++) {
        action |= pers_file_xfers[i]->poll(now);
    }

    return action;
}

// Insert a PERS_FILE_XFER object into the set.  We will decide which ones to start
// when we hit the polling loop
//
int PERS_FILE_XFER_SET::insert(PERS_FILE_XFER* pfx) {
    pers_file_xfers.push_back(pfx);
    return 0;
}

// Remove a PERS_FILE_XFER object from the set.  What should the action here be?
//
int PERS_FILE_XFER_SET::remove(PERS_FILE_XFER* pfx) {
    vector<PERS_FILE_XFER*>::iterator iter;

    iter = pers_file_xfers.begin();
    while (iter != pers_file_xfers.end()) {
        if (*iter == pfx) {
            pers_file_xfers.erase(iter);
            return 0;
        }
        iter++;
    }
    fprintf(stderr, "PERS_FILE_XFER_SET::remove(): not found\n");
    return 1;
}
