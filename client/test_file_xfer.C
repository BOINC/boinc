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

#include <stdio.h>
#include <unistd.h>

#include "http.h"
#include "file_xfer.h"

#define DOWNLOAD_URL "http://localhost.localdomain/download/input"
#define UPLOAD_URL "http://localhost.localdomain/upload/test_out.html"

int main() {
    NET_XFER_SET nxs;
    HTTP_OP_SET hos(&nxs);
    FILE_XFER_SET fxs(&hos);
    int retval, n;

    FILE_XFER* fx1 = new FILE_XFER;
    retval = fx1->init_download(DOWNLOAD_URL, "test_fx_out");
    if (retval) {
        printf("init_download failed\n");
        exit(1);
    }
    FILE_XFER* fx2= new FILE_XFER;
    retval = fx2->init_upload(UPLOAD_URL, "test_fx_in");
    if (retval) {
        printf("init_upload failed\n");
        exit(1);
    }

    retval = fxs.insert(fx1);
    if (retval) {
        printf("insert failed\n");
        exit(1);
    }
    retval = fxs.insert(fx2);
    if (retval) {
        printf("insert failed\n");
        exit(1);
    }

    while (1) {
	nxs.poll(100000, n);
	hos.poll();
        fxs.poll();
	if (fx1 && fx1->file_xfer_done) {
	    printf("download done; status %d\n", fx1->file_xfer_retval);
	    fxs.remove(fx1);
            fx1 = 0;
	}
	if (fx2 && fx2->file_xfer_done) {
	    printf("upload done; status %d\n", fx2->file_xfer_retval);
	    fxs.remove(fx2);
            fx2 = 0;
	}
        if (!fx1 && !fx2) break;
	boinc_sleep(1);
    }
    printf("all done\n");
}
