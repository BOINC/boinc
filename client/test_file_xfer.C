// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "cpp.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "http.h"
#include "file_xfer.h"
#include "util.h"

#define DOWNLOAD_URL "http://localhost.localdomain/download/input"
#define UPLOAD_URL "http://localhost.localdomain/boinc-cgi/file_upload_handler"

// Skeleton class to force compilation
class PERS_FILE_XFER {
public:
    int write(FILE* fout);
    int parse(FILE* fin);
    PERS_FILE_XFER();
};

PERS_FILE_XFER::PERS_FILE_XFER() {
}

int main() {
    NET_XFER_SET nxs;
    HTTP_OP_SET hos(&nxs);
    FILE_XFER_SET fxs(&hos);
    int retval;
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
        strcpy(fi2.signed_xml,
            "    <name>uc_wu_1_0</name>\n"
            "    <generated_locally/>\n"
            "    <upload_when_present/>\n"
            "    <max_nbytes>10000</max_nbytes>\n"
            "    <url>http://localhost/upload/uc_wu_1_0</url>\n"
        );
        strcpy(fi2.xml_signature,
            "9d1f8152371c67af1d26b25db104014dbf7e9ad3b61fc8334ee06e01c7529b1a\n"
            "7681c3e7c7828525361a01040d1197147286085231ee5d2554e59ecb40b3e6a5\n"
            "afbaf00ff15bc5b1acf5aa6318bc84f2671a9502ada9c2ce37a9c45480a0e3b7\n"
            "b3dcb6c3bf09feaebc81b76063ef12b0031cf041eaef811166839533067b74f6\n"
            ".\n"
        );
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
	nxs.poll();
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
	boinc_sleep(1.);
    }
    printf("all done\n");
}

// Skeleton function to force compilation
int PERS_FILE_XFER::parse(FILE* fin) {
    return 0;
}

// Skeleton function to force compilation
int PERS_FILE_XFER::write(FILE* fout) {
    return 0;
}


const char *BOINC_RCSID_e3e5151a51 = "$Id$";
