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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>

#include "boinc_db.h"
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
    bool found=false;

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
            found = true;
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
    if (!found) {
        fprintf(stderr, "create_work: bad WU template - no <workunit>\n");
        return -1;
    }
    return 0;
}

// Set the time-varying fields of a result to their initial state.
// This is used to create clones of existing results,
// so set only the time-varying fields
//
void initialize_result(DB_RESULT& result, DB_WORKUNIT& wu) {
    result.id = 0;
    result.create_time = time(0);
    result.workunitid = wu.id;
    result.server_state = RESULT_SERVER_STATE_UNSENT;
    result.hostid = 0;
    result.report_deadline = 0;
    result.sent_time = 0;
    result.received_time = 0;
    result.client_state = 0;
    result.cpu_time = 0;
    strcpy(result.xml_doc_out, "");
    strcpy(result.stderr_out, "");
    result.outcome = RESULT_OUTCOME_INIT;
    result.file_delete_state = ASSIMILATE_INIT;
    result.validate_state = VALIDATE_STATE_INIT;
    result.claimed_credit = 0;
    result.granted_credit = 0;
}

// Create a new result for the given WU.
//
int create_result(
    DB_WORKUNIT& wu, char* result_template,
    char* result_name_suffix, R_RSA_PRIVATE_KEY& key,
    char* upload_url
) {
    DB_RESULT result;
    char base_outfile_name[256];
    char result_template_copy[MAX_BLOB_SIZE];
    int retval;

    result.clear();
    initialize_result(result, wu);
    sprintf(result.name, "%s_%s", wu.name, result_name_suffix);
    sprintf(base_outfile_name, "%s_", result.name);

    strcpy(result_template_copy, result_template);
    retval = process_result_template(
        result_template_copy,
        key,
        base_outfile_name,
        upload_url
    );
    strcpy(result.xml_doc_in, result_template_copy);

    // NOTE: result::insert() sets random

    retval = result.insert();
    if (retval) {
        fprintf(stderr, "result.insert(): %d\n", retval);
    }
    return retval;
}

int create_work(
    DB_WORKUNIT& wu,
    char* _wu_template,
    char* result_template_filename,
    char* infile_dir,
    char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY& key,
    char* upload_url, char* download_url
) {
    int retval;
    char _result_template[MAX_BLOB_SIZE];
    char wu_template[MAX_BLOB_SIZE];

    strcpy(wu_template, _wu_template);
    wu.create_time = time(0);
    retval = process_wu_template(
        wu.name, wu_template, wu.xml_doc, infile_dir, infiles, ninfiles,
        upload_url, download_url
    );
    if (retval) {
        fprintf(stderr, "process_wu_template: %d\n", retval);
        return retval;
    }

    retval = read_filename(result_template_filename, _result_template);
    if (retval) {
        fprintf(stderr, "create_work: can't read result template\n");
        return retval;
    }

    strcpy(wu.result_template, _result_template);
    process_result_template_upload_url_only(wu.result_template, upload_url);

    retval = wu.insert();
    if (retval) {
        fprintf(stderr, "create_work: workunit.insert() %d\n", retval);
        return retval;
    }
    wu.transition_time = time(0);
    wu.id = boinc_db_insert_id();

#if 0
    char suffix[256];
    char result_template[MAX_BLOB_SIZE];
    int i;
    for (i=0; i<wu.target_nresults; i++) {
        sprintf(suffix, "%d", i);
        strcpy(result_template, _result_template);
        retval = create_result(
            wu, result_template, suffix, key, upload_url
        );
        if (retval) {
            fprintf(stderr, "create_result: %d\n", retval);
            break;
        }
    }
#endif
    return 0;
}

int create_sequence(
    DB_WORKUNIT& wu,
    char* wu_template,
    char* result_template_filename,
    char* infile_dir,
    char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY& key,
    char* upload_url, char* download_url,
    int nsteps
) {
    int i, retval;
    DB_WORKSEQ ws;

    retval = ws.insert();
    if (retval) return retval;
    for (i=0; i<nsteps; i++) {
        // to be completed
    }
    return 0;
}

int create_sequence_group(
    DB_WORKUNIT& wu,
    char* wu_template,
    char* result_template_filename,
    char* infile_dir,
    char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY& key,
    char* upload_url, char* download_url,
    int nsteps
) {
    // WORKSEQ ws;
    // int i;

    // for (i=0; i<redundancy; i++) {
    // }
    return 0;
}
