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
    char temp[LARGE_BLOB_SIZE], buf[256];
    int retval;

    while (1) {
        p = strstr(result_template, OUTFILE_MACRO);
        if (p) {
            q = p+strlen(OUTFILE_MACRO);
            char* endptr = strstr(q, "/>");
            if (!endptr) return ERR_XML_PARSE;
            if (strchr(q, '>') != endptr+1) return ERR_XML_PARSE;
            *endptr = 0;
            strcpy(buf, q);
            strcpy(temp, endptr+2);
            strcpy(p, base_filename);
            strcat(p, buf);
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

const char *BOINC_RCSID_e5e1e879f3 = "$Id$";
