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
#include "log_flags.h"
#include "file_xfer.h"

FILE_XFER::FILE_XFER() {
    file_xfer_done = false;
    file_xfer_retval = 0;
}

FILE_XFER::~FILE_XFER() {
}

#if 0
int FILE_XFER::init_download(char* url, char* outfile) {
    return HTTP_OP::init_get(url, outfile);
}

int FILE_XFER::init_upload(char* url, char* infile) {
    return HTTP_OP::init_put(url, infile);
}
#endif

int FILE_XFER::init_download(FILE_INFO& file_info) {
    fip = &file_info;
    get_pathname(fip, pathname);
    return HTTP_OP::init_get((char*)(&fip->urls[0]), pathname);
}

// for uploads, we need to build a header with signature etc.
// (see file_upload_handler.C for a spec)
// Do this in memory.
//
int FILE_XFER::init_upload(FILE_INFO& file_info) {
    fip = &file_info;
    get_pathname(fip, pathname);
    sprintf(header,
        "<file_info>\n"
        "%s"
        "<signature>\n"
        "%s"
        "</signature>\n"
        "</file_info>\n"
        "<nbytes>%f</nbytes>\n"
        "<offset>0</offset>\n"
        "<data>\n",
        file_info.signed_xml,
        file_info.signature,
        file_info.nbytes
    );
    return HTTP_OP::init_post2((char*)(&fip->urls[0]), header, pathname, 0);
}

double FILE_XFER::elapsed_time() {
    return end_time - start_time;
}

FILE_XFER_SET::FILE_XFER_SET(HTTP_OP_SET* p) {
    http_ops = p;
}

int FILE_XFER_SET::insert(FILE_XFER* fxp) {
    int retval;

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
    fprintf(stderr, "FILE_XFER_SET::remove(): not found\n");
    return 1;

}

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
                fxp->file_xfer_retval = fxp->http_op_retval;
            }
        }
    }
    return action;
}
