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

#include "util.h"
#include "file_names.h"
#include "filesys.h"
#include "log_flags.h"
#include "file_xfer.h"
#include "error_numbers.h"

FILE_XFER::FILE_XFER() {
    file_xfer_done = false;
    file_xfer_retval = 0;
    start_time = 0;
    end_time = 0;
    fip = NULL;
    strcpy(pathname,"");
    strcpy(header,"");
    //state = ?
}

// Do we need to do anything special when destroying the FILE_XFER object?
//
FILE_XFER::~FILE_XFER() {
}

#if 0
// Is there any reason to keep this around?
int FILE_XFER::init_download(char* url, char* outfile) {
    if(url==NULL) {
        fprintf(stderr, "error: FILE_XFER.init_download: unexpected NULL pointer url\n");
        return ERR_NULL;
    }
    if(outfile==NULL) {
        fprintf(stderr, "error: FILE_XFER.init_download: unexpected NULL pointer outfile\n");
        return ERR_NULL;
    }

    return HTTP_OP::init_get(url, outfile, file_size);
}

// Is there any reason to keep this around?
int FILE_XFER::init_upload(char* url, char* infile) {
    if(url==NULL) {
        fprintf(stderr, "error: FILE_XFER.init_upload: unexpected NULL pointer url\n");
        return ERR_NULL;
    }
    if(outfile==NULL) {
        fprintf(stderr, "error: FILE_XFER.init_upload: unexpected NULL pointer infile\n");
        return ERR_NULL;
    }
    return HTTP_OP::init_put(url, infile);
}
#endif

int FILE_XFER::init_download(FILE_INFO& file_info) {
    int f_size;

    fip = &file_info;
    get_pathname(fip, pathname);
    if (file_size(pathname, f_size)) {
        f_size = 0;
    }
    return HTTP_OP::init_get(fip->get_url(), pathname, false, f_size);
}

// for uploads, we need to build a header with xml_signature etc.
// (see file_upload_handler.C for a spec)
// Do this in memory.
//
int FILE_XFER::init_upload(FILE_INFO& file_info) {
    // If upload_offset < 0, we need to query the upload handler
    // for the offset information
    // TODO: give priority to URL of unfinished upload if there
    // are multiple choices
    //
    fip = &file_info;
    get_pathname(fip, pathname);

    if (file_info.upload_offset < 0) {
        sprintf(header,
            "<file_size_req>%s</file_size_req>\n",
            file_info.name
        );
        return HTTP_OP::init_post2(fip->get_url(), header, NULL, 0);
    } else {
        sprintf(header,
            "<file_info>\n"
            "%s"
            "<xml_signature>\n"
            "%s"
            "</xml_signature>\n"
            "</file_info>\n"
            "<nbytes>%f</nbytes>\n"
            "<offset>%.0f</offset>\n"
            "<data>\n",
            file_info.signed_xml,
            file_info.xml_signature,
            file_info.nbytes,
            file_info.upload_offset
        );
        return HTTP_OP::init_post2(fip->get_url(), header, pathname, fip->upload_offset);
    }
}

// Returns the total time that the file xfer has taken
//
double FILE_XFER::elapsed_time() {
    return end_time - start_time;
}

// Create a new empty FILE_XFER_SET
//
FILE_XFER_SET::FILE_XFER_SET(HTTP_OP_SET* p) {
    if(p==NULL) {
        fprintf(stderr, "error: FILE_XFER_SET: unexpected NULL pointer p\n");
    }
    http_ops = p;
}

// Insert a FILE_XFER object into the set
//
int FILE_XFER_SET::insert(FILE_XFER* fxp) {
    int retval;
    if(fxp==NULL) {
        fprintf(stderr, "error: FILE_XFER_SET.insert: unexpected NULL pointer fxp\n");
        return ERR_NULL;
    }
    // record start time.
    // This could be made more accurate by omitting the connection
    // setup and initial request times.
    //
    fxp->start_time = dtime();
    retval = http_ops->insert(fxp);
    if (retval) return retval;
    file_xfers.push_back(fxp);
    return 0;
}

// Remove a FILE_XFER object from the set
//
int FILE_XFER_SET::remove(FILE_XFER* fxp) {
    vector<FILE_XFER*>::iterator iter;
    if(fxp==NULL) {
        fprintf(stderr, "error: FILE_XFER_SET.remove: unexpected NULL pointer fxp\n");
        return ERR_NULL;
    }
    http_ops->remove(fxp);

    iter = file_xfers.begin();
    while (iter != file_xfers.end()) {
        if (*iter == fxp) {
            file_xfers.erase(iter);
            return 0;
        }
        iter++;
    }
    fprintf(stderr, "FILE_XFER_SET::remove(): not found\n");
    return 1;
}

// Run through the FILE_XFER_SET and determine if any of the file
// transfers are complete or had an error
//
bool FILE_XFER_SET::poll() {
    unsigned int i;
    FILE_XFER* fxp;
    bool action = false;

    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        if (fxp->http_op_done()) {
            action = true;
            fxp->end_time = dtime();
            fxp->file_xfer_done = true;
            if (log_flags.file_xfer_debug) {
                printf("http retval: %d\n", fxp->http_op_retval);
            }
            if (fxp->http_op_retval == 200) {
                fxp->file_xfer_retval = 0;
            } else {
                // Remove the transfer from the set.  The actual object
                // will be removed later by it's associated PERS_FILE_XFER
                remove(fxp);
                fxp->file_xfer_retval = fxp->http_op_retval;
                i--;
            }
        }
    }
    return action;
}
