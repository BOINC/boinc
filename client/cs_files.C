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

#include <sys/stat.h>

#include "md5_file.h"
#include "file_xfer.h"
#include "file_names.h"
#include "client_types.h"
#include "log_flags.h"
#include "client_state.h"

bool CLIENT_STATE::start_file_xfers() {
    unsigned int i;
    FILE_INFO* fip;
    FILE_XFER* fxp;
    char pathname[256];
    char cksum[64];
    int retval;
    bool action = false;

    // scan all FILE_INFOs.
    // start downloads and uploads as needed.
    // check for completion of existing transfers
    //
    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        fxp = fip->file_xfer;
        if (!fip->generated_locally && !fip->file_present && !fxp) {
            fxp = new FILE_XFER;
            get_pathname(fip, pathname);
            fxp->init_download( fip->url, pathname);
            retval = file_xfers->insert(fxp);
            if (retval) {
                fprintf(stderr,
                    "couldn't start download for %s: error %d\n",
                    fip->url, retval
                );
            } else {
                fip->file_xfer = fxp;
                if (log_flags.file_xfer) {
                    printf("started download of %s to %s\n", fip->url, pathname);
                }
            }
            action = true;
        } else if (
            fip->upload_when_present && fip->file_present
            && !fip->uploaded && !fxp
        ) {
            fxp = new FILE_XFER;
            get_pathname(fip, pathname);
            fxp->init_upload( fip->url, pathname);
            retval = file_xfers->insert(fxp);
            if (retval) {
                fprintf(stderr,
                    "couldn't start upload for %s: error %d\n",
                    fip->url, retval
                );
            } else {
                fip->file_xfer = fxp;
                if (log_flags.file_xfer) {
                    printf("started upload of %s to %s\n", pathname, fip->url);
                }
            }
            action = true;
        } else if (fxp) {
            if (fxp->file_xfer_done) {
                action = true;
                if (log_flags.file_xfer) {
                    printf(
                        "file transfer done for %s; retval %d\n",
                        fip->url, fxp->file_xfer_retval
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
                        get_pathname(fip, pathname);
                        md5_file(pathname, cksum, fip->nbytes);
                        net_stats.update(false, fip->nbytes, fxp->elapsed_time());
                        if (strcmp(cksum, fip->md5_cksum)) {
                            fprintf(stderr,
                                "checksum error for %s\n", fip->name
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
