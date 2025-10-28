// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cmath>
#include <cstdlib>
#endif

#include "error_numbers.h"
#include "md5_file.h"
#include "parse.h"
#include "util.h"
#include "str_util.h"
#include "filesys.h"

#include "client_state.h"
#include "client_types.h"
#include "client_msgs.h"
#include "file_names.h"
#include "log_flags.h"
#include "project.h"

using std::vector;

PERS_FILE_XFER::PERS_FILE_XFER() {
    nretry = 0;
    first_request_time = gstate.now;
    is_upload = false;
    next_request_time = first_request_time;
    time_so_far = 0;
    last_time = 0;
    last_bytes_xferred = 0;
    pers_xfer_done = false;
    fxp = NULL;
    fip = NULL;
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
    URL_LIST ul = f->get_url_list(is_upload);
    const char* p = ul.get_init_url();
    if (!p) {
        msg_printf(NULL, MSG_INTERNAL_ERROR, "No URL for file transfer of %s", f->name);
        return ERR_NULL;
    }
    return 0;
}

int PERS_FILE_XFER::start_xfer() {
    if (is_upload) {
        if (gstate.exit_before_upload) {
            exit(0);
        }
        return fxp->init_upload(*fip);
    } else {
        return fxp->init_download(*fip);
    }
}

// Possibly create and start a file transfer
//
int PERS_FILE_XFER::create_xfer() {
    FILE_XFER *file_xfer;
    int retval;

    // if download, see if file already exists and is valid
    //
    if (!is_upload) {
        char pathname[256];
        get_pathname(fip, pathname, sizeof(pathname));

        retval = fip->verify_file(true, false, true);
        if (!retval) {
            retval = fip->set_permissions();
            fip->status = FILE_PRESENT;
            pers_xfer_done = true;

            if (log_flags.file_xfer) {
                msg_printf(
                    fip->project, MSG_INFO,
                    "File %s exists already, skipping download", fip->name
                );
            }

            return 0;
        } else if (retval == ERR_IN_PROGRESS) {
            pers_xfer_done = true;
            return ERR_IN_PROGRESS;
        } else {
            // Mark file as not present but don't delete it.
            // It might be partly downloaded.
            //
            fip->status = FILE_NOT_PRESENT;
        }
    }

    // Decide whether to start a new file transfer
    //
    if (!gstate.start_new_file_xfer(*this)) {
        return ERR_IDLE_PERIOD;
    }

    URL_LIST& ul = fip->get_url_list(is_upload);
    file_xfer = new FILE_XFER;
    fxp = file_xfer;
    retval = start_xfer();
    if (!retval) retval = gstate.file_xfers->insert(file_xfer);
    if (retval) {
        if (log_flags.http_debug) {
            msg_printf(
                fip->project, MSG_INFO, "[file_xfer] Couldn't start %s of %s",
                (is_upload ? "upload" : "download"), fip->name
            );
            msg_printf(
                fip->project, MSG_INFO, "[file_xfer] URL %s: %s",
                ul.get_current_url(*fip), boincerror(retval)
            );
        }

        fxp->file_xfer_retval = retval;
        if (retval == ERR_HTTP_PERMANENT) {
            permanent_failure(retval);
        } else {
            transient_failure(retval);
        }
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
    if (log_flags.file_xfer_debug) {
        msg_printf(fip->project, MSG_INFO,
            "[file_xfer] URL: %s\n",
            ul.get_current_url(*fip)
        );
    }
    return 0;
}

// Poll the status of this persistent file transfer.
// If it's time to start it, then attempt to start it.
// If it has finished or failed:
//      handle the success or failure
//      remove the FILE_XFER from gstate.file_xfers and delete it
// Return true if it finished
//
bool PERS_FILE_XFER::poll() {
    if (pers_xfer_done) {
        return false;
    }
    if (!fxp) {
        // No file xfer is active.
        // Either initial or resume after failure.
        // See if it's time to try again.
        //
        if (gstate.now < next_request_time) {
            return false;
        }
        FILE_XFER_BACKOFF& fxb = fip->project->file_xfer_backoff(is_upload);
        if (!fxb.ok_to_transfer() && nretry>0) {
#if 0
            if (log_flags.file_xfer_debug) {
                msg_printf(fip->project, MSG_INFO,
                    "[file_xfer] delaying %s of %s: project-wide backoff %f sec",
                    is_upload?"upload":"download", fip->name,
                    fxb.next_xfer_time - gstate.now
                );
            }
#endif
            return false;
        }
        last_time = gstate.now;
        create_xfer();
        return false;
    }

    // copy bytes_xferred for use in GUI
    //
    last_bytes_xferred = fxp->bytes_xferred;
    if (is_upload) {
        last_bytes_xferred += fxp->file_offset;
    }

    // don't count suspended periods in total time
    //
    double diff = gstate.now - last_time;
    if (diff > 0 && diff <= 2) {
        time_so_far += diff;
    }
    last_time = gstate.now;

    if (fxp->file_xfer_done) {
        if (log_flags.file_xfer_debug) {
            msg_printf(fip->project, MSG_INFO,
                "[file_xfer] file transfer status %d (%s)",
                fxp->file_xfer_retval, boincerror(fxp->file_xfer_retval)
            );
        }
        switch (fxp->file_xfer_retval) {
        case 0:
            fip->project->file_xfer_backoff(is_upload).file_xfer_succeeded();
            if (log_flags.file_xfer) {
                // project files can have fip->nbytes == 0,
                // so use last_bytes_xferred as size
                //
                msg_printf(
                    fip->project, MSG_INFO, "Finished %s of %s (%.0f bytes)",
                    is_upload?"upload":"download",
                    fip->name, (fip->nbytes != 0 ? fip->nbytes : last_bytes_xferred)
                );
            }
            if (log_flags.file_xfer_debug) {
                if (fxp->xfer_speed < 0) {
                    msg_printf(fip->project, MSG_INFO, "[file_xfer] No data transferred");
                } else {
                    msg_printf(
                        fip->project, MSG_INFO, "[file_xfer] Throughput %d bytes/sec",
                        (int)fxp->xfer_speed
                    );
                }
            }
            pers_xfer_done = true;
            break;
        case ERR_UPLOAD_PERMANENT:
            permanent_failure(fxp->file_xfer_retval);
            break;
        case ERR_NOT_FOUND:
        case ERR_HTTP_PERMANENT:
            if (is_upload) {
                // if we get a "not found" on an upload,
                // the project must not have a file_upload_handler.
                // Treat this as a transient error.
                //
                transient_failure(fxp->file_xfer_retval);
            } else {
                permanent_failure(fxp->file_xfer_retval);
            }
            break;
        default:
            if (log_flags.file_xfer) {
                msg_printf(
                    fip->project, MSG_INFO, "Temporarily failed %s of %s: %s",
                    is_upload?"upload":"download", fip->name,
                    boincerror(fxp->file_xfer_retval)
                );
            }
            transient_failure(fxp->file_xfer_retval);
        }

        // If we transferred any bytes, or there are >1 URLs,
        // set upload_offset back to -1
        // so that we'll query file size on next retry.
        // Otherwise leave it as is, avoiding unnecessary size query.
        //
        if (last_bytes_xferred || (fip->upload_urls.urls.size() > 1)) {
            fip->upload_offset = -1;
        }

        // fxp could have already been freed and zeroed above
        // so check before trying to remove
        //
        if (fxp) {
            gstate.file_xfers->remove(fxp);
            delete fxp;
            fxp = NULL;
        }

        if (is_upload && !fip->project->uploading()) {
            gstate.request_work_fetch("project finished uploading");
        }

        return true;
    }
    return false;
}

void PERS_FILE_XFER::permanent_failure(int retval) {
    // Cycle to the next URL to try.
    // If we reach the URL that we started at or there is only one, then give up.
    //
    URL_LIST& ul = fip->get_url_list(is_upload);
    if (!ul.get_next_url()) {
        gstate.file_xfers->remove(fxp);
        delete fxp;
        fxp = NULL;
        fip->status = retval;
        pers_xfer_done = true;
        if (log_flags.file_xfer) {
            msg_printf(
                fip->project, MSG_INFO, "Giving up on %s of %s: %s",
                is_upload?"upload":"download", fip->name, boincerror(retval)
            );
        }
        fip->error_msg = boincerror(retval);
    }
}

// Handle a transient failure
//
void PERS_FILE_XFER::transient_failure(int retval) {

    // If it was a bad range request, delete the file and start over
    //
    if (retval == HTTP_STATUS_RANGE_REQUEST_ERROR) {
        fip->delete_file();
        return;
    }

    // If too much time has elapsed, give up
    //
    if ((gstate.now - first_request_time) > gstate.file_xfer_giveup_period) {
        permanent_failure(ERR_TIMEOUT);
    }

    // Cycle to the next URL to try.
    // If we reach the URL that we started at, then back off.
    //

    URL_LIST& ul = fip->get_url_list(is_upload);
    if (!ul.get_next_url()) {
        do_backoff();
    }
}

// per-file backoff policy: sets next_request_time
//
void PERS_FILE_XFER::do_backoff() {
    double backoff = 0;

    // don't count it as a server failure if network is down
    //
    if (!net_status.need_physical_connection) {
        nretry++;
    }

    // keep track of transient failures per project (not currently used)
    //
    PROJECT* p = fip->project;
    p->file_xfer_backoff(is_upload).file_xfer_failed(p);

    // Do an exponential backoff of e^nretry seconds,
    // keeping within the bounds of pers_retry_delay_min and
    // pers_retry_delay_max
    //
    backoff = calculate_exponential_backoff(
        nretry, gstate.pers_retry_delay_min, gstate.pers_retry_delay_max
    );
    next_request_time = gstate.now + backoff;
    msg_printf(fip->project, MSG_INFO,
        "Backing off %s on %s of %s",
        timediff_format(backoff).c_str(),
        is_upload?"upload":"download",
        fip->name
    );
}

void PERS_FILE_XFER::abort() {
    if (fxp) {
        gstate.file_xfers->remove(fxp);
        delete fxp;
        fxp = NULL;
    }
    fip->status = ERR_ABORTED_VIA_GUI;
    fip->error_msg = "user requested transfer abort";
    pers_xfer_done = true;
}

// Parse XML information about a persistent file transfer
//
int PERS_FILE_XFER::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/persistent_file_xfer")) return 0;
        else if (xp.parse_int("num_retries", nretry)) continue;
        else if (xp.parse_double("first_request_time", first_request_time)) {
            continue;
        }
        else if (xp.parse_double("next_request_time", next_request_time)) {
            continue;
        }
        else if (xp.parse_double("time_so_far", time_so_far)) continue;
        else if (xp.parse_double("last_bytes_xferred", last_bytes_xferred)) continue;
        else if (xp.parse_bool("is_upload", is_upload)) continue;
        else {
            if (log_flags.unparsed_xml) {
                msg_printf(NULL, MSG_INFO,
                    "[unparsed_xml] Unparsed line in file transfer info: %s",
                    xp.parsed_tag
                );
            }
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
        "        <last_bytes_xferred>%f</last_bytes_xferred>\n"
        "        <is_upload>%d</is_upload>\n"
        "    </persistent_file_xfer>\n",
        nretry,
        first_request_time,
        next_request_time,
        time_so_far,
        last_bytes_xferred,
        is_upload?1:0
    );

    // the following is for GUI RPCs
    //
    if (fxp) {
        fout.printf(
            "    <file_xfer>\n"
            "        <estimated_xfer_time_remaining>%f</estimated_xfer_time_remaining>\n"
            "        <bytes_xferred>%f</bytes_xferred>\n"
            "        <file_offset>%f</file_offset>\n"
            "        <xfer_speed>%f</xfer_speed>\n"
            "        <url>%s</url>\n"
            "    </file_xfer>\n",
            estimated_xfer_time_remaining(),
            fxp->bytes_xferred,
            fxp->file_offset,
            fxp->xfer_speed,
            fxp->m_url
        );
    }
    return 0;
}

// suspend file transfers by killing them.
// They'll restart automatically later.
//
void PERS_FILE_XFER::suspend() {
    if (fxp) {
        last_bytes_xferred = fxp->bytes_xferred;  // save bytes transferred
        if (fxp->is_upload) {
            last_bytes_xferred += fxp->file_offset;
        }
        gstate.file_xfers->remove(fxp);     // this removes from http_op_set too
        delete fxp;
        fxp = 0;
    }
    fip->upload_offset = -1;
}

// Determines the amount of time for a pfx to complete.  Returns time in seconds.
//
double PERS_FILE_XFER::estimated_xfer_time_remaining() {
    // The estimated transfer duration will be set to 0 (or, '---' as displayed in the Manager) in three conditions:
    // 1.  The pfx is complete.
    // 2.  The file has not started transferring.
    // 3.  If the transfer speed is 0.  This is for conditions when xfer_speed has not been calculated yet
    //      (either from the transfer returning from suspension or the BOINC starting up).
    if (pers_xfer_done || fxp==0 || fxp->xfer_speed==0) {
        return 0;
    }
    double bytes_remaining = (fip->nbytes - last_bytes_xferred);
    double est_duration = bytes_remaining / fxp->xfer_speed;
    if (est_duration <= 0) est_duration = 1;
    return est_duration;
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

    if (!gstate.clock_change && gstate.now - last_time < PERS_FILE_XFER_POLL_PERIOD) return false;
    last_time = gstate.now;

    // try to finish ones we've already started
    //
    for (i=0; i<pers_file_xfers.size(); i++) {
        if (!pers_file_xfers[i]->last_bytes_xferred) continue;
        action |= pers_file_xfers[i]->poll();
    }
    for (i=0; i<pers_file_xfers.size(); i++) {
        if (pers_file_xfers[i]->last_bytes_xferred) continue;
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
            iter = pers_file_xfers.erase(iter);
            return 0;
        }
        ++iter;
    }
    msg_printf(
        pfx->fip->project, MSG_INTERNAL_ERROR,
        "Persistent file transfer object not found"
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

// add a random delay 0..x to all transfers
// (used when emerging from bandwidth quota suspension)
//
void PERS_FILE_XFER_SET::add_random_delay(double x) {
    unsigned int i;
    double y = gstate.now + x*drand();
    for (i=0; i<pers_file_xfers.size(); i++) {
        if (y > pers_file_xfers[i]->next_request_time) {
            pers_file_xfers[i]->next_request_time = y;
        }
    }
}
