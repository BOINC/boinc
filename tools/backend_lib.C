static volatile const char *BOINCrcsid="$Id$";
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

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cassert>
#include <unistd.h>
#include <cmath>

#include "boinc_db.h"
#include "crypt.h"
#include "error_numbers.h"
#include "md5_file.h"
#include "parse.h"
#include "util.h"
#include "filesys.h"

#include "backend_lib.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

static struct random_init {
    random_init() {
    srand48(getpid() + time(0));
                        }
} random_init;

int read_file(FILE* f, char* buf, int len) {
    int n = fread(buf, 1, len, f);
    buf[n] = 0;
    return 0;
}

int read_filename(const char* path, char* buf, int len) {
    int retval;
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    retval = read_file(f, buf, len);
    fclose(f);
    return retval;
}


// process WU template
//
static int process_wu_template(
    WORKUNIT& wu,
    char* tmplate,
    const char** infiles,
    int n,
    SCHED_CONFIG& config
) {
    char* p;
    char buf[LARGE_BLOB_SIZE], md5[33], path[256], url[256], top_download_path[256];
    char out[LARGE_BLOB_SIZE];
    int retval, file_number;
    double nbytes;
    char open_name[256];
    bool found=false;

    strcpy(out, "");
    for (p=strtok(tmplate, "\n"); p; p=strtok(0, "\n")) {
        if (match_tag(p, "<file_info>")) {
            file_number = -1;
            strcat(out, "<file_info>\n");
            while (1) {
                p = strtok(0, "\n");
                if (!p) break;
                if (parse_int(p, "<number>", file_number)) {
                    continue;
                } else if (match_tag(p, "</file_info>")) {
                    if (file_number < 0) {
                        fprintf(stderr, "No file number found\n");
                        return ERR_XML_PARSE;
                    }
                    dir_hier_path(
                        infiles[file_number], config.download_dir,
                        config.uldl_dir_fanout, path, true
                    );
                    if (!boinc_file_exists(path)) {
                        sprintf(top_download_path,
                            "%s/%s",config.download_dir,
                            infiles[file_number]
                        );
                        boinc_copy(top_download_path,path);
                    }

                    retval = md5_file(path, md5, nbytes);
                    if (retval) {
                        fprintf(stderr, "process_wu_template: md5_file %d\n", retval);
                        return retval;
                    }
                    dir_hier_url(
                        infiles[file_number], config.download_url,
                        config.uldl_dir_fanout, url
                    );
                    sprintf(buf,
                        "    <name>%s</name>\n"
                        "    <url>%s</url>\n"
                        "    <md5_cksum>%s</md5_cksum>\n"
                        "    <nbytes>%.0f</nbytes>\n"
                        "</file_info>\n",
                        infiles[file_number],
                        url,
                        md5,
                        nbytes
                    );
                    strcat(out, buf);
                    break;
                } else {
                    strcat(out, p);
                    strcat(out, "\n");
                }
            }
        } else if (match_tag(p, "<workunit>")) {
            found = true;
            strcat(out, "<workunit>\n");
        } else if (match_tag(p, "</workunit>")) {
            strcat(out, "</workunit>\n");
        } else if (match_tag(p, "<file_ref>")) {
            file_number = -1;
            while (1) {
                p = strtok(0, "\n");
                if (!p) break;
                if (parse_int(p, "<file_number>", file_number)) {
                    continue;
                } else if (parse_str(p, "<open_name>", open_name, sizeof(open_name))) {
                    continue;
                } else if (match_tag(p, "</file_ref>")) {
                    if (file_number < 0) {
                        fprintf(stderr, "No file number found\n");
                        return ERR_XML_PARSE;
                    }
                    sprintf(buf,
                        "<file_ref>\n"
                        "    <file_name>%s</file_name>\n"
                        "    <open_name>%s</open_name>\n"
                        "</file_ref>\n",
                        infiles[file_number],
                        open_name
                    );
                    strcat(out, buf);
                    break;
                }
            }
        } else if (parse_double(p, "<rsc_fpops_est>", wu.rsc_fpops_est)) {
            continue;
        } else if (parse_double(p, "<rsc_fpops_bound>", wu.rsc_fpops_bound)) {
            continue;
        } else if (parse_double(p, "<rsc_memory_bound>", wu.rsc_memory_bound)) {
            continue;
        } else if (parse_double(p, "<rsc_disk_bound>", wu.rsc_disk_bound)) {
            continue;
        } else if (parse_int(p, "<batch>", wu.batch)) {
            continue;
        } else if (parse_int(p, "<delay_bound>", wu.delay_bound)) {
            continue;
        } else if (parse_int(p, "<min_quorum>", wu.min_quorum)) {
            continue;
        } else if (parse_int(p, "<target_nresults>", wu.target_nresults)) {
            continue;
        } else if (parse_int(p, "<max_error_results>", wu.max_error_results)) {
            continue;
        } else if (parse_int(p, "<max_total_results>", wu.max_total_results)) {
            continue;
        } else if (parse_int(p, "<max_success_results>", wu.max_success_results)) {
            continue;
        } else {
            strcat(out, p);
            strcat(out, "\n");
        }
    }
    if (!found) {
        fprintf(stderr, "create_work: bad WU template - no <workunit>\n");
        return -1;
    }
    if (strlen(out) > sizeof(wu.xml_doc)-1) {
        fprintf(stderr,
            "create_work: WU XML field is too long (%d bytes; max is %d)\n",
            strlen(out), sizeof(wu.xml_doc)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    safe_strncpy(wu.xml_doc, out, sizeof(wu.xml_doc));
    return 0;
}

// Set the time-varying fields of a result to their initial state.
// This is used to create clones of existing results,
// so set only the time-varying fields
//
void initialize_result(DB_RESULT& result, int workunit_id, int workunit_appid) {
    result.id = 0;
    result.create_time = time(0);
    result.workunitid = workunit_id;
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
    result.appid = workunit_appid;
}

// Create a new result for the given WU.
// This is called ONLY from the transitioner
//
int create_result(
    int workunit_id,
    int workunit_appid,
    char* wu_name,
    char* result_template_filename,
    char* result_name_suffix,
    R_RSA_PRIVATE_KEY& key,
    SCHED_CONFIG& config,
    char* query_string
        // if nonzero, write value list here; else do insert
) {
    DB_RESULT result;
    char base_outfile_name[256];
    char result_template[LARGE_BLOB_SIZE];
    int retval;

    result.clear();
    initialize_result(result, workunit_id, workunit_appid);
    sprintf(result.name, "%s_%s", wu_name, result_name_suffix);
    sprintf(base_outfile_name, "%s_", result.name);

    retval = read_filename(result_template_filename, result_template, sizeof(result_template));
    if (retval) {
        fprintf(stderr,
            "Failed to read result template file '%s': %d\n",
            result_template_filename, retval
        );
        return retval;
    }

    retval = process_result_template(
        result_template,
        key,
        base_outfile_name,
        config
    );
    if (strlen(result_template) > sizeof(result.xml_doc_in)-1) {
        fprintf(stderr,
            "result XML doc is too long: %d bytes, max is %d\n",
            strlen(result_template), sizeof(result.xml_doc_in)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    safe_strncpy(result.xml_doc_in, result_template, sizeof(result.xml_doc_in));

    result.random = lrand48();

    if (query_string) {
        result.db_print_values(query_string);
    } else {
        retval = result.insert();
        if (retval) {
            fprintf(stderr, "result.insert(): %d\n", retval);
            return retval;
        }
    }
    return 0;
}

// make sure a WU's input files are actually there
//
int check_files(char** infiles, int ninfiles, SCHED_CONFIG& config) {
    int i;
    char path[256];
    FILE* f;

    for (i=0; i<ninfiles; i++) {
        dir_hier_path(
            infiles[i], config.download_dir, config.uldl_dir_fanout, path
        );
        f = fopen(path, "r");
        if (f) {
            fclose(f);
        } else {
            return 1;
        }
    }
    return 0;
}

int create_work(
    DB_WORKUNIT& wu,
    const char* _wu_template,
    const char* result_template_filename,
    const char* result_template_filepath,
    const char** infiles,
    int ninfiles,
    R_RSA_PRIVATE_KEY& key,
    SCHED_CONFIG& config
) {
    int retval;
    char _result_template[LARGE_BLOB_SIZE];
    char wu_template[LARGE_BLOB_SIZE];

#if 0
    retval = check_files(infiles, ninfiles, config);
    if (retval) {
        fprintf(stderr, "Missing input file: %s\n", infiles[0]);
        return -1;
    }
#endif

    strcpy(wu_template, _wu_template);
    wu.create_time = time(0);
    retval = process_wu_template(
        wu, wu_template, infiles, ninfiles, config
    );
    if (retval) {
        fprintf(stderr, "process_wu_template: %d\n", retval);
        return retval;
    }

    retval = read_filename(
        result_template_filepath, _result_template, sizeof(_result_template)
    );
    if (retval) {
        fprintf(stderr, "create_work: can't read result template file %s\n", result_template_filepath);
        return retval;
    }

    if (strlen(result_template_filename) > sizeof(wu.result_template_file)-1) {
        fprintf(stderr, "result template filename is too big: %d bytes, max is %d\n",
            strlen(result_template_filename), sizeof(wu.result_template_file)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    safe_strncpy(wu.result_template_file, result_template_filename, sizeof(wu.result_template_file));

    wu.transition_time = time(0);
    retval = wu.insert();
    if (retval) {
        fprintf(stderr, "create_work: workunit.insert() %d\n", retval);
        return retval;
    }
    wu.id = boinc_db.insert_id();

    return 0;
}
