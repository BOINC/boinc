// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#include "config.h"
#include <cstring>
#include <string>
#include <cstdlib>
#include <cassert>

#include "boinc_db.h"
#include "error_numbers.h"
#include "parse.h"
#include "sched_config.h"
#include "str_replace.h"
#include "crypt.h"
#include "boinc_stdio.h"

#define OUTFILE_MACRO   "<OUTFILE_"
#define UPLOAD_URL_MACRO      "<UPLOAD_URL/>"

// Add a signature at the end of every <file_info> element,
//
int add_signatures(char* xml, R_RSA_PRIVATE_KEY& key) {
    char* p = xml, *q1, *q2, buf[BLOB_SIZE], buf2[BLOB_SIZE];
    char signature_hex[BLOB_SIZE];
    char signature_xml[BLOB_SIZE];
    char signed_xml[1024];
    int retval, len;

    while (1) {
        q1 = strstr(p, "<file_info>\n");
        if (!q1) break;
        q2 = strstr(q1, "</file_info>");
        if (!q2) {
            boinc::fprintf(stderr, "add_signatures: malformed XML: %s\n", xml);
            return ERR_XML_PARSE;
        }

        q1 += strlen("<file_info>\n");
        len = q2 - q1;
        memcpy(buf, q1, len);
        buf[len] = 0;
        char name[1024];
        if (!parse_str(buf, "<name>", name, sizeof(name))) {
            boinc::fprintf(stderr, "add_signatures: missing name: %s", buf);
            return ERR_XML_PARSE;
        }
        double max_nbytes;
        if (!parse_double(buf, "<max_nbytes>", max_nbytes)) {
            boinc::fprintf(stderr, "add_signatures: missing max_nbytes: %s", buf);
            return ERR_XML_PARSE;
        }
        sprintf(signed_xml, "<name>%s</name><max_nbytes>%.0f</max_nbytes>",
            name, max_nbytes
        );
        retval = generate_signature(signed_xml, signature_hex, key);
        sprintf(signature_xml,
            "<xml_signature>\n%s</xml_signature>\n", signature_hex
        );
        if (retval) return retval;
        safe_strcpy(buf2, q2);
        strcpy(q1, buf);
        strcat(q1, signature_xml);
        strcat(q1, buf2);
        p = q1;
    }
    return 0;
}

#if 0   // is this used anywhere??

// remove file upload signatures from a result XML doc
//
int remove_signatures(char* xml) {
    char* p, *q;
    while (1) {
        p = strstr(xml, "<xml_signature>");
        if (!p) break;
        q = strstr(p, "</xml_signature>");
        if (!q) {
            boinc::fprintf(stderr, "remove_signatures: invalid XML:\n%s", xml);
            return ERR_XML_PARSE;
        }
        q += strlen("</xml_signature>\n");
        strcpy(p, q);
    }
    return 0;
}
#endif

// macro-substitute a result template:
// - replace OUTFILE_x with base_filename_x, etc.
// - add signatures for file uploads
// - strip enclosing <output_template> tags
//
// This is called only from the transitioner,
// to create a new result for a WU
//
int process_result_template(
    char* result_template,
    R_RSA_PRIVATE_KEY& key,
    char* base_filename,
    SCHED_CONFIG& config_loc
) {
    char* p,*q;
    char temp[BLOB_SIZE], buf[256];
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
            strcpy(p, config_loc.upload_url);
            strcat(p, temp);
            continue;
        }
        break;
    }
    if (!config_loc.dont_generate_upload_certificates) {
        retval = add_signatures(result_template, key);
        if (retval) return retval;
    }

    // strip enclosing <output_template> tags, if any
    //
    p = strstr(result_template, "<output_template>");
    if (p) {
        safe_strcpy(temp, result_template+strlen("<output_template>"));
        q = strstr(temp, "</output_template>");
        if (q) *q = 0;
        strcpy(result_template, temp);
    }
    strip_whitespace(result_template);
    return 0;
}
