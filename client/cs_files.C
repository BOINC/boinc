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

// functions relating to file transfer
//

#include "windows_cpp.h"

#ifdef _WIN32
#include <io.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include "md5_file.h"
#include "crypt.h"

#include "file_xfer.h"
#include "file_names.h"
#include "client_types.h"
#include "log_flags.h"
#include "client_state.h"
#include "error_numbers.h"

// Make a directory for each of the projects present
// in the client state
//
int CLIENT_STATE::make_project_dirs() {
    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        make_project_dir(*projects[i]);
    }
    return 0;
}

// Verify the validity of a downloaded file, through MD5 checksum
// or an RSA signature
//
int verify_downloaded_file(char* pathname, FILE_INFO& file_info) {
    char cksum[64];
    PROJECT* project;
    bool verified;
    int retval;

    if (file_info.signature_required) {
        project = file_info.project;
        retval = verify_file2(
            pathname, file_info.file_signature, project->code_sign_key, verified
        );
        if (retval) {
            fprintf(stderr, "error: verify_file2: internal error\n");
            return ERR_RSA_FAILED;
        }
        if (!verified) {
            fprintf(stderr, "error: verify_file2: file not verified\n");
            return ERR_RSA_FAILED;
        }
    } else if (file_info.md5_cksum) {
        md5_file(pathname, cksum, file_info.nbytes);
        if (strcmp(cksum, file_info.md5_cksum)) {
            fprintf(stderr, "error: verify_file2: MD5 check failed\n");
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

    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        pfx = fip->pers_file_xfer;
        if (!fip->generated_locally && fip->status == FILE_NOT_PRESENT && !pfx) {
            // Set up the persistent file transfer object.  This will start
            // the download when there is available bandwidth
            //
            pfx = new PERS_FILE_XFER;
            pfx->init(fip, false);
            fip->pers_file_xfer = pfx;
	    pers_xfers->insert( fip->pers_file_xfer );
            action = true;
        } else if (fip->upload_when_present && fip->status == FILE_PRESENT && !fip->uploaded && !pfx ) {
            // Set up the persistent file transfer object.  This will start
            // the upload when there is available bandwidth
            //
            pfx = new PERS_FILE_XFER;
            pfx->init( fip, true );
            fip->pers_file_xfer = pfx;
            pers_xfers->insert(fip->pers_file_xfer);
            action = true;
        }
    }

    for (i=0; i<pers_xfers->pers_file_xfers.size(); i++) {
        pfx = pers_xfers->pers_file_xfers[i];
        // If the transfer finished, remove the PERS_FILE_XFER object
        // from the set and delete it
        if (pfx->xfer_done) {
            pfx->fip->pers_file_xfer = NULL;
            pers_xfers->remove(pfx);
            delete pfx;
            action = true;
        }
    }
    
    return action;
}
