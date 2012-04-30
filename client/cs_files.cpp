// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// The "policy" part of file transfer is here.
// The "mechanism" part is in pers_file_xfer.C and file_xfer.C
//

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "md5_file.h"
#include "crypt.h"
#include "str_util.h"
#include "filesys.h"
#include "cert_sig.h"
#include "error_numbers.h"

#include "async_file.h"
#include "file_names.h"
#include "client_types.h"
#include "client_state.h"
#include "client_msgs.h"
#include "file_xfer.h"
#include "project.h"
#include "sandbox.h"

using std::vector;

// Decide whether to consider starting a new file transfer
//
bool CLIENT_STATE::start_new_file_xfer(PERS_FILE_XFER& pfx) {
    unsigned int i;
    int ntotal=0, nproj=0;

    if (network_suspended) return false;
    if (file_xfers_suspended) return false;


    // limit the number of file transfers per project
    // (uploads and downloads are limited separately)
    //
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        FILE_XFER* fxp = file_xfers->file_xfers[i];
        if (pfx.is_upload == fxp->is_upload) {
            ntotal++;
            if (pfx.fip->project == fxp->fip->project) {
                nproj++;
            }
        }
    }
    if (nproj >= config.max_file_xfers_per_project) return false;
    if (ntotal >= config.max_file_xfers) return false;
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

// Is app signed by one of the Application Certifiers?
//
bool FILE_INFO::verify_file_certs() {
    char file[256];
    bool retval = false;

    if (!is_dir(CERTIFICATE_DIRECTORY)) return false;
    DIRREF dir = dir_open(CERTIFICATE_DIRECTORY);
    while (dir_scan(file, dir, sizeof(file))) {
        if (cert_verify_file(cert_sigs, file, CERTIFICATE_DIRECTORY)) {
            msg_printf(project, MSG_INFO,
                "Signature verified using certificate %s", file
            );
            retval = true;
            break;
        }
    }
    dir_close(dir);
    return retval;
}

#ifndef SIM
// Check the existence and/or validity of a file.
// Return 0 if it exists and is valid.
//
//  verify_contents
//      if true, validate the contents of the file based either on =
//      the digital signature of the file or its MD5 checksum.
//      Otherwise just check its existence and size.
//  show_errors
//      write log msg on failure
//  allow_async
//      whether the operation can be done asynchronously.
//      If this is true, and verify_contents is set
//      (i.e. we have to read the file)
//      and the file size is above a threshold,
//      then we do the operation asynchronously.
//      In this case the file status is set to FILE_VERIFY_PENDING
//      and we return ERR_IN_PROGRESS.
//      When the asynchronous op is complete,
//      the status is set to FILE_PRESENT.
//
// This is called
// 1) right after a download is finished
//		(CLIENT_STATE::create_and_delete_pers_file_xfers() in cs_files.cpp)
//		precondition: status is FILE_NOT_PRESENT
//		verify_contents: true
//      show_errors: true
//      allow_async: true
// 2) to see if a file marked as NOT_PRESENT is actually on disk
//		(PERS_FILE_XFER::create_xfer() in pers_file_xfer.cpp)
//		precondition: status is FILE_NOT_PRESENT
//		verify_contents: true
//      show_errors: false
//      allow_async: true
// 3) when checking whether a result's input files are available
//		(CLIENT_STATE::input_files_available( in cs_apps.cpp)).
//		precondition: status is FILE_PRESENT
//		verify_contents: either true or false
//      show_errors: true
//      allow_async: false
//
// If a failure occurs, set the file's "status" field to an error number.
// This will cause the app_version or workunit that used the file to error out
// (via APP_VERSION::had_download_failure() or WORKUNIT::had_download_failure())
//
int FILE_INFO::verify_file(
    bool verify_contents, bool show_errors, bool allow_async
) {
    char cksum[64], pathname[256];
    bool verified;
    int retval;
    double size, local_nbytes;

    if (log_flags.async_file_debug) {
        msg_printf(project, MSG_INFO, "[async] verify file (%s): %s",
            verify_contents?"strict":"not strict", name
        );
    }

	if (status == FILE_VERIFY_PENDING) return ERR_IN_PROGRESS;

    get_pathname(this, pathname, sizeof(pathname));

    strcpy(cksum, "");

    // see if we need to unzip it
    //
    if (download_gzipped && !boinc_file_exists(pathname)) {
        char gzpath[256];
        sprintf(gzpath, "%s.gz", pathname);
        if (boinc_file_exists(gzpath) ) {
			if (allow_async && nbytes > ASYNC_FILE_THRESHOLD) {
				ASYNC_VERIFY* avp = new ASYNC_VERIFY;
				retval = avp->init(this);
                if (retval) {
                    status = retval;
                    return retval;
                }
				status = FILE_VERIFY_PENDING;
				return ERR_IN_PROGRESS;
			}
            retval = gunzip(cksum);
            if (retval) return retval;
        } else {
            strcat(gzpath, "t");
            if (!boinc_file_exists(gzpath)) {
                status = FILE_NOT_PRESENT;
            }
            return ERR_FILE_MISSING;
        }
    }


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

    if (nbytes && (nbytes != size) && (!config.dont_check_file_sizes)) {
        if (show_errors) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "File %s has wrong size: expected %.0f, got %.0f",
                name, nbytes, size
            );
        }
        status = ERR_WRONG_SIZE;
        return ERR_WRONG_SIZE;
    }

    if (!verify_contents) return 0;

    if (signature_required) {
        if (!strlen(file_signature) && !cert_sigs) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Application file %s missing signature", name
            );
            msg_printf(project, MSG_INTERNAL_ERROR,
                "BOINC cannot accept this file"
            );
            error_msg = "missing signature";
            status = ERR_NO_SIGNATURE;
            return ERR_NO_SIGNATURE;
        }
        if (config.use_certs || config.use_certs_only) {
            if (verify_file_certs()) {
                verified = true;
                return 0;
            }
        }
        if (config.use_certs_only) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Unable to verify %s using certificates", name
            );
            return ERR_NO_SIGNATURE;
        }
		if (allow_async && nbytes > ASYNC_FILE_THRESHOLD) {
			ASYNC_VERIFY* avp = new ASYNC_VERIFY();
			retval = avp->init(this);
            if (retval) {
                status = retval;
                return retval;
            }
			status = FILE_VERIFY_PENDING;
			return ERR_IN_PROGRESS;
		}
        if (!strlen(cksum)) {
            double file_length;
            retval = md5_file(pathname, cksum, file_length);
            if (retval) {
                status = retval;
                msg_printf(project, MSG_INFO,
                    "md5_file failed for %s: %s",
                    pathname, boincerror(retval)
                );
                return retval;
            }
        }
        retval = check_file_signature2(
            cksum, file_signature, project->code_sign_key, verified
        );
        if (retval) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Signature verification error for %s",
                name
            );
            error_msg = "signature verification error";
            status = ERR_RSA_FAILED;
            return ERR_RSA_FAILED;
        }
        if (!verified && show_errors) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "Signature verification failed for %s",
               name
            );
            error_msg = "signature verification failed";
            status = ERR_RSA_FAILED;
            return ERR_RSA_FAILED;
        }
    } else if (strlen(md5_cksum)) {
        if (!strlen(cksum)) {
			if (allow_async && nbytes > ASYNC_FILE_THRESHOLD) {
				ASYNC_VERIFY* avp = new ASYNC_VERIFY();
				retval = avp->init(this);
                if (retval) {
                    status = retval;
                    return retval;
                }
				status = FILE_VERIFY_PENDING;
				return ERR_IN_PROGRESS;
			}
            retval = md5_file(pathname, cksum, local_nbytes);
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "MD5 computation error for %s: %s\n",
                    name, boincerror(retval)
                );
                error_msg = "MD5 computation error";
                status = retval;
                return retval;
            }
        }
        if (strcmp(cksum, md5_cksum)) {
            if (show_errors) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "MD5 check failed for %s", name
                );
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "expected %s, got %s\n", md5_cksum, cksum
                );
            }
            error_msg = "MD5 check failed";
            status = ERR_MD5_FAILED;
            return ERR_MD5_FAILED;
        }
    }
    return 0;
}

// scan FILE_INFOs and create PERS_FILE_XFERs as needed.
// NOTE: this doesn't start the file transfers
// scan PERS_FILE_XFERs and delete finished ones.
//
bool CLIENT_STATE::create_and_delete_pers_file_xfers() {
    unsigned int i;
    FILE_INFO* fip;
    PERS_FILE_XFER *pfx;
    bool action = false;
    int retval;
    static double last_time;

    if (now - last_time < PERS_FILE_XFER_START_PERIOD) return false;
    last_time = now;

    // Look for FILE_INFOs for which we should start a transfer,
    // and make PERS_FILE_XFERs for them
    //
    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        pfx = fip->pers_file_xfer;
        if (pfx) continue;
        if (fip->downloadable() && fip->status == FILE_NOT_PRESENT) {
            pfx = new PERS_FILE_XFER;
            pfx->init(fip, false);
            fip->pers_file_xfer = pfx;
            pers_file_xfers->insert(fip->pers_file_xfer);
            action = true;
        } else if (fip->uploadable() && fip->status == FILE_PRESENT && !fip->uploaded) {
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
            if (pfx->is_upload) {
                // file has been uploaded - delete if not sticky
                //
                if (!fip->sticky) {
                    fip->delete_file();
                }
                fip->uploaded = true;
                active_tasks.upload_notify_app(fip);
            } else if (fip->status >= 0) {
                // file transfer did not fail (non-negative status)

                // If this was a compressed download, rename .gzt to .gz
                //
                if (fip->download_gzipped) {
                    char path[256], from_path[256], to_path[256];
                    get_pathname(fip, path, sizeof(path));
                    sprintf(from_path, "%s.gzt", path);
                    sprintf(to_path, "%s.gz", path);
                    boinc_rename(from_path, to_path);
                }

                // verify the file with RSA or MD5, and change permissions
                //
                retval = fip->verify_file(true, true, true);
				if (retval == ERR_IN_PROGRESS) {
					// do nothing
				} else if (retval) {
                    msg_printf(fip->project, MSG_INTERNAL_ERROR,
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

                // if it's a user file, tell running apps to reread prefs
                //
                if (fip->is_user_file) {
                    active_tasks.request_reread_prefs(fip->project);
                }

                // if it's a project file, make a link in project dir
                //
                if (fip->is_project_file) {
                    PROJECT* p = fip->project;
                    p->write_symlink_for_project_file(fip);
                    p->update_project_files_downloaded_time();
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
#endif

// called at startup to ensure that if the core client
// thinks a file is there, it actually is, and is the right size
//
void CLIENT_STATE::check_file_existence() {
    unsigned int i;
    char path[1024];

    for (i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if (fip->status == FILE_PRESENT) {
            get_pathname(fip, path, sizeof(path));
            double size;
            int retval = file_size(path, size);
            if (retval || (fip->nbytes && (size != fip->nbytes))) {
                delete_project_owned_file(path, true);
                fip->status = FILE_NOT_PRESENT;
                msg_printf(NULL, MSG_INFO, "file %s not found", path);
            }
        }
    }
}

