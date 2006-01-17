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

// The "policy" part of file transfer is here.
// The "mechanism" part is in pers_file_xfer.C and file_xfer.C
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "md5_file.h"
#include "crypt.h"
#include "util.h"
#include "filesys.h"
#include "error_numbers.h"

#include "file_names.h"
#include "client_types.h"
#include "client_state.h"
#include "client_msgs.h"
#include "file_xfer.h"
#include "log_flags.h"

#define MAX_TRANSFERS_PER_PROJECT   2

using std::vector;

// Decide whether to consider starting a new file transfer
//
bool CLIENT_STATE::start_new_file_xfer(PERS_FILE_XFER& pfx) {
    unsigned int i;
    int n;

    if (network_suspended) return false;

    // limit the number of file transfers per project
    //
    n = 0;
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        FILE_XFER* fxp = file_xfers->file_xfers[i];
        if (pfx.fip->project == fxp->fip->project) {
            n++;
        }
    }
    if (n >= MAX_TRANSFERS_PER_PROJECT) return false;
    return true;
}

// Make a directory for each of the projects in the client state
//
int CLIENT_STATE::make_project_dirs() {
    unsigned int i;
    int retval;
    for (i=0; i<projects.size(); i++) {
        retval = make_project_dir(*projects[i]);
        if (retval) return retval;
    }
    return 0;
}

// Verify the validity of a file,
// based on its size, its MD5 checksum or an RSA signature
// This is called
// 1) right after download is finished (CLIENT_STATE::handle_pers_file_xfers())
// 2) if a needed file is already on disk (PERS_FILE_XFER::start_xfer())
// 3) in checking whether a result's input files are available
//    (CLIENT_STATE::input_files_available()).
//    In this case "strictI is false,
//    and we just check existence and size (no checksum)
//
// If a failure occurs, set the file's "status" field.
// This will cause the app_version or workunit that used the file
// to error out (via APP_VERSION::had_download_failure()
// WORKUNIT::had_download_failure())
//
int FILE_INFO::verify_file(bool strict) {
    char cksum[64], pathname[256];
    bool verified;
    int retval;
    double size;

    get_pathname(this, pathname);

    // If the file isn't there at all, set status to FILE_NOT_PRESENT;
    // this will trigger a new download rather than erroring out
    //
    if (file_size(pathname, size)) {
        status = FILE_NOT_PRESENT;
        return ERR_FILE_MISSING;
    }

    if (gstate.global_prefs.dont_verify_images
        && is_image_file(name)
        && size>0
    ) {
        return 0;
    }

    if (nbytes && (nbytes != size) && (!log_flags.dont_check_file_sizes)) {
        status = ERR_WRONG_SIZE;
        return ERR_WRONG_SIZE;
    }

    if (!strict) return 0;

    if (signature_required) {
        if (!strlen(file_signature)) {
            msg_printf(project, MSG_ERROR,
                "Application file %s missing signature", name
            );
            msg_printf(project, MSG_ERROR,
                "BOINC cannot accept this file"
            );
            error_msg = "missing signature";
            status = ERR_NO_SIGNATURE;
            return ERR_NO_SIGNATURE;
        }
        retval = verify_file2(
            pathname, file_signature, project->code_sign_key, verified
        );
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "Signature verification error for %s",
                name
            );
            error_msg = "signature verification error";
            status = ERR_RSA_FAILED;
            return ERR_RSA_FAILED;
        }
        if (!verified) {
            msg_printf(project, MSG_ERROR,
                "Signature verification failed for %s",
               name
            );
            error_msg = "signature verification failed";
            status = ERR_RSA_FAILED;
            return ERR_RSA_FAILED;
        }
    } else if (strlen(md5_cksum)) {
        retval = md5_file(pathname, cksum, nbytes);
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "MD5 computation error for %s: %s\n",
                name, boincerror(retval)
            );
            error_msg = "MD5 computation error";
            status = retval;
            return retval;
        }
        if (strcmp(cksum, md5_cksum)) {
            msg_printf(project, MSG_ERROR,
                "MD5 check failed for %s", name
            );
            msg_printf(project, MSG_ERROR,
                "expected %s, got %s\n", md5_cksum, cksum
            );
            error_msg = "MD5 check failed";
            status = ERR_MD5_FAILED;
            return ERR_MD5_FAILED;
        }
    }
    return 0;
}

// scan all FILE_INFOs and PERS_FILE_XFERs.
// start and finish downloads and uploads as needed.
//
bool CLIENT_STATE::handle_pers_file_xfers() {
    unsigned int i;
    FILE_INFO* fip;
    PERS_FILE_XFER *pfx;
    bool action = false;
    int retval;
    static double last_time;

    if (gstate.now - last_time < 1.0) return false;
    last_time = gstate.now;

    // Look for FILE_INFOs for which we should start a transfer,
    // and make PERS_FILE_XFERs for them
    //
    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        pfx = fip->pers_file_xfer;
        if (pfx) continue;
        if (!fip->generated_locally && fip->status == FILE_NOT_PRESENT) {
            pfx = new PERS_FILE_XFER;
            pfx->init(fip, false);
            fip->pers_file_xfer = pfx;
            pers_file_xfers->insert(fip->pers_file_xfer);
            action = true;
        } else if (fip->upload_when_present && fip->status == FILE_PRESENT && !fip->uploaded) {
            pfx = new PERS_FILE_XFER;
            pfx->init(fip, true);
            fip->pers_file_xfer = pfx;
            pers_file_xfers->insert(fip->pers_file_xfer);
            action = true;

        }
    }

    // Scan existing PERS_FILE_XFERs, looking for those that are done,
    // and deleting them
    //
    vector<PERS_FILE_XFER*>::iterator iter;
    iter = pers_file_xfers->pers_file_xfers.begin();
    while (iter != pers_file_xfers->pers_file_xfers.end()) {
        pfx = *iter;

        // If the transfer finished, remove the PERS_FILE_XFER object
        // from the set and delete it
        //
        if (pfx->pers_xfer_done) {
            fip = pfx->fip;
            if (fip->generated_locally || fip->upload_when_present) {
                // file has been uploaded - delete if not sticky
                //
                if (!fip->sticky) {
                    fip->delete_file();
                }
                fip->uploaded = true;
                active_tasks.upload_notify_app(fip);
            } else if (fip->status >= 0) {
                // file transfer did not fail (non-negative status)

                // verify the file with RSA or MD5, and change permissions
                //
                retval = fip->verify_file(true);
                if (retval) {
                    msg_printf(fip->project, MSG_ERROR,
                        "Checksum or signature error for %s", fip->name
                    );
                    fip->status = retval;
                } else {
                    // Set the appropriate permissions depending on whether
                    // it's an executable or normal file
                    //
                    retval = fip->set_permissions();
                    fip->status = FILE_PRESENT;
                }

                // if it's a user file, reread prefs for running apps
                //
                if (fip->is_user_file) {
                    active_tasks.request_reread_prefs(fip->project);
                }
            }
            iter = pers_file_xfers->pers_file_xfers.erase(iter);
            delete pfx;
            action = true;
            // `delete pfx' should have set pfx->fip->pfx to NULL
            assert (fip == NULL || fip->pers_file_xfer == NULL);
        } else {
            iter++;
        }
    }

    return action;
}


const char *BOINC_RCSID_66410b3cab = "$Id$";
