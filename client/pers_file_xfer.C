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

#include "windows_cpp.h"

#include <math.h>
#include <stdlib.h>

#include "client_state.h"
#include "client_types.h"
#include "error_numbers.h"
#include "file_names.h"
#include "log_flags.h"
#include "parse.h"
#include "util.h"

// PERS_FILE_XFER represents a persistent file transfer.
// A set of URLs is given.
//
// For download, the object attempts to download the file
// from any of the URLs.
// If one fails or is not available, try another,
// using an exponential backoff policy to avoid flooding servers.
//
// For upload, try to upload the file to the first URL;
// if that fails try the others.

PERS_FILE_XFER::PERS_FILE_XFER() {
    nretry = 0;
    first_request_time = time(0);
    next_request_time = first_request_time;
    time_so_far = 0;
}

int PERS_FILE_XFER::init(FILE_INFO* f, bool is_file_upload) {
    fxp = NULL;
    fip = f;
    is_upload = is_file_upload;
    xfer_done = false;

    return 0;
}

// Try to start the file transfer associated with this persistent file transfer.
// Returns true if transfer was successfully started, false otherwise
//
bool PERS_FILE_XFER::start_xfer() {
    FILE_XFER *file_xfer;
    int retval;
    //struct tm *newtime;
    //time_t now;
    char buf[256];

    //now = time(0);

    // Decide whether to start a new file transfer
    //
    if (!gstate.start_new_file_xfer()) {
        return false;
    }
    
    // Create a new FILE_XFER object and initialize a
    // download or upload for the persistent file transfer
    //
    file_xfer = new FILE_XFER;
    if (gstate.use_http_proxy) {
        file_xfer->use_http_proxy = true;
        safe_strcpy(file_xfer->proxy_server_name, gstate.proxy_server_name);
        file_xfer->proxy_server_port = gstate.proxy_server_port;
    }
    if (is_upload) {
        if (gstate.exit_before_upload) {
            exit(0);
        }
        retval = file_xfer->init_upload(*fip);
    } else {
        retval = file_xfer->init_download(*fip);
    }
    if (retval) {
        sprintf(buf,
            "Couldn't start %s for %s: error %d",
            (is_upload ? "upload" : "download"), fip->get_url(), retval
        );
        show_message(fip->project, buf, MSG_ERROR);
        // TODO: do we need to do anything here?
    } else {
        retval = gstate.file_xfers->insert(file_xfer);
        fxp = file_xfer;
        if (retval) {
            if (log_flags.file_xfer) {
                show_message(fip->project, "file_xfer insert failed", MSG_ERROR);
            }
            fxp->file_xfer_retval = retval;
            handle_xfer_failure(time(0));
            fxp = NULL;
            return false;
        }
        if (log_flags.file_xfer) {
            sprintf(buf,
                "Started %s of %s",
                (is_upload ? "upload" : "download"), fip->name
            );
            show_message(fip->project, buf, MSG_INFO);
        }
        if (log_flags.file_xfer_debug) {
            sprintf(buf,
                "URL: %s",
                fip->get_url()
            );
            show_message(fip->project, buf, MSG_INFO);
        }
        return true;
    }
    return false;
}

// Poll the status of this persistent file transfer.
// If it's time to start it, then attempt to start it.
// If it has finished or failed, then deal with it appropriately
//
bool PERS_FILE_XFER::poll(time_t now) {
    int retval;
    char pathname[256], buf[256];
    
    if (xfer_done) {
        return false;
    }
    if (!fxp) {
        // No file xfer is active.
        // We must be waiting after a failure.
        // See if it's time to try again.
        //
        if (now >= next_request_time) {
            bool ok = start_xfer();
            last_time = dtime();
            return ok;
        } else {
            return false;
        }
    }

    if (dtime() - last_time <= 2) {
        time_so_far += dtime() - last_time;
    }
    last_time = dtime();

    if (fxp->file_xfer_done) {
        if (log_flags.file_xfer) {
            sprintf(
                buf,
                "Finished %s of %s",
                is_upload?"upload":"download", fip->name
            );
            show_message(fip->project, buf, MSG_INFO);
        }
        if (log_flags.file_xfer_debug) {
            sprintf(
                buf,
                "file transfer status %d",
                fxp->file_xfer_retval
            );
            show_message(fip->project, buf, MSG_INFO);
        }
        if (fxp->file_xfer_retval == 0) {
            // The transfer finished with no errors.
            //
            if (fip->generated_locally) {
                // file has been uploaded - delete if not sticky
                //
                if (!fip->sticky) {
                    fip->delete_file();
                }
                fip->uploaded = true;
                xfer_done = true;
            } else {

                // verify the file with RSA or MD5, and change permissions
                //
                get_pathname(fip, pathname);
                retval = verify_downloaded_file(pathname, *fip);
                if (retval) {
                    sprintf(buf, "Checksum or signature error for %s", fip->name);
                    show_message(fip->project, buf, MSG_ERROR);
                    fip->status = retval;
                } else {
                    if (log_flags.file_xfer_debug) {
                        sprintf(buf, "MD5 checksum validated for %s", pathname);
                        show_message(fip->project, buf, MSG_INFO);
                    }
                    // Set the appropriate permissions depending on whether
                    // it's an executable or normal file
                    //
                    retval = fip->set_permissions();
                    fip->status = FILE_PRESENT;
                }
                xfer_done = true;
            }
        } else {
            handle_xfer_failure(now);
        }
        // remove fxp from file_xfer_set and deallocate it
        //
        gstate.file_xfers->remove(fxp);
        delete fxp;
        fxp = NULL;

        return true;
    }

    return false;
}

// Handle a transfer failure
//
void PERS_FILE_XFER::handle_xfer_failure(time_t cur_time) {
    char buf[256];

    // If it was a bad range request, delete the file and start over
    //
    if (fxp->file_xfer_retval == HTTP_STATUS_RANGE_REQUEST_ERROR) {
        fip->delete_file();
    }

    retry_or_backoff(cur_time);
    
    // See if it's time to give up on the persistent file xfer
    //
    if ((cur_time - first_request_time) > gstate.file_xfer_giveup_period) {
        if (is_upload) {
            fip->status = ERR_GIVEUP_UPLOAD;
        } else {
            fip->status = ERR_GIVEUP_DOWNLOAD;
        }
        xfer_done = true;
        sprintf(buf,
            "Giving up on file transfer for %s: %d",
            fip->name, fip->status
        );
        show_message(fip->project, buf, MSG_ERROR);
    }
}

// Cycle to the next URL, or if we've hit all URLs in this cycle,
// backoff and try again later
//
void PERS_FILE_XFER::retry_or_backoff(time_t cur_time) {
    double exp_backoff;
    struct tm *newtime;
    time_t aclock;
    char buf[256];
    
    time(&aclock);
    newtime = localtime(&aclock);

    // Cycle to the next URL to try
    //
    fip->current_url = (fip->current_url + 1)%fip->urls.size();

    // If we reach the URL that we started at, then we have tried all
    // servers without success
    //
    if (fip->current_url == fip->start_url) {
        nretry++;
        exp_backoff = exp(((double)rand()/(double)RAND_MAX)*nretry);

        // Do an exponential backoff of e^nretry seconds,
        // keeping within the bounds of PERS_RETRY_DELAY_MIN and
        // PERS_RETRY_DELAY_MAX
        //
        next_request_time = cur_time+(int)max(PERS_RETRY_DELAY_MIN,min(PERS_RETRY_DELAY_MAX,exp_backoff));
    }
    if (log_flags.file_xfer_debug) {
        sprintf(buf,
            "Backing off %f seconds on transfer of file %s",
            exp_backoff, fip->name
        );
        show_message(fip->project, buf, MSG_INFO);
    }
}

// Parse XML information about a single persistent file transfer
//
int PERS_FILE_XFER::parse(FILE* fin) {
    char buf[256], buf2[256];

    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</persistent_file_xfer>")) return 0;
        else if (parse_int(buf, "<num_retries>", nretry)) continue;
        else if (parse_int(buf, "<first_request_time>", first_request_time)) continue;
        else if (parse_int(buf, "<next_request_time>", next_request_time)) continue;
        else if (parse_double(buf, "<time_so_far>", time_so_far)) continue;
        else {
            sprintf(buf2, "PERS_FILE_XFER::parse(): unrecognized: %s", buf);
            show_message(fip->project, buf2, MSG_ERROR);
        }
    }
    return -1;
}

// Write XML information about a particular persistent file transfer
//
int PERS_FILE_XFER::write(FILE* fout) {
    fprintf(fout,
        "    <persistent_file_xfer>\n"
        "        <num_retries>%d</num_retries>\n"
        "        <first_request_time>%d</first_request_time>\n"
        "        <next_request_time>%d</next_request_time>\n"
        "        <time_so_far>%f</time_so_far>\n"
        "    </persistent_file_xfer>\n",
        nretry, first_request_time, next_request_time, time_so_far
    );
    return 0;
}

PERS_FILE_XFER_SET::PERS_FILE_XFER_SET(FILE_XFER_SET* p) {
    file_xfers = p;
}

// Run through the set, starting any transfers that need to be
// started and deleting any that have finished
//
bool PERS_FILE_XFER_SET::poll() {
    unsigned int i;
    bool action = false;
    int now = time(0);

    for (i=0; i<pers_file_xfers.size(); i++) {
        action |= pers_file_xfers[i]->poll(now);
    }

    if (action) gstate.set_client_state_dirty("pers_file_xfer_set poll");

    return action;
}

// Insert a PERS_FILE_XFER object into the set.
// We will decide which ones to start when we hit the polling loop
//
int PERS_FILE_XFER_SET::insert(PERS_FILE_XFER* pfx) {
    pers_file_xfers.push_back(pfx);
    return 0;
}

// Remove a PERS_FILE_XFER object from the set.
// What should the action here be?
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
    show_message(pfx->fip->project, "PERS_FILE_XFER_SET::remove(): not found", MSG_ERROR);
    return 1;
}
