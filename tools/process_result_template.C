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

// macro-substitute a result template file:
// - replace OUTFILE_x with base_filename_x,
// - WU_NAME with WU name
// - RESULT_NAME with result name
// - At the end of every <file_info> element, add a signature
//   of its contents up to that point.

#include <string.h>
#include <stdlib.h>

#include "db.h"
#include "parse.h"
#include "crypt.h"

#define WU_NAME_MACRO   "<WU_NAME/>"
#define RESULT_NAME_MACRO   "<RESULT_NAME/>"
#define OUTFILE_MACRO   "<OUTFILE_"
#define UPLOAD_URL_MACRO      "<UPLOAD_URL/>"
#define DOWNLOAD_URL_MACRO      "<DOWNLOAD_URL/>"
#define UPLOAD_URL      "http://localhost/boinc-cgi/file_upload_handler"
#define DOWNLOAD_URL      "http://localhost/download/"

int process_result_template(
    FILE* in, FILE* out,
    R_RSA_PRIVATE_KEY& key,
    char* base_filename, char* wu_name, char* result_name
) {
    char* p,*q, *signed_xml=strdup("");
    char buf[256], temp[256];
    unsigned char signature_buf[SIGNATURE_SIZE];
    DATA_BLOCK block, signature;
    char num;
    int i;
    bool found;


    while (fgets(buf, 256, in)) {

        // when we reach the end of a <file_info> element,
        // generate a signature for the contents thus far
        //
        if (match_tag(buf, "<file_info>")) {
            free(signed_xml);
            signed_xml = strdup("");
            fputs(buf, out);
            continue;
        }
        if (match_tag(buf, "</file_info>")) {
            block.data = (unsigned char*)signed_xml;
            block.len = strlen(signed_xml);
            signature.data = signature_buf;
            signature.len = SIGNATURE_SIZE;
            sign_block(block, key, signature);
            fprintf(out, "<xml_signature>\n");
            print_hex_data(out, signature);
#if 0
            printf("signing [\n%s]\n", signed_xml);
            printf("signature: [\n");
            print_hex_data(stdout, signature);
            printf("]\n");
#endif
            fprintf(out, "</xml_signature>\n");
            fprintf(out, "</file_info>\n");
            continue;
        }

        while (1) {
            found = false;
            p = strstr(buf, OUTFILE_MACRO);
            if (p) {
                found = true;
                i = atoi(p+strlen(OUTFILE_MACRO));
                q = p+strlen(OUTFILE_MACRO);
                num = q[0];
                strcpy(temp, p+strlen(OUTFILE_MACRO)+1+2);
                strcpy(p, base_filename);
                strncat(p, &num, 1);
                strcat(p, temp);
            }
            p = strstr(buf, UPLOAD_URL_MACRO);
            if (p) {
                found = true;
                strcpy(temp, p+strlen(UPLOAD_URL_MACRO));
                strcpy(p, UPLOAD_URL);
                strcat(p, temp);
            }
            p = strstr(buf, DOWNLOAD_URL_MACRO);
            if (p) {
                found = true;
                strcpy(temp, p+strlen(DOWNLOAD_URL_MACRO));
                strcpy(p, DOWNLOAD_URL);
                strcat(p, temp);
            }
            p = strstr(buf, WU_NAME_MACRO);
            if (p) {
                found = true;
                strcpy(temp, p+strlen(WU_NAME_MACRO));
                strcpy(p, wu_name);
                strcat(p, temp);
            }
            p = strstr(buf, RESULT_NAME_MACRO);
            if (p) {
                found = true;
                strcpy(temp, p+strlen(RESULT_NAME_MACRO));
                strcpy(p, result_name);
                strcat(p, temp);
            }
            if (!found) break;
        }
        strcatdup(signed_xml, buf);
        fputs(buf, out);
    }
    return 0;
}
