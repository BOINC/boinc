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

// Verify the validity of a downloaded file, through MD5 checksum
// or an RSA signature
//
int verify_downloaded_file(char* pathname, FILE_INFO& file_info) {
    char cksum[64];
    PROJECT* project;
    bool verified;
    int retval;

    if(pathname==NULL) {
        fprintf(stderr, "error: verify_downloaded_file: unexpected NULL pointer pathname\n");
        return ERR_NULL;
    }
    if (file_info.signature_required) {
        if (!file_info.file_signature) {
            fprintf(stderr, "error: verify_downloaded_file: unexpected NULL pointer file_signature\n");
            return -1;
        }
        project = file_info.project;
        retval = verify_file2(
            pathname, file_info.file_signature, project->code_sign_key, verified
        );
        if(retval) {
            fprintf(stderr, "error: verify_file2: internal error\n");
            //return -1;
        }
        if(!verified) {
            fprintf(stderr, "error: verify_file2: file not verified\n");
            //return -1;
        }
        if (retval || !verified) {
            fprintf(stderr, "error: verify_file2: could not verify file\n");
            return -1;
        }
    } else if (file_info.md5_cksum) {
        md5_file(pathname, cksum, file_info.nbytes);
        if (strcmp(cksum, file_info.md5_cksum)) return -1;
    }
    return 0;
}

// scan all FILE_INFOs.
// start downloads and uploads as needed.
//
bool CLIENT_STATE::start_file_xfers() {
    unsigned int i;
    FILE_INFO* fip;
    PERS_FILE_XFER *pfx;
    bool action = false;

    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        pfx = fip->pers_file_xfer;
        if (!fip->generated_locally && !fip->file_present && !pfx) {
            // Set up the persistent file transfer object.  This will start
            // the download when there is available bandwidth
            //
            pfx = new PERS_FILE_XFER;
            pfx->init( fip, false );
            fip->pers_file_xfer = pfx;
            // Pop PERS_FILE_XFER onto pers_file_xfer stack
            if (fip->pers_file_xfer) pers_xfers->insert( fip->pers_file_xfer );
            action = true;
        } else if ( fip->upload_when_present && fip->file_present && !fip->uploaded && !pfx ) {
            // Set up the persistent file transfer object.  This will start
            // the upload when there is available bandwidth
            //
            pfx = new PERS_FILE_XFER;
            pfx->init( fip, true );
            fip->pers_file_xfer = pfx;
            // Pop PERS_FILE_XFER onto pers_file_xfer stack
            if (fip->pers_file_xfer) pers_xfers->insert( fip->pers_file_xfer );
            action = true;
        }
    }
    return action;
}
