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
#include "util.h"

#define DOWNLOAD_URL "http://localhost.localdomain/download/input"
#define UPLOAD_URL "http://localhost.localdomain/boinc-cgi/file_upload_handler"


int main() {
    NET_XFER_SET nxs;
    HTTP_OP_SET hos(&nxs);
    FILE_XFER_SET fxs(&hos);
    int retval, n;
    FILE_XFER* fx1=0, *fx2 = 0;
    bool do_upload = true;
    bool do_download = false;
    FILE_INFO fi1, fi2;
    STRING256 str;

    if (do_download) {
        fx1 = new FILE_XFER;
        memset(&fi1, 0, sizeof(fi1));
        strcpy(fi1.name, "test_fx_out");
        strcpy(str.text, DOWNLOAD_URL);
        fi1.urls.push_back(str);
        retval = fx1->init_download(fi1);
        if (retval) {
            printf("init_download failed\n");
            exit(1);
        }
        retval = fxs.insert(fx1);
        if (retval) {
            printf("insert failed\n");
            exit(1);
        }
    }

    if (do_upload) {
        fx2= new FILE_XFER;
        memset(&fi2, 0, sizeof(fi1));
        strcpy(fi2.name, "test_fx_in");
        strcpy(str.text, UPLOAD_URL);
        fi2.urls.push_back(str);
        fi2.signed_xml = \
            "    <name>uc_wu_1_0</name>\n"
            "    <generated_locally/>\n"
            "    <upload_when_present/>\n"
            "    <max_nbytes>10000</max_nbytes>\n"
            "    <url>http://localhost/upload/uc_wu_1_0</url>\n";
        fi2.xml_signature = \
            "9d1f8152371c67af1d26b25db104014dbf7e9ad3b61fc8334ee06e01c7529b1a\n"
            "7681c3e7c7828525361a01040d1197147286085231ee5d2554e59ecb40b3e6a5\n"
            "afbaf00ff15bc5b1acf5aa6318bc84f2671a9502ada9c2ce37a9c45480a0e3b7\n"
            "b3dcb6c3bf09feaebc81b76063ef12b0031cf041eaef811166839533067b74f6\n"
            ".\n";
        retval = fx2->init_upload(fi2);
        if (retval) {
            printf("init_upload failed\n");
            exit(1);
        }
        retval = fxs.insert(fx2);
        if (retval) {
            printf("insert failed\n");
            exit(1);
        }
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
