// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#include <cstring>
#include <cstdlib>
#include <cassert>

#include "boinc_db.h"
#include "error_numbers.h"
#include "parse.h"
#include "sched_config.h"
#include "crypt.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

#define OUTFILE_MACRO   "<OUTFILE_"
#define UPLOAD_URL_MACRO      "<UPLOAD_URL/>"

// compute an XML signature element for some text
//
int generate_signature(
    char* signed_xml, char* signature_xml, R_RSA_PRIVATE_KEY& key
)  {
    DATA_BLOCK block, signature;
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    char buf[LARGE_BLOB_SIZE];
    int retval;

    block.data = (unsigned char*)signed_xml;
    block.len = strlen(signed_xml);
    signature.data = signature_buf;
    signature.len = SIGNATURE_SIZE_BINARY;
    retval = sign_block(block, key, signature);
    if (retval) return retval;
    sprint_hex_data(buf, signature);
#if 0
    printf("signing [\n%s]\n", signed_xml);
    printf("signature: [\n%s]\n", buf);
#endif
    sprintf(signature_xml,
        "<xml_signature>\n%s</xml_signature>\n", buf
    );
    return 0;
}

// At the end of every <file_info> element,
// add a signature of its contents up to that point.
//
int add_signatures(char* xml, R_RSA_PRIVATE_KEY& key) {
    char* p = xml, *q1, *q2, buf[LARGE_BLOB_SIZE], buf2[LARGE_BLOB_SIZE];;
    char signature[LARGE_BLOB_SIZE];
    int retval, len;

    while (1) {
        q1 = strstr(p, "<file_info>\n");
        if (!q1) break;
        q2 = strstr(q1, "</file_info>");
        if (!q2) {
            fprintf(stderr, "add_signatures: malformed XML: %s\n", xml);
            return ERR_XML_PARSE;
        }
        q1 += strlen("<file_info>\n");
        len = q2 - q1;
        memcpy(buf, q1, len);
        buf[len] = 0;
        retval = generate_signature(buf, signature, key);
        if (retval) return retval;
        strcpy(buf2, q2);
        strcpy(q1, buf);
        strcat(q1, signature);
        strcat(q1, buf2);
        p = q1;
    }
    return 0;
}

// remove file upload signatures from a result XML doc
//
int remove_signatures(char* xml) {
    char* p, *q;
    while (1) {
        p = strstr(xml, "<xml_signature>");
        if (!p) break;
        q = strstr(p, "</xml_signature>");
        if (!q) {
            fprintf(stderr, "remove_signatures: invalid XML:\n%s", xml);
            return ERR_XML_PARSE;
        }
        q += strlen("</xml_signature>\n");
        strcpy(p, q);
    }
    return 0;
}

// macro-substitute a result template:
// - replace OUTFILE_x with base_filename_x, etc.
// - add signatures for file uploads
//
// This is called only from the transitioner,
// to create a new result for a WU
//
int process_result_template(
    char* result_template,
    R_RSA_PRIVATE_KEY& key,
    char* base_filename,
    SCHED_CONFIG& config
) {
    char* p,*q;
    char temp[LARGE_BLOB_SIZE];
    char num;
    int i, retval;

    while (1) {
        p = strstr(result_template, OUTFILE_MACRO);
        if (p) {
            i = atoi(p+strlen(OUTFILE_MACRO));
            q = p+strlen(OUTFILE_MACRO);
            num = q[0];
            strcpy(temp, p+strlen(OUTFILE_MACRO)+1+2);
            strcpy(p, base_filename);
            strncat(p, &num, 1);
            strcat(p, temp);
            continue;
        }
        p = strstr(result_template, UPLOAD_URL_MACRO);
        if (p) {
            strcpy(temp, p+strlen(UPLOAD_URL_MACRO));
            strcpy(p, config.upload_url);
            strcat(p, temp);
            continue;
        }
        break;
    }
    if (!config.dont_generate_upload_certificates) {
        retval = add_signatures(result_template, key);
        if (retval) return retval;
    }
    return 0;
}
