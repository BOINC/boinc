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
// check for completion of existing transfers
//
bool CLIENT_STATE::start_file_xfers() {
    unsigned int i;
    FILE_INFO* fip;
    FILE_XFER* fxp;
    char pathname[256];
    int retval;
    bool action = false;

    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        fxp = fip->file_xfer;
        if (!fip->generated_locally && !fip->file_present && !fxp) {
            fxp = new FILE_XFER;
            fxp->init_download(*fip);
            retval = file_xfers->insert(fxp);
            if (retval) {
                fprintf(stderr,
                    "couldn't start download for %s: error %d\n",
                    fip->urls[0].text, retval
                );
            } else {
                fip->file_xfer = fxp;
                if (log_flags.file_xfer) {
                    printf(
                        "started download of %s\n",
                        fip->urls[0].text
                    );
                }
            }
            action = true;
        } else if (
            fip->upload_when_present && fip->file_present
            && !fip->uploaded && !fxp
        ) {
            fxp = new FILE_XFER;
            fxp->init_upload(*fip);
            retval = file_xfers->insert(fxp);
            if (retval) {
                fprintf(stderr,
                    "couldn't start upload for %s: error %d\n",
                    fip->urls[0].text, retval
                );
            } else {
                fip->file_xfer = fxp;
                if (log_flags.file_xfer) {
                    printf("started upload to %s\n", fip->urls[0].text);
                }
            }
            action = true;
        } else if (fxp) {
            if (fxp->file_xfer_done) {
                action = true;
                if (log_flags.file_xfer) {
                    printf(
                        "file transfer done for %s; retval %d\n",
                        fip->urls[0].text, fxp->file_xfer_retval
                    );
                }
                file_xfers->remove(fxp);
                fip->file_xfer = 0;
                if (fip->generated_locally) {
                    if (fxp->file_xfer_retval == 0) {
                        net_stats.update(true, fip->nbytes, fxp->elapsed_time());

                        // file has been uploaded - delete if not sticky
                        if (!fip->sticky) {
                            fip->delete_file();
                        }
                        fip->uploaded = true;
                    }
                } else {
                    if (fxp->file_xfer_retval == 0) {
                        net_stats.update(false, fip->nbytes, fxp->elapsed_time());
                        get_pathname(fip, pathname);
                        retval = verify_downloaded_file(pathname, *fip);
                        if (retval) {
                            fprintf(stderr,
                                "checksum or signature error for %s\n", fip->name
                            );
                        } else {
                            if (log_flags.file_xfer_debug) {
                                printf("MD5 checksum validated for %s\n", pathname);
                            }
                            if (fip->executable) {
                                retval = chmod(pathname, S_IEXEC|S_IREAD|S_IWRITE);
                            } else {
                                get_pathname(fip, pathname);
                                retval = chmod(pathname, S_IREAD|S_IWRITE);
                            }
                            fip->file_present = true;
                        }
                    }
                }
                client_state_dirty = true;
                delete fxp;
            }
        }
    }
    return action;
}
