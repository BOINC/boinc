// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <ctime>
#include <cassert>
#include <unistd.h>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>


#include "boinc_db.h"
#include "crypt.h"
#include "error_numbers.h"
#include "md5_file.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "common_defs.h"
#include "filesys.h"
#include "sched_util.h"
#include "util.h"

#include "process_input_template.h"

#include "backend_lib.h"

using std::string;

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
#ifndef _USING_FCGI_
    FILE* f = fopen(path, "r");
#else
    FCGI_FILE *f=FCGI::fopen(path, "r");
#endif
    if (!f) return -1;
    retval = read_file(f, buf, len);
    fclose(f);
    return retval;
}

// initialize an about-to-be-created result, given its WU
//
static void initialize_result(DB_RESULT& result, WORKUNIT& wu) {
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
    result.appid = wu.appid;
    result.priority = wu.priority;
    result.batch = wu.batch;
}

int create_result_ti(
    TRANSITIONER_ITEM& ti,
    char* result_template_filename,
    char* result_name_suffix,
    R_RSA_PRIVATE_KEY& key,
    SCHED_CONFIG& config_loc,
    char* query_string,
        // if nonzero, write value list here; else do insert
    int priority_increase
) {
    WORKUNIT wu;

    // copy relevant fields from TRANSITIONER_ITEM to WORKUNIT
    //
    strcpy(wu.name, ti.name);
    wu.id = ti.id;
    wu.appid = ti.appid;
    wu.priority = ti.priority;
    wu.batch = ti.batch;
    return create_result(
        wu,
        result_template_filename,
        result_name_suffix,
        key,
        config_loc,
        query_string,
        priority_increase
    );
}

// Create a new result for the given WU.
// This is called from:
// - the transitioner
// - the scheduler (for assigned jobs)
//
int create_result(
    WORKUNIT& wu,
    char* result_template_filename,
    char* result_name_suffix,
    R_RSA_PRIVATE_KEY& key,
    SCHED_CONFIG& config_loc,
    char* query_string,
        // if nonzero, write value list here; else do insert
    int priority_increase
) {
    DB_RESULT result;
    char base_outfile_name[256];
    char result_template[BLOB_SIZE];
    int retval;

    result.clear();
    initialize_result(result, wu);
    result.priority += priority_increase;
    sprintf(result.name, "%s_%s", wu.name, result_name_suffix);
    sprintf(base_outfile_name, "%s_", result.name);
    retval = read_filename(
        result_template_filename, result_template, sizeof(result_template)
    );
    if (retval) {
        fprintf(stderr,
            "Failed to read result template file '%s': %s\n",
            result_template_filename, boincerror(retval)
        );
        return retval;
    }

    retval = process_result_template(
        result_template, key, base_outfile_name, config_loc
    );
    if (retval) {
        fprintf(stderr,
            "process_result_template() error: %s\n", boincerror(retval)
        );
    }
    if (strlen(result_template) > sizeof(result.xml_doc_in)-1) {
        fprintf(stderr,
            "result XML doc is too long: %d bytes, max is %d\n",
            (int)strlen(result_template), (int)sizeof(result.xml_doc_in)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    strlcpy(result.xml_doc_in, result_template, sizeof(result.xml_doc_in));

    result.random = lrand48();

    if (query_string) {
        result.db_print_values(query_string);
    } else {
        retval = result.insert();
        if (retval) {
            fprintf(stderr, "result.insert(): %s\n", boincerror(retval));
            return retval;
        }
    }

    return 0;
}

// make sure a WU's input files are actually there
//
int check_files(char** infiles, int ninfiles, SCHED_CONFIG& config_loc) {
    int i;
    char path[MAXPATHLEN];

    for (i=0; i<ninfiles; i++) {
        dir_hier_path(
            infiles[i], config_loc.download_dir, config_loc.uldl_dir_fanout, path
        );
        if (!boinc_file_exists(path)) {
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
    SCHED_CONFIG& config_loc,
    const char* command_line,
    const char* additional_xml
) {
    int retval;
    char _result_template[BLOB_SIZE];
    char wu_template[BLOB_SIZE];

#if 0
    retval = check_files(infiles, ninfiles, config_loc);
    if (retval) {
        fprintf(stderr, "Missing input file: %s\n", infiles[0]);
        return -1;
    }
#endif

    strcpy(wu_template, _wu_template);
    wu.create_time = time(0);
    retval = process_input_template(
        wu, wu_template, infiles, ninfiles, config_loc, command_line, additional_xml
    );
    if (retval) {
        fprintf(stderr, "process_input_template(): %s\n", boincerror(retval));
        return retval;
    }

    retval = read_filename(
        result_template_filepath, _result_template, sizeof(_result_template)
    );
    if (retval) {
        fprintf(stderr,
            "create_work: can't read result template file %s\n",
            result_template_filepath
        );
        return retval;
    }

    if (strlen(result_template_filename) > sizeof(wu.result_template_file)-1) {
        fprintf(stderr,
            "result template filename is too big: %d bytes, max is %d\n",
            (int)strlen(result_template_filename),
            (int)sizeof(wu.result_template_file)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    strlcpy(wu.result_template_file, result_template_filename, sizeof(wu.result_template_file));

    if (wu.rsc_fpops_est == 0) {
        fprintf(stderr, "no rsc_fpops_est given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.rsc_fpops_bound == 0) {
        fprintf(stderr, "no rsc_fpops_bound given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.rsc_disk_bound == 0) {
        fprintf(stderr, "no rsc_disk_bound given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.target_nresults == 0) {
        fprintf(stderr, "no target_nresults given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_error_results == 0) {
        fprintf(stderr, "no max_error_results given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_total_results == 0) {
        fprintf(stderr, "no max_total_results given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_success_results == 0) {
        fprintf(stderr, "no max_success_results given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_success_results > wu.max_total_results) {
        fprintf(stderr, "max_success_results > max_total_results; can't create job\n");
        return ERR_INVALID_PARAM;
    }
    if (wu.max_error_results > wu.max_total_results) {
        fprintf(stderr, "max_error_results > max_total_results; can't create job\n");
        return ERR_INVALID_PARAM;
    }
    if (wu.target_nresults > wu.max_success_results) {
        fprintf(stderr, "target_nresults > max_success_results; can't create job\n");
        return ERR_INVALID_PARAM;
    }
    if (wu.transitioner_flags & TRANSITION_NONE) {
        wu.transition_time = INT_MAX;
    } else {
        wu.transition_time = time(0);
    }
    if (wu.id) {
        retval = wu.update();
        if (retval) {
            fprintf(stderr,
                "create_work: workunit.update() %s\n", boincerror(retval)
            );
            return retval;
        }
    } else {
        retval = wu.insert();
        if (retval) {
            fprintf(stderr,
                "create_work: workunit.insert() %s\n", boincerror(retval)
            );
            return retval;
        }
        wu.id = boinc_db.insert_id();
    }

    return 0;
}

// STUFF RELATED TO FILE UPLOAD/DOWNLOAD

int get_file_xml(
    const char* file_name, vector<const char*> urls,
    double max_nbytes,
    double report_deadline,
    bool generate_upload_certificate,
    R_RSA_PRIVATE_KEY& key,
    char* out
) {
    char buf[8192];
    sprintf(out,
        "<app>\n"
        "    <name>file_xfer</name>\n"
        "</app>\n"
        "<app_version>\n"
        "    <app_name>file_xfer</app_name>\n"
        "    <version_num>0</version_num>\n"
        "</app_version>\n"
        "<file_info>\n"
        "    <name>%s</name>\n"
        "    <max_nbytes>%.0f</max_nbytes>\n",
        file_name,
        max_nbytes
    );
    for (unsigned int i=0; i<urls.size(); i++) {
        sprintf(buf, "    <url>%s</url>\n", urls[i]);
        strcat(out, buf);
    }
    sprintf(buf,
        "</file_info>\n"
        "<workunit>\n"
        "    <name>upload_%s</name>\n"
        "    <app_name>file_xfer</app_name>\n"
        "</workunit>\n"
        "<result>\n"
        "    <wu_name>upload_%s</wu_name>\n"
        "    <name>upload_%s</name>\n"
        "    <file_ref>\n"
        "        <file_name>%s</file_name>\n"
        "    </file_ref>\n"
        "    <report_deadline>%f</report_deadline>\n"
        "</result>\n",
        file_name,
        file_name,
        file_name,
        file_name,
        report_deadline
    );
    strcat(out, buf);
    if (generate_upload_certificate) {
        add_signatures(out, key);
    }
    return 0;
}

int create_get_file_msg(
    int host_id, const char* file_name, vector<const char*> urls,
    double max_nbytes,
    double report_deadline,
    bool generate_upload_certificate,
    R_RSA_PRIVATE_KEY& key
) {
    DB_MSG_TO_HOST mth;
    int retval;

    mth.clear();
    mth.create_time = time(0);
    mth.hostid = host_id;
    strcpy(mth.variety, "file_xfer");
    mth.handled = false;
    get_file_xml(
        file_name, urls, max_nbytes, report_deadline,
        generate_upload_certificate, key,
        mth.xml
    );
    retval = mth.insert();
    if (retval) {
        fprintf(stderr, "msg_to_host.insert(): %s\n", boincerror(retval));
        return retval;
    }
    return 0;
}

int put_file_xml(
    const char* file_name,
    vector<const char*> urls, const char* md5, double nbytes,
    double report_deadline,
    char* out
) {
    char buf[8192];
    sprintf(out,
        "<app>\n"
        "    <name>file_xfer</name>\n"
        "</app>\n"
        "<app_version>\n"
        "    <app_name>file_xfer</app_name>\n"
        "    <version_num>0</version_num>\n"
        "</app_version>\n"
         "<file_info>\n"
        "    <name>%s</name>\n",
        file_name
    );
    for (unsigned int i=0; i<urls.size(); i++) {
        sprintf(buf, "    <url>%s</url>\n", urls[i]);
        strcat(out, buf);
    }
    sprintf(buf,
        "    <md5_cksum>%s</md5_cksum>\n"
        "    <nbytes>%.0f</nbytes>\n"
        "    <sticky/>\n"
        "</file_info>\n"
        "<workunit>\n"
        "    <name>download_%s</name>\n"
        "    <app_name>file_xfer</app_name>\n"
        "    <file_ref>\n"
        "        <file_name>%s</file_name>\n"
        "    </file_ref>\n"
        "</workunit>\n"
        "<result>\n"
        "    <wu_name>download_%s</wu_name>\n"
        "    <name>download_%s</name>\n"
        "    <report_deadline>%f</report_deadline>\n"
        "</result>\n",
        md5,
        nbytes,
        file_name,
        file_name,
        file_name,
        file_name,
        report_deadline
    );
    strcat(out, buf);
    return 0;
}

int create_put_file_msg(
    int host_id, const char* file_name,
    vector<const char*> urls, const char* md5, double nbytes,
    double report_deadline
) {
    DB_MSG_TO_HOST mth;
    int retval;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = host_id;
    strcpy(mth.variety, "file_xfer");
    mth.handled = false;
    put_file_xml(file_name, urls, md5, nbytes, report_deadline, mth.xml);
    retval = mth.insert();
    if (retval) {
        fprintf(stderr, "msg_to_host.insert(): %s\n", boincerror(retval));
        return retval;
    }
    return 0;
}

int delete_file_xml(const char* file_name, char* out) {
    sprintf(out, "<delete_file_info>%s</delete_file_info>\n", file_name);
    return 0;
}

int create_delete_file_msg(int host_id, const char* file_name) {
    DB_MSG_TO_HOST mth;
    int retval;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = host_id;
    mth.handled = false;
    delete_file_xml(file_name, mth.xml);
    sprintf(mth.variety, "delete_file");
    retval = mth.insert();
    if (retval) {
        fprintf(stderr, "msg_to_host.insert(): %s\n", boincerror(retval));
        return retval;
    }
    return 0;
}

// cancel jobs in a range of workunit IDs
//
int cancel_jobs(int min_id, int max_id) {
    DB_WORKUNIT wu;
    DB_RESULT result;
    char set_clause[256], where_clause[256];
    int retval;

    sprintf(set_clause, "server_state=%d, outcome=%d",
        RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_DIDNT_NEED
    );
    sprintf(where_clause, "server_state=%d and workunitid >=%d and workunitid<= %d",
        RESULT_SERVER_STATE_UNSENT, min_id, max_id
    );
    retval = result.update_fields_noid(set_clause, where_clause);
    if (retval) return retval;

    sprintf(set_clause, "error_mask=error_mask|%d, transition_time=%d",
        WU_ERROR_CANCELLED, (int)(time(0))
    );
    sprintf(where_clause, "id>=%d and id<=%d", min_id, max_id);
    retval = wu.update_fields_noid(set_clause, where_clause);
    if (retval) return retval;
    return 0;
}

// cancel a particular job
//
int cancel_job(DB_WORKUNIT& wu) {
    DB_RESULT result;
    char set_clause[256], where_clause[256];
    int retval;

    // cancel unsent results
    //
    sprintf(set_clause, "server_state=%d, outcome=%d",
        RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_DIDNT_NEED
    );
    sprintf(where_clause, "server_state=%d and workunitid=%d",
        RESULT_SERVER_STATE_UNSENT, wu.id
    );
    retval = result.update_fields_noid(set_clause, where_clause);
    if (retval) return retval;

    // cancel the workunit
    //
    sprintf(set_clause, "error_mask=error_mask|%d, transition_time=%d",
        WU_ERROR_CANCELLED, (int)(time(0))
    );
    retval = wu.update_field(set_clause);
    if (retval) return retval;
    return 0;
}

// return the sum of user quotas
//
static int total_quota(double& total) {
    DB_USER_SUBMIT us;

    total = 0;
    while (1) {
        int retval = us.enumerate("");
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) {
            return retval;
        }
        total += us.quota;
    }
    return 0;
}

// return the project's total RAC.
// The easiest way to do this is to add up the RAC
// update user (job submitter) priority fields given the assumption
// that they did X FLOPS of computing
//
int adjust_user_priority(
    DB_USER_SUBMIT& us, double flop_count, double project_flops
) {
    int retval;
    double tq;
    char set_clause[256], where_clause[256];

    retval = total_quota(tq);
    if (retval) return retval;

    double runtime = flop_count / project_flops;
    double share = us.quota / tq;
    runtime /= share;
    us.logical_start_time += runtime;

    sprintf(set_clause, "logical_start_time=%f", us.logical_start_time);
    sprintf(where_clause, "user_id=%d", us.user_id);
    return us.update_fields_noid(set_clause, where_clause);
}

const char *BOINC_RCSID_b5f8b10eb5 = "$Id$";
