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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "db.h"
#include "crypt.h"
#include "md5_file.h"

#include "backend_lib.h"

#define INFILE_MACRO    "<INFILE_"
#define MD5_MACRO       "<MD5_"
#define WU_NAME_MACRO   "<WU_NAME/>"
#define RESULT_NAME_MACRO   "<RESULT_NAME/>"
#define OUTFILE_MACRO   "<OUTFILE_"
#define UPLOAD_URL_MACRO      "<UPLOAD_URL/>"
#define DOWNLOAD_URL_MACRO      "<DOWNLOAD_URL/>"

int read_file(FILE* f, char* buf) {
    assert(f);
    assert(buf);
    int n = fread(buf, 1, MAX_BLOB_SIZE, f);
    buf[n] = 0;
    return 0;
}

int read_filename(char* path, char* buf) {
    int retval;
    assert(path);
    assert(buf);
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    retval = read_file(f, buf);
    fclose(f);
    return retval;
}

// replace INFILE_x with filename from array,
// MD5_x with checksum of file,
// WU_NAME with WU name
//
static int process_wu_template(
    char* wu_name, char* tmplate, char* out,
    char* dirpath, char** infiles, int n
) {
    char* p;
    char buf[MAX_BLOB_SIZE], md5[33], path[256];
    bool found;
    int i;
    double nbytes;
    assert(wu_name!=NULL);
    assert(tmplate!=NULL);
    assert(out!=NULL);
    assert(dirpath!=NULL);
    assert(infiles!=NULL);
    assert(n>=0);
    strcpy(out, tmplate);
    while (1) {
        found = false;
        p = strstr(out, INFILE_MACRO);
        if (p) {
            found = true;
            i = atoi(p+strlen(INFILE_MACRO));
            if (i >= n) {
                fprintf(stderr, "invalid file number\n");
                return 1;
            }
            strcpy(buf, p+strlen(INFILE_MACRO)+1+2);      // assume <= 10 files
            strcpy(p, infiles[i]);
            strcat(p, buf);
        }
        p = strstr(out, UPLOAD_URL_MACRO);
        if (p) {
            found = true;
            strcpy(buf, p+strlen(UPLOAD_URL_MACRO));
            strcpy(p, getenv("BOINC_UPLOAD_URL"));
            strcat(p, buf);
        }
        p = strstr(out, DOWNLOAD_URL_MACRO);
        if (p) {
            found = true;
            strcpy(buf, p+strlen(DOWNLOAD_URL_MACRO));
            strcpy(p, getenv("BOINC_DOWNLOAD_URL"));
            strcat(p, buf);
        }
        p = strstr(out, MD5_MACRO);
        if (p) {
            found = true;
            i = atoi(p+strlen(MD5_MACRO));
            if (i >= n) {
                fprintf(stderr, "invalid file number\n");
                return 1;
            }
            sprintf(path, "%s/%s", dirpath, infiles[i]);
            md5_file(path, md5, nbytes);
            strcpy(buf, p+strlen(MD5_MACRO)+1+2);     // assume <= 10 files
            strcpy(p, md5);
            strcat(p, buf);
        }
        p = strstr(out, WU_NAME_MACRO);
        if (p) {
            found = true;
            strcpy(buf, p+strlen(WU_NAME_MACRO));
            strcpy(p, wu_name);
            strcat(p, buf);
        }
        if (!found) break;
    }
    return 0;
}

int create_result(
    WORKUNIT& wu, char* result_template_filename, int i, R_RSA_PRIVATE_KEY& key
) {
    RESULT r;
    char base_outfile_name[256];
    int retval;
    FILE* result_template_file, *tempfile;
    assert(result_template_filename!=NULL);
    memset(&r, 0, sizeof(r));
    r.report_deadline = time(0) + 1000;
        // TODO: pass this in
    r.create_time = time(0);
    r.workunitid = wu.id;
    r.state = RESULT_STATE_UNSENT;
    sprintf(r.name, "%s_%d", wu.name, i);
    sprintf(base_outfile_name, "%s_", r.name);

    result_template_file = fopen(result_template_filename, "r");
    tempfile = tmpfile();
    retval = process_result_template(
        result_template_file,
        tempfile,
        key,
        base_outfile_name, wu.name, r.name
    );
    rewind(tempfile);
    read_file(tempfile, r.xml_doc_in);
    fclose(tempfile);

    retval = db_result_new(r);
    if (retval) {
        fprintf(stderr, "db_result_new: %d\n", retval);
    }
    return retval;
}

int create_work(
    WORKUNIT& wu,
    char* wu_template,
    char* result_template_file,
    int nresults,
    char* infile_dir,
    char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY& key
) {
    int i, retval;
    assert(wu_template!=NULL);
    assert(result_template_file!=NULL);
    assert(nresults>=0);
    assert(infile_dir!=NULL);
    assert(infiles!=NULL);
    assert(ninfiles>=0);
    wu.create_time = time(0);
    retval = process_wu_template(
        wu.name, wu_template, wu.xml_doc, infile_dir, infiles, ninfiles
    );
    if (retval) return retval;
    if (wu.dynamic_results) {
        wu.max_results = nresults;
    } else {
        wu.nresults_unsent = nresults;
    }
    retval = db_workunit_new(wu);
    wu.id = db_insert_id();

    if (!wu.dynamic_results) {
        for (i=0; i<nresults; i++) {
            create_result(wu, result_template_file, i, key);
        }
    }
    return 0;
}
