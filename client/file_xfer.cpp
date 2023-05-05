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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "error_numbers.h"
#include "str_replace.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"
#include "util.h"

#include "client_state.h"
#include "client_msgs.h"
#include "file_xfer.h"
#include "project.h"

using std::vector;

FILE_XFER::FILE_XFER() {
    file_xfer_done = false;
    file_xfer_retval = 0;
    fip = NULL;
    safe_strcpy(pathname, "");
    safe_strcpy(header, "");
    file_size_query = false;
    is_upload = false;
    starting_size = 0.0;
}

FILE_XFER::~FILE_XFER() {
    if (fip && fip->pers_file_xfer) {
        fip->pers_file_xfer->fxp = NULL;
    }
}

int FILE_XFER::init_download(FILE_INFO& file_info) {
    is_upload = false;
    fip = &file_info;

    // create subdirs of project dir if needed
    //
    int retval = boinc_make_dirs(fip->project->project_dir(), fip->name);
    if (retval) return retval;

    get_pathname(fip, pathname, sizeof(pathname));
    if (fip->download_gzipped) {
        safe_strcat(pathname, ".gzt");
    }

    // if file is already as large or larger than it's supposed to be,
    // something's screwy; start reading it from the beginning.
    //
    if (file_size(pathname, starting_size) || starting_size >= fip->nbytes) {
        starting_size = 0;
    }
    bytes_xferred = starting_size;

    const char* url = fip->download_urls.get_current_url(file_info);
    if (!url) return ERR_INVALID_URL;
    return HTTP_OP::init_get(
        file_info.project, url, pathname, false, starting_size, file_info.nbytes
    );
}

// for uploads, we need to build a header with xml_signature etc.
// (see doc/upload.php)
// Do this in memory.
//
int FILE_XFER::init_upload(FILE_INFO& file_info) {
    // If upload_offset < 0, we need to query the upload handler
    // for the offset information
    //
    fip = &file_info;
    get_pathname(fip, pathname, sizeof(pathname));
    if (!boinc_file_exists(pathname)) {
        return ERR_NOT_FOUND;
    }

    is_upload = true;

    // skip file size check if file is small
    //
    if (fip->nbytes < FILE_SIZE_CHECK_THRESHOLD) {
        fip->upload_offset = 0;
    }

    if (log_flags.file_xfer_debug) {
        msg_printf(file_info.project, MSG_INFO,
            "[fxd] starting upload, upload_offset %.0f", file_info.upload_offset
        );
    }
    if (file_info.upload_offset < 0) {
        bytes_xferred = 0;
        snprintf(header, sizeof(header),
            "<data_server_request>\n"
            "    <core_client_major_version>%d</core_client_major_version>\n"
            "    <core_client_minor_version>%d</core_client_minor_version>\n"
            "    <core_client_release>%d</core_client_release>\n"
            "    <get_file_size>%s</get_file_size>\n"
            "</data_server_request>\n",
            BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, BOINC_RELEASE,
            file_info.name
        );
        file_size_query = true;
        const char* url = fip->upload_urls.get_current_url(file_info);
        if (!url) return ERR_INVALID_URL;
        return HTTP_OP::init_post2(
            file_info.project, url, header, sizeof(header), NULL, 0
        );
    } else {
        bytes_xferred = file_info.upload_offset;
        snprintf(header, sizeof(header),
            "<data_server_request>\n"
            "    <core_client_major_version>%d</core_client_major_version>\n"
            "    <core_client_minor_version>%d</core_client_minor_version>\n"
            "    <core_client_release>%d</core_client_release>\n"
            "<file_upload>\n"
            "<file_info>\n"
            "<name>%s</name>\n"
            "<xml_signature>\n"
            "%s"
            "</xml_signature>\n"
            "<max_nbytes>%.0f</max_nbytes>\n"
            "</file_info>\n"
            "<nbytes>%.0f</nbytes>\n"
            "<md5_cksum>%s</md5_cksum>\n"
            "<offset>%.0f</offset>\n"
            "<data>\n",
            BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, BOINC_RELEASE,
            file_info.name,
            file_info.xml_signature,
            file_info.max_nbytes,
            file_info.nbytes,
            file_info.md5_cksum,
            file_info.upload_offset
        );
        file_size_query = false;
        const char* url = fip->upload_urls.get_current_url(file_info);
        if (!url) return ERR_INVALID_URL;
        return HTTP_OP::init_post2(
            file_info.project, url, header, sizeof(header),
            pathname, fip->upload_offset
        );
    }
}

// Parse the file upload handler response in req1
//
int FILE_XFER::parse_upload_response(double &nbytes) {
    int status = ERR_UPLOAD_TRANSIENT, x;
    char buf[256];

    nbytes = -1;
    parse_double(req1, "<file_size>", nbytes);
    if (parse_int(req1, "<status>", x)) {
        switch (x) {
        case -1: status = ERR_UPLOAD_PERMANENT; break;
        case 0: status = 0; break;
        case 1: status = ERR_UPLOAD_TRANSIENT; break;
        default: status = ERR_UPLOAD_TRANSIENT; break;
        }
    } else {
        status = ERR_UPLOAD_TRANSIENT;
    }

    if (parse_str(req1, "<message>", buf, sizeof(buf))) {
        msg_printf(fip->project, MSG_INTERNAL_ERROR,
            "Error reported by file upload server: %s", buf
        );
    }
    if (log_flags.file_xfer_debug) {
        msg_printf(fip->project, MSG_INFO,
            "[file_xfer] parsing upload response: %s", req1
        );
        msg_printf(fip->project, MSG_INFO,
            "[file_xfer] parsing status: %d", status
        );
    }

    return status;
}

// Create a new empty FILE_XFER_SET
//
FILE_XFER_SET::FILE_XFER_SET(HTTP_OP_SET* p) {
    http_ops = p;
    up_active = false;
    down_active = false;
}

// start a FILE_XFER going (connect to server etc.)
// If successful, add to the set
//
int FILE_XFER_SET::insert(FILE_XFER* fxp) {
    http_ops->insert(fxp);
    file_xfers.push_back(fxp);
    enforce_bandwidth_limits(fxp->is_upload);
    return 0;
}

// Remove a FILE_XFER object from the set
//
int FILE_XFER_SET::remove(FILE_XFER* fxp) {
    vector<FILE_XFER*>::iterator iter;

    http_ops->remove(fxp);

    iter = file_xfers.begin();
    while (iter != file_xfers.end()) {
        if (*iter == fxp) {
            iter = file_xfers.erase(iter);
            enforce_bandwidth_limits(fxp->is_upload);
            return 0;
        }
        ++iter;
    }
    msg_printf(fxp->fip->project, MSG_INTERNAL_ERROR,
        "File transfer for %s not found", fxp->fip->name
    );
    return ERR_NOT_FOUND;
}

// Run through the FILE_XFER_SET and determine if any of the file
// transfers are complete or had an error
//
bool FILE_XFER_SET::poll() {
    unsigned int i;
    FILE_XFER* fxp;
    bool action = false;
    static double last_time=0;
    char pathname[256];
    double size;

    if (!gstate.clock_change && gstate.now - last_time < FILE_XFER_POLL_PERIOD) return false;
    last_time = gstate.now;

    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        if (!fxp->http_op_done()) continue;

        action = true;
        fxp->file_xfer_done = true;
        if (log_flags.file_xfer_debug) {
            msg_printf(fxp->fip->project, MSG_INFO,
                "[file_xfer] http op done; retval %d (%s)\n",
                fxp->http_op_retval, boincerror(fxp->http_op_retval)
            );
        }
        fxp->file_xfer_retval = fxp->http_op_retval;
        if (fxp->file_xfer_retval == 0) {
            if (fxp->is_upload) {
                fxp->file_xfer_retval = fxp->parse_upload_response(
                    fxp->fip->upload_offset
                );
            }

            // If this was a file size query, restart the transfer
            // using the remote file size information
            //
            if (fxp->file_size_query) {
                if (fxp->file_xfer_retval) {
                    fxp->fip->upload_offset = -1;
                } else {

                    // if the server's file size is bigger than ours,
                    // something bad has happened
                    // (like a result got sent to multiple users).
                    // Pretend the file was successfully uploaded
                    //
                    if (fxp->fip->upload_offset >= fxp->fip->nbytes) {
                        fxp->file_xfer_done = true;
                        fxp->file_xfer_retval = 0;
                    } else {
                        // Restart the upload, using the newly obtained
                        // upload_offset
                        //
                        fxp->close_socket();
                        fxp->file_xfer_retval = fxp->init_upload(*fxp->fip);

                        if (!fxp->file_xfer_retval) {
                            remove(fxp);
                            i--;
                            fxp->file_xfer_retval = insert(fxp);
                            if (!fxp->file_xfer_retval) {
                                fxp->file_xfer_done = false;
                                fxp->file_xfer_retval = 0;
                                fxp->http_op_retval = 0;
                            }
                        }
                    }
                }
            }
        } else if (fxp->file_xfer_retval == HTTP_STATUS_RANGE_REQUEST_ERROR) {
            fxp->fip->error_msg = "Local copy is at least as large as server copy";
        }

        // deal with various error cases for downloads
        //
        if (!fxp->is_upload) {
            get_pathname(fxp->fip, pathname, sizeof(pathname));
            if (file_size(pathname, size)) continue;
            double diff = size - fxp->starting_size;
            if (fxp->http_op_retval == 0) {
                // If no HTTP error,
                // see if we read less than 5 KB and file is incomplete.
                // If so truncate the amount read,
                // since it may be a proxy error message
                //
                if (fxp->fip->nbytes) {
                    if (size == fxp->fip->nbytes) continue;

                    // but skip this check if it's an image file
                    // and user has image verification disabled
                    // (i.e. they're behind a proxy that shrinks images)
                    // The shrunk image could be < 5 KB
                    //
                    if (is_image_file(pathname) && gstate.global_prefs.dont_verify_images) {
                        continue;
                    }

                    if (diff>0 && diff<MIN_DOWNLOAD_INCREMENT) {
                        msg_printf(fxp->fip->project, MSG_INFO,
                            "Incomplete read of %f < 5KB for %s - truncating",
                            diff, fxp->fip->name
                        );
                        boinc_truncate(pathname, fxp->starting_size);
                    }
                }
            } else {
                // got HTTP error; truncate last 5KB of file, since some
                // error-reporting HTML may have been appended
                //
                if (diff < MIN_DOWNLOAD_INCREMENT) {
                    diff = 0;
                } else {
                    diff -= MIN_DOWNLOAD_INCREMENT;
                }
                boinc_truncate(pathname, fxp->starting_size + diff);
            }
        }

        // for downloads: if we requested a partial transfer,
        // and the HTTP response is 200,
        // and the file is larger than it should be,
        // the server or proxy must have sent us the entire file
        // (i.e. it doesn't understand Range: requests).
        // In this case, trim off the initial part of the file
        //
        if (!fxp->is_upload && fxp->starting_size
            && fxp->response==HTTP_STATUS_OK
        ) {
            get_pathname(fxp->fip, pathname, sizeof(pathname));
            if (file_size(pathname, size)) continue;
            if (size > fxp->fip->nbytes) {
                FILE* f1 = boinc_fopen(pathname, "rb");
                if (!f1) {
                    fxp->file_xfer_retval = ERR_FOPEN;
                    msg_printf(fxp->fip->project, MSG_INTERNAL_ERROR,
                        "File size mismatch, can't open %s", pathname
                    );
                    continue;
                }
                FILE* f2 = boinc_fopen(TEMP_FILE_NAME, "wb");
                if (!f2) {
                    msg_printf(fxp->fip->project, MSG_INTERNAL_ERROR,
                        "File size mismatch, can't open temp %s", TEMP_FILE_NAME
                    );
                    fxp->file_xfer_retval = ERR_FOPEN;
                    fclose(f1);
                    continue;
                }
                fseek(f1, (long)fxp->starting_size, SEEK_SET);
                copy_stream(f1, f2);
                fclose(f1);
                fclose(f2);
                f1 = boinc_fopen(TEMP_FILE_NAME, "rb");
                f2 = boinc_fopen(pathname, "wb");
                copy_stream(f1, f2);
                fclose(f1);
                fclose(f2);
            }
        }
    }
    return action;
}

// return true if an upload is currently in progress
// or has been since the last call to this.
// Similar for download.
//
void FILE_XFER_SET::check_active(bool& up, bool& down) {
    unsigned int i;
    FILE_XFER* fxp;

    up = up_active;
    down = down_active;
    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        fxp->is_upload?up=true:down=true;
    }
    up_active = false;
    down_active = false;
}

// if bandwidth limit exists, divide bandwidth equally among active xfers.
// Called on prefs change if limit exists, and on xfer start/stop
//
void FILE_XFER_SET::enforce_bandwidth_limits(bool is_upload) {
    double max_bytes_sec = is_upload ? gstate.global_prefs.max_bytes_sec_up
        : gstate.global_prefs.max_bytes_sec_down;
    if (max_bytes_sec == 0) return;
    int nxfers = 0;
    unsigned int i;
    FILE_XFER* fxp;
    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        if (!fxp->is_active()) continue;
        if (is_upload) {
            if (!fxp->is_upload) continue;
        } else {
            if (fxp->is_upload) continue;
        }
        nxfers++;
    }
    if (nxfers == 0) return;
    max_bytes_sec /= nxfers;
    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        if (!fxp->is_active()) continue;
        if (is_upload) {
            if (!fxp->is_upload) continue;
            fxp->set_speed_limit(true, max_bytes_sec);
        } else {
            if (fxp->is_upload) continue;
            fxp->set_speed_limit(false, max_bytes_sec);
        }
    }
}

// clear bandwidth limit on active xfers.
// called on prefs change if no limit
//
void FILE_XFER_SET::clear_bandwidth_limits(bool is_upload) {
    unsigned int i;
    FILE_XFER* fxp;
    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        if (!fxp->is_active()) continue;
        if (is_upload) {
            if (!fxp->is_upload) continue;
            fxp->set_speed_limit(true, 0);
        } else {
            if (fxp->is_upload) continue;
            fxp->set_speed_limit(false, 0);
        }
    }
}

// called on prefs change
//
void FILE_XFER_SET::set_bandwidth_limits(bool is_upload) {
    double max_bytes_sec = is_upload ? gstate.global_prefs.max_bytes_sec_up
        : gstate.global_prefs.max_bytes_sec_down;

    if (max_bytes_sec == 0) {
        clear_bandwidth_limits(is_upload);
    } else {
        enforce_bandwidth_limits(is_upload);
    }
}

