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

#include <string.h>
#include <stdlib.h>

#include "db.h"

#define WU_NAME_MACRO   "<WU_NAME/>"
#define RESULT_NAME_MACRO   "<RESULT_NAME/>"
#define OUTFILE_MACRO   "<OUTFILE_"
#define UPLOAD_URL_MACRO      "<UPLOAD_URL/>"
#define DOWNLOAD_URL_MACRO      "<DOWNLOAD_URL/>"
#define UPLOAD_URL      "http://localhost/upload/"
#define DOWNLOAD_URL      "http://localhost/download/"

// replace OUTFILE_x with base_filename_x,
// WU_NAME with WU name
// RESULT_NAME with result name
//
int process_result_template(
    char* out, char* base_filename, char* wu_name, char* result_name
) {
    char* p,*q;
    char buf[MAX_BLOB_SIZE];
    char num;
    int i;
    bool found;

    while (1) {
        found = false;
        p = strstr(out, OUTFILE_MACRO);
        if (p) {
            found = true;
            i = atoi(p+strlen(OUTFILE_MACRO));
            q = p+strlen(OUTFILE_MACRO);
            num = q[0];
            strcpy(buf, p+strlen(OUTFILE_MACRO)+1+2);
            strcpy(p, base_filename);
            strncat(p, &num, 1);
            strcat(p, buf);
        }
        p = strstr(out, UPLOAD_URL_MACRO);
        if (p) {
            found = true;
            strcpy(buf, p+strlen(UPLOAD_URL_MACRO));
            strcpy(p, UPLOAD_URL);
            strcat(p, buf);
        }
        p = strstr(out, DOWNLOAD_URL_MACRO);
        if (p) {
            found = true;
            strcpy(buf, p+strlen(DOWNLOAD_URL_MACRO));
            strcpy(p, DOWNLOAD_URL);
            strcat(p, buf);
        }
        p = strstr(out, WU_NAME_MACRO);
        if (p) {
            found = true;
            strcpy(buf, p+strlen(WU_NAME_MACRO));
            strcpy(p, wu_name);
            strcat(p, buf);
        }
        p = strstr(out, RESULT_NAME_MACRO);
        if (p) {
            found = true;
            strcpy(buf, p+strlen(RESULT_NAME_MACRO));
            strcpy(p, result_name);
            strcat(p, buf);
        }
        if (!found) break;
    }
    return 0;
}
