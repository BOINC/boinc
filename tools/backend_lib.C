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
// The Original Code is the Berkeley Open Infrastructure for Network Computing. // 
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
#include "parse.h"

#include "backend_lib.h"

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

int read_key_file(char* keyfile, R_RSA_PRIVATE_KEY& key) {
    int retval;
    FILE* fkey = fopen(keyfile, "r");
    if (!fkey) {
        fprintf(stderr, "can't open key file (%s)\n", keyfile);
        return -1;
    }
    retval = scan_key_hex(fkey, (KEY*)&key, sizeof(key));
    fclose(fkey);
    if (retval) {
        fprintf(stderr, "can't parse key\n");
        return -1;
    }
    return 0;
}


// process WU template
//
static int process_wu_template(
    char* wu_name, char* tmplate, char* out,
    char* dirpath, char** infiles, int n,
    char* upload_url, char* download_url
) {
    char* p;
    char buf[MAX_BLOB_SIZE], md5[33], path[256];
    int retval, file_number;
    double nbytes;
    char open_name[256];

    assert(wu_name!=NULL);
    assert(tmplate!=NULL);
    assert(out!=NULL);
    assert(dirpath!=NULL);
    assert(infiles!=NULL);
    assert(n>=0);

    strcpy(out, "");
    p = strtok(tmplate, "\n");
    while (p) {
        if (match_tag(p, "<file_info>")) {
        } else if (parse_int(p, "<number>", file_number)) {
        } else if (match_tag(p, "</file_info>")) {
            sprintf(path, "%s/%s", dirpath, infiles[file_number]);
            retval = md5_file(path, md5, nbytes);
            if (retval) {
                fprintf(stderr, "process_wu_template: md5_file %d\n", retval);
                return 1;
            }
            sprintf(buf,
                "<file_info>\n"
                "    <name>%s</name>\n"
                "    <url>%s/%s</url>\n"
                "    <md5_cksum>%s</md5_cksum>\n"
                "    <nbytes>%.0f</nbytes>\n"
                "</file_info>\n",
                infiles[file_number],
                download_url, infiles[file_number],
                md5,
                nbytes
            );
            strcat(out, buf);
        } else if (match_tag(p, "<workunit>")) {
            strcat(out, "<workunit>\n");
        } else if (match_tag(p, "</workunit>")) {
            strcat(out, "</workunit>\n");
        } else if (match_tag(p, "<file_ref>")) {
        } else if (parse_int(p, "<file_number>", file_number)) {
        } else if (parse_str(p, "<open_name>", open_name, sizeof(open_name))) {
        } else if (match_tag(p, "</file_ref>")) {
            sprintf(buf,
                "<file_ref>\n"
                "    <file_name>%s</file_name>\n"
                "    <open_name>%s</open_name>\n"
                "</file_ref>\n",
                infiles[file_number],
                open_name
            );
            strcat(out, buf);
        } else {
            strcat(out, p);
            strcat(out, "\n");
        }
        p = strtok(0, "\n");
    }
    return 0;
}

// Set the time-varying fields of a result to their initial state.
// This is used to create clones of existing results,
// so set only the time-varying fields
//
void initialize_result(RESULT& result, WORKUNIT& wu) {
    result.id = 0;
    result.create_time = time(0);
    result.workunitid = wu.id;
    result.server_state = RESULT_SERVER_STATE_UNSENT;
    result.hostid = 0;
    result.report_deadline = time(0) + wu.delay_bound;
    result.sent_time = 0;
    result.received_time = 0;
    result.client_state = 0;
    result.cpu_time = 0;
    strcpy(result.xml_doc_out, "");
    strcpy(result.stderr_out, "");
    result.project_state = 0;
    result.validate_state = VALIDATE_STATE_INITIAL;
    result.claimed_credit = 0;
    result.granted_credit = 0;
}

// Create a new result for the given WU.
//
int create_result(
    WORKUNIT& wu, char* result_template,
    char* result_name_suffix, R_RSA_PRIVATE_KEY& key,
    char* upload_url, char* download_url
) {
    RESULT r;
    char base_outfile_name[256];
    char result_template_copy[MAX_BLOB_SIZE];
    int retval;

    memset(&r, 0, sizeof(r));
    initialize_result(r, wu);
    sprintf(r.name, "%s_%s", wu.name, result_name_suffix);
    sprintf(base_outfile_name, "%s_", r.name);

    strcpy(result_template_copy, result_template);
    retval = process_result_template(
        result_template_copy,
        key,
        base_outfile_name,
        upload_url, download_url
    );
    strcpy(r.xml_doc_in, result_template_copy);

    retval = db_result_new(r);
    if (retval) {
        fprintf(stderr, "db_result_new: %d\n", retval);
    }
    return retval;
}

int create_work(
    WORKUNIT& wu,
    char* wu_template,
    char* result_template_filename,
    int nresults,
    char* infile_dir,
    char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY& key,
    char* upload_url, char* download_url
) {
    int i, retval;
    char suffix[256];
    char result_template[MAX_BLOB_SIZE];

    wu.create_time = time(0);
    retval = process_wu_template(
        wu.name, wu_template, wu.xml_doc, infile_dir, infiles, ninfiles,
        upload_url, download_url
    );
    if (retval) {
        fprintf(stderr, "process_wu_template: %d\n", retval);
        return retval;
    }
    retval = db_workunit_new(wu);
    if (retval) {
        fprintf(stderr, "create_work: db_workunit_new %d\n", retval);
        return retval;
    }
    wu.id = db_insert_id();

    retval = read_filename(result_template_filename, result_template);
    if (retval) {
        fprintf(stderr, "create_work: can't read result template\n");
        return retval;
    }
    for (i=0; i<nresults; i++) {
        sprintf(suffix, "%d", i);
        retval = create_result(
            wu, result_template, suffix, key, upload_url, download_url
        );
        if (retval) {
            fprintf(stderr, "create_result: %d\n", retval);
            break;
        }
    }
    return 0;
}

int create_sequence(
    WORKUNIT& wu,
    char* wu_template,
    char* result_template_filename,
    int redundancy,
    char* infile_dir,
    char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY& key,
    char* upload_url, char* download_url,
    int nsteps
) {
    int i, retval;
    WORKSEQ ws;

    retval = db_workseq_new(ws);
    if (retval) return retval;
    for (i=0; i<nsteps; i++) {
        // to be completed
    }
    return 0;
}

int create_sequence_group(
    WORKUNIT& wu,
    char* wu_template,
    char* result_template_filename,
    int redundancy,
    char* infile_dir,
    char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY& key,
    char* upload_url, char* download_url,
    int nsteps
) {
    WORKSEQ ws;
    int i;

    for (i=0; i<redundancy; i++) {
    }
    return 0;
}
