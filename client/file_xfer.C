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

#include "cpp.h"

#include "util.h"
#include "file_names.h"
#include "client_state.h"
#include "filesys.h"
#include "message.h"
#include "file_xfer.h"
#include "parse.h"
#include "error_numbers.h"

FILE_XFER::FILE_XFER() {
    file_xfer_done = false;
    file_xfer_retval = 0;
    fip = NULL;
    strcpy(pathname, "");
    strcpy(header, "");
    file_size_query = false;
}

FILE_XFER::~FILE_XFER() {
    if (fip && fip->pers_file_xfer) {
        fip->pers_file_xfer->fxp = NULL;
    }
}

int FILE_XFER::init_download(FILE_INFO& file_info) {
    double f_size;

    is_upload = false;
    fip = &file_info;
    get_pathname(fip, pathname);
    if (file_size(pathname, f_size)) {
        f_size = 0;
    }
    bytes_xferred = f_size;

    return HTTP_OP::init_get(fip->get_url(), pathname, false, (int)f_size);
}

// for uploads, we need to build a header with xml_signature etc.
// (see doc/upload.html)
// Do this in memory.
//
int FILE_XFER::init_upload(FILE_INFO& file_info) {  
    // If upload_offset < 0, we need to query the upload handler
    // for the offset information
    // TODO: give priority to unfinished upload if there are multiple choices
    //
    fip = &file_info;
    get_pathname(fip, pathname);

    is_upload = true;
    if (file_info.upload_offset < 0) { // NOTE: Should this be <=?
        bytes_xferred = 0;
        sprintf(header,
            "<data_server_request>\n"
            "    <core_client_major_version>%d</core_client_major_version>\n"
            "    <core_client_minor_version>%d</core_client_minor_version>\n"
            "    <get_file_size>%s</get_file_size>\n"
            "</data_server_request>\n",
            MAJOR_VERSION, MINOR_VERSION,
            file_info.name
        );
        file_size_query = true;
        return HTTP_OP::init_post2(fip->get_url(), header, NULL, 0);
    } else {
        bytes_xferred = file_info.upload_offset;
        sprintf(header,
            "<data_server_request>\n"
            "    <core_client_major_version>%d</core_client_major_version>\n"
            "    <core_client_minor_version>%d</core_client_minor_version>\n"
            "<file_upload>\n"
            "<file_info>\n"
            "%s"
            "<xml_signature>\n"
            "%s"
            "</xml_signature>\n"
            "</file_info>\n"
            "<nbytes>%.0f</nbytes>\n"
            "<offset>%.0f</offset>\n"
            "<data>\n",
            MAJOR_VERSION, MINOR_VERSION,
            file_info.signed_xml,
            file_info.xml_signature,
            file_info.nbytes,
            file_info.upload_offset
        );
        file_size_query = false;
        return HTTP_OP::init_post2(
            fip->get_url(), header, pathname, fip->upload_offset
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
        msg_printf(fip->project, MSG_ERROR, "Error on file upload: %s", buf);
    }

    return status;
}

// Create a new empty FILE_XFER_SET
//
FILE_XFER_SET::FILE_XFER_SET(HTTP_OP_SET* p) {
    http_ops = p;
}

// Insert a FILE_XFER object into the set
//
int FILE_XFER_SET::insert(FILE_XFER* fxp) {
    int retval;

    retval = http_ops->insert(fxp);
    if (retval) return retval;
    file_xfers.push_back(fxp);
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
            file_xfers.erase(iter);
            return 0;
        }
        iter++;
    }
    msg_printf(NULL, MSG_ERROR, "FILE_XFER_SET::remove(): not found\n");
    return 1;
}

// Run through the FILE_XFER_SET and determine if any of the file
// transfers are complete or had an error
//
bool FILE_XFER_SET::poll() {
    unsigned int i;
    FILE_XFER* fxp;
    bool action = false;

    ScopeMessages scope_messages(log_messages, ClientMessages::DEBUG_FILE_XFER);

    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        if (fxp->http_op_done()) {
            action = true;
            fxp->file_xfer_done = true;
            scope_messages.printf("FILE_XFER_SET::poll(): http op done; retval %d\n", fxp->http_op_retval);
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
                        printf("ERROR: file upload returned %d\n", fxp->file_xfer_retval);
                        fxp->fip->upload_offset = -1;
                    } else {
                        remove(fxp);
                        i--;

                        // if the server's file size is bigger than ours,
                        // something bad has happened (like a result
                        // got sent to multiple users).
                        // Pretend the file was successfully uploaded
                        //
                        if (fxp->fip->upload_offset >= fxp->fip->nbytes) {
                            fxp->file_xfer_done = true;
                            fxp->file_xfer_retval = 0;
                        } else {
                            // Restart the upload, using the newly obtained
                            // upload_offset
                            fxp->file_xfer_retval = fxp->init_upload(*fxp->fip);

                            if (!fxp->file_xfer_retval) {
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
				fxp->fip->error_msg = "Existing file too large; can't resume";
			}
        }
    }
    return action;
}
