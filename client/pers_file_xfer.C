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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <cmath>
#include <cstdlib>
#endif

#include "error_numbers.h"
#include "md5_file.h"
#include "parse.h"
#include "util.h"
#include "filesys.h"

#include "log_flags.h"
#include "file_names.h"
#include "client_state.h"
#include "client_types.h"
#include "client_msgs.h"

using std::vector;

PERS_FILE_XFER::PERS_FILE_XFER() {
    nretry = 0;
    first_request_time = gstate.now;
    next_request_time = first_request_time;
    time_so_far = 0;
    fip = NULL;
    fxp = NULL;
}

PERS_FILE_XFER::~PERS_FILE_XFER() {
    if (fip) {
        fip->pers_file_xfer = NULL;
    }
}

int PERS_FILE_XFER::init(FILE_INFO* f, bool is_file_upload) {
    fxp = NULL;
    fip = f;
    is_upload = is_file_upload;
    pers_xfer_done = false;
    const char* p = f->get_init_url(is_file_upload);
    if (!p) {
        msg_printf(NULL, MSG_ERROR, "No URL for file transfer of %s", f->name);
        return ERR_NULL;
    }
    return 0;
}

// Try to start the file transfer associated with this persistent file transfer.
//
int PERS_FILE_XFER::start_xfer() {
    FILE_XFER *file_xfer;
    int retval;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_FILE_XFER);

    // Decide whether to start a new file transfer
    //
    if (!gstate.start_new_file_xfer(*this)) {
        return ERR_IDLE_PERIOD;
    }

    // Does the file exist already? this could happen for example if we are
    // downloading an application which exists from a previous installation
    //
    if (!is_upload) {
        char pathname[256];
        get_pathname(fip, pathname);

        // see if file already exists and is valid
        //
        if (!fip->verify_file(true)) {
            retval = fip->set_permissions();
            fip->status = FILE_PRESENT;
            pers_xfer_done = true;

            msg_printf(
                fip->project, MSG_INFO,
                "File %s exists already, skipping download", fip->name
            );

            return 0;
        }
    }

    // Create a new FILE_XFER object and initialize a
    // download or upload for the persistent file transfer
    //
    file_xfer = new FILE_XFER;
    file_xfer->set_proxy(&gstate.proxy_info);
    if (is_upload) {
        if (gstate.exit_before_upload) {
            exit(0);
        }
        retval = file_xfer->init_upload(*fip);
    } else {
        retval = file_xfer->init_download(*fip);
    }
    fxp = file_xfer;
    if (!retval) retval = gstate.file_xfers->insert(file_xfer);
    if (retval) {
        msg_printf(
            fip->project, MSG_ERROR, "Couldn't start %s of %s",
            (is_upload ? "upload" : "download"), fip->name
        );
        msg_printf(
            fip->project, MSG_ERROR, "URL %s: error %d",
            fip->get_current_url(is_upload), retval
        );

        fxp->file_xfer_retval = retval;
        handle_xfer_failure();
        delete fxp;
        fxp = NULL;
        return retval;
    }
    if (log_flags.file_xfer) {
        msg_printf(
            fip->project, MSG_INFO, "Started %s of %s",
            (is_upload ? "upload" : "download"), fip->name
        );
    }
    scope_messages.printf("PERS_FILE_XFER::start_xfer(): URL: %s\n",fip->get_current_url(is_upload));
    return 0;
}

// Poll the status of this persistent file transfer.
// If it's time to start it, then attempt to start it.
// If it has finished or failed, then deal with it appropriately
//
bool PERS_FILE_XFER::poll() {
    int retval;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_FILE_XFER);

    if (pers_xfer_done) {
        return false;
    }
    if (!fxp) {
        // No file xfer is active.
        // Either initial or resume after failure.
        // See if it's time to try again.
        //
        if (gstate.now >= next_request_time) {
            last_time = gstate.now;
            fip->upload_offset = -1;
            retval = start_xfer();
            return (retval == 0);
        } else {
            return false;
        }
    }

    // don't count suspended periods in total time
    //
    double diff = gstate.now - last_time;
    if (diff <= 2) {
        time_so_far += diff;
    }
    last_time = gstate.now;

    if (fxp->file_xfer_done) {
        scope_messages.printf(
            "PERS_FILE_XFER::poll(): file transfer status %d",
            fxp->file_xfer_retval
        );
        if (fxp->file_xfer_retval == 0) {
            // The transfer finished with no errors.
            if (log_flags.file_xfer) {
                msg_printf(
                    fip->project, MSG_INFO, "Finished %s of %s",
                    is_upload?"upload":"download", fip->name
                );
                if (fxp->xfer_speed < 0) {
                    msg_printf(fip->project, MSG_INFO, "No data transferred");
                } else {
                    msg_printf(
                        fip->project, MSG_INFO, "Throughput %d bytes/sec",
                        (int)fxp->xfer_speed
                    );
                }
            }
            pers_xfer_done = true;
        } else if (fxp->file_xfer_retval == ERR_UPLOAD_PERMANENT) {
            if (log_flags.file_xfer) {
                msg_printf(
                    fip->project, MSG_INFO, "Permanently failed %s of %s",
                    is_upload?"upload":"download", fip->name
                );
            }
            check_giveup("server rejected file");
        } else {
            if (log_flags.file_xfer) {
                msg_printf(
                    fip->project, MSG_INFO, "Temporarily failed %s of %s: %d",
                    is_upload?"upload":"download", fip->name,
                    fxp->file_xfer_retval
                );
            }
            handle_xfer_failure();
        }

        // fxp could have already been freed and zeroed by check_giveup
        // so check before trying to remove
        //
        if (fxp) {
            gstate.file_xfers->remove(fxp);
            delete fxp;
            fxp = NULL;
        }
        return true;
    }
    return false;
}

// A file transfer (to a particular server)
// has had a failure
// TODO ?? transient ? permanent? terminology??
//
// Takes a reason why a transfer has failed.
//
// Checks to see if there are no more valid URLs listed in the file_info.
//
// If no more urls are present, the file is then given up on, the reason is
// listed in the error_msg field, and the appropriate status code is given.
//
// If there are more URLs to try, the file_xfer is restarted with these new
// urls until a good transfer is made or it completely gives up.
//
void PERS_FILE_XFER::check_giveup(const char* why) {
    if (fip->get_next_url(fip->upload_when_present) == NULL) {
        // the file has no appropriate download location
        // remove the file from the directory and delete the file xfer object
        gstate.file_xfers->remove(fxp);
        delete fxp;
        fxp = NULL;
        if (is_upload) {
            fip->status = ERR_GIVEUP_UPLOAD;
        } else {
            fip->status = ERR_GIVEUP_DOWNLOAD;
        }
        pers_xfer_done = true;
        msg_printf(
            fip->project, MSG_ERROR, "Giving up on %s of %s: %s",
            is_upload?"upload":"download", fip->name, why
        );
        fip->error_msg = why;
        fip->delete_file();
    } else {
        if (is_upload) {
            if (gstate.exit_before_upload) {
                exit(0);
            }
            fxp->init_upload(*fip);
        } else {
            fxp->init_download(*fip);
        }
    }
}

// Handle a transfer failure
//
void PERS_FILE_XFER::handle_xfer_failure() {

    // If it was a bad range request, delete the file and start over
    //
    if (fxp->file_xfer_retval == HTTP_STATUS_RANGE_REQUEST_ERROR) {
        fip->delete_file();
        return;
    }

    if (fxp->file_xfer_retval == HTTP_STATUS_NOT_FOUND) {
        // If it is uploading and receives a HTTP_STATUS_NOT_FOUND then
        //   the file upload handler could not be found.
        if (!fxp->is_upload) {
            check_giveup("file was not found on server");
            return;
        } else {
            retry_or_backoff();
            return;
        }
    }

    // See if it's time to give up on the persistent file xfer
    //
    if ((gstate.now - first_request_time) > gstate.file_xfer_giveup_period) {
        check_giveup("too much elapsed time");
    } else {
        retry_or_backoff();
    }
}
// Cycle to the next URL, or if we've hit all URLs in this cycle,
// backoff and try again later
//
void PERS_FILE_XFER::retry_or_backoff() {
    double backoff = 0;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_FILE_XFER);

    // Cycle to the next URL to try
    // If we reach the URL that we started at, then we have tried all
    // servers without success
    //

    if (fip->get_next_url(is_upload) == NULL) {
        nretry++;

        // Do an exponential backoff of e^nretry seconds,
        // keeping within the bounds of pers_retry_delay_min and
        // pers_retry_delay_max
        //
        backoff = calculate_exponential_backoff(
            "pers_file_xfer",
            nretry, gstate.pers_retry_delay_min, gstate.pers_retry_delay_max
        );
        next_request_time = gstate.now + backoff;
        msg_printf(fip->project, MSG_INFO,
            "Backing off %s on %s of file %s",
            timediff_format(backoff).c_str(),
            is_upload?"upload":"download",
            fip->name
        );
    }
}

void PERS_FILE_XFER::abort() {
    if (fxp) {
        gstate.file_xfers->remove(fxp);
        delete fxp;
        fxp = NULL;
    }
    fip->status = is_upload?ERR_GIVEUP_UPLOAD:ERR_GIVEUP_DOWNLOAD;
    fip->error_msg = "user requested transfer abort";
    pers_xfer_done = true;
}

// Parse XML information about a persistent file transfer
//
int PERS_FILE_XFER::parse(MIOFILE& fin) {
    char buf[256];

    while (fin.fgets(buf, 256)) {
        if (match_tag(buf, "</persistent_file_xfer>")) return 0;
        else if (parse_int(buf, "<num_retries>", nretry)) continue;
        else if (parse_double(buf, "<first_request_time>", first_request_time)) {
            validate_time(first_request_time);
            continue;
        }
        else if (parse_double(buf, "<next_request_time>", next_request_time)) {
            validate_time(next_request_time);
            continue;
        }
        else if (parse_double(buf, "<time_so_far>", time_so_far)) continue;
        else {
            msg_printf(fip->project, MSG_ERROR, "PERS_FILE_XFER::parse(): unrecognized: %s", buf);
        }
    }
    return ERR_XML_PARSE;
}

// Write XML information about a persistent file transfer
//
int PERS_FILE_XFER::write(MIOFILE& fout) {
    fout.printf(
        "    <persistent_file_xfer>\n"
        "        <num_retries>%d</num_retries>\n"
        "        <first_request_time>%f</first_request_time>\n"
        "        <next_request_time>%f</next_request_time>\n"
        "        <time_so_far>%f</time_so_far>\n"
        "    </persistent_file_xfer>\n",
        nretry, first_request_time, next_request_time, time_so_far
    );
    if (fxp) {
        fout.printf(
            "    <file_xfer>\n"
            "        <bytes_xferred>%f</bytes_xferred>\n"
            "        <file_offset>%f</file_offset>\n"
            "        <xfer_speed>%f</xfer_speed>\n"
            "        <hostname>%s</hostname>\n"
            "    </file_xfer>\n",
            fxp->bytes_xferred,
            fxp->file_offset,
            fxp->xfer_speed,
            fxp->get_hostname()
        );
    }
    return 0;
}

void PERS_FILE_XFER::suspend() {
    if (fxp) {
        gstate.file_xfers->remove(fxp);     // this removes from http_op_set too
        delete fxp;
        fxp = 0;
    }
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
    static double last_time=0;

    if (gstate.now - last_time < 1.0) return false;
    last_time = gstate.now;

    for (i=0; i<pers_file_xfers.size(); i++) {
        action |= pers_file_xfers[i]->poll();
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
    msg_printf(
        pfx->fip->project, MSG_ERROR,
        "PERS_FILE_XFER_SET::remove(): not found"
    );
    return ERR_NOT_FOUND;
}

// suspend all PERS_FILE_XFERs
//
void PERS_FILE_XFER_SET::suspend() {
    unsigned int i;

    for (i=0; i<pers_file_xfers.size(); i++) {
        pers_file_xfers[i]->suspend();
    }
}

const char *BOINC_RCSID_76edfcfb49 = "$Id$";
