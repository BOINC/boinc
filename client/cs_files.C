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

// The "policy" part of file transfer is here.
// The "mechanism" part is in pers_file_xfer.C and file_xfer.C
//

#include "cpp.h"

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "md5_file.h"
#include "crypt.h"

#include "filesys.h"
#include "error_numbers.h"
#include "file_names.h"
#include "client_types.h"
#include "client_state.h"
#include "client_msgs.h"
#include "file_xfer.h"

#define MAX_TRANSFERS_PER_PROJECT   2

// Decide whether to consider starting a new file transfer
//
bool CLIENT_STATE::start_new_file_xfer(PERS_FILE_XFER& pfx) {
    unsigned int i;
    int n;

    if (activities_suspended || network_suspended) return false;

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

void CLIENT_STATE::trunc_stderr_stdout() {
    double f_size;

    fflush(stdout);
    fflush(stderr);

    // If the stderr.txt or stdout.txt files are too big, reset them
    // TODO: should we tell the user we're resetting these?
    file_size(STDERR_FILE_NAME, f_size);
    if (f_size > MAX_STDERR_FILE_SIZE) {
        freopen(STDERR_FILE_NAME, "w", stderr);
    }
    file_size(STDOUT_FILE_NAME, f_size);
    if (f_size > MAX_STDOUT_FILE_SIZE) {
        freopen(STDOUT_FILE_NAME, "w", stdout);
    }
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

// Verify the validity of a downloaded file,
// through MD5 checksum or an RSA signature
//
int verify_downloaded_file(char* pathname, FILE_INFO& file_info) {
    char cksum[64];
    PROJECT* project = file_info.project;
    bool verified;
    int retval;

    if (file_info.signature_required) {
        if (!file_info.file_signature) {
            fprintf(stdout, "ERROR: file %s missing signature\n", file_info.name);
            return ERR_NO_SIGNATURE;
        }
        retval = verify_file2(
            pathname, file_info.file_signature, project->code_sign_key, verified
        );
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "verify_downloaded_file(): %s: internal error\n",
                pathname
            );
            return ERR_RSA_FAILED;
        }
        if (!verified) {
            msg_printf(project, MSG_ERROR,
                "verify_downloaded_file(): %s: file not verified\n",
                pathname
            );
            return ERR_RSA_FAILED;
        }
    } else if (strlen(file_info.md5_cksum)) {
        retval = md5_file(pathname, cksum, file_info.nbytes);
        if (retval) {
            msg_printf(project, MSG_ERROR,
                "verify_downloaded_file(): %s: MD5 computation failed: %d\n",
                pathname, retval
            );
            return retval;
        }
        if (strcmp(cksum, file_info.md5_cksum)) {
            msg_printf(project, MSG_ERROR,
                "verify_downloaded_file(): %s: MD5 check failed\n", pathname
            );
            msg_printf(project, MSG_ERROR,
                "expected %s, got %s\n", file_info.md5_cksum, cksum
            );
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
    char pathname[256];
    int retval;

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
        if (pfx->xfer_done) {
            fip = pfx->fip;
            if (fip->generated_locally) {
                // file has been uploaded - delete if not sticky
                //
                if (!fip->sticky) {
                    fip->delete_file();
                }
                fip->uploaded = true;
            } else {

                // verify the file with RSA or MD5, and change permissions
                //
                get_pathname(fip, pathname);
                retval = verify_downloaded_file(pathname, *fip);
                if (retval) {
                    msg_printf(fip->project, MSG_ERROR, "Checksum or signature error for %s", fip->name);
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
