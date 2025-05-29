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

// functions called from create_work
// and backend programs (scheduler, transitioner etc.)
// to create result records and other utilities

#include "config.h"
#include "boinc_stdio.h"
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
#include "common_defs.h"
#include "crypt.h"
#include "error_numbers.h"
#include "filesys.h"
#include "md5_file.h"
#include "parse.h"
#include "sched_util.h"
#include "str_replace.h"
#include "str_util.h"
#include "util.h"

#include "process_input_template.h"

#include "backend_lib.h"

using std::string;

// Initialize RNG based on time and PID
// (the random part of output filenames needs to be hard to guess)
//
static struct random_init {
    random_init() {
        srand48(getpid() + time(0));
    }
} random_init;

// read file into buffer
//
int read_file(FILE* f, char* buf, int len) {
    int n = boinc::fread(buf, 1, len, f);
    buf[n] = 0;
    return 0;
}

int read_filename(const char* path, char* buf, int len) {
    int retval;
    FILE* f = boinc::fopen(path, "r");
    if (!f) return -1;
    retval = read_file(f, buf, len);
    boinc::fclose(f);
    return retval;
}

// initialize an about-to-be-created result, given its WU
//
static void initialize_result(DB_RESULT& result, WORKUNIT& wu) {
    result.id = 0;
    result.create_time = time(0);
    result.workunitid = wu.id;
    result.size_class = wu.size_class;
    if (result.size_class < 0) {
        result.server_state = RESULT_SERVER_STATE_UNSENT;
    } else {
        result.server_state = RESULT_SERVER_STATE_INACTIVE;
    }
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
    safe_strcpy(wu.name, ti.name);
    wu.id = ti.id;
    wu.appid = ti.appid;
    wu.priority = ti.priority;
    wu.batch = ti.batch;
    wu.size_class = ti.size_class;
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
    char base_outfile_name[MAXPATHLEN];
    char result_template[BLOB_SIZE];
    int retval;

    initialize_result(result, wu);
    result.random = lrand48();

    result.priority += priority_increase;
    snprintf(result.name, sizeof(result.name), "%s_%s", wu.name, result_name_suffix);
    snprintf(base_outfile_name, sizeof(base_outfile_name), "%s_r%ld_", result.name, lrand48());
    retval = read_filename(
        result_template_filename, result_template, sizeof(result_template)
    );
    if (retval) {
        boinc::fprintf(stderr,
            "Failed to read result template file '%s': %s\n",
            result_template_filename, boincerror(retval)
        );
        return retval;
    }

    retval = process_result_template(
        result_template, key, base_outfile_name, config_loc
    );
    if (retval) {
        boinc::fprintf(stderr,
            "process_result_template() error: %s\n", boincerror(retval)
        );
    }
    if (strlen(result_template) > sizeof(result.xml_doc_in)-1) {
        boinc::fprintf(stderr,
            "result XML doc is too long: %d bytes, max is %d\n",
            (int)strlen(result_template), (int)sizeof(result.xml_doc_in)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    strlcpy(result.xml_doc_in, result_template, sizeof(result.xml_doc_in));

    if (query_string) {
        result.db_print_values(query_string);
    } else {
        retval = result.insert();
        if (retval) {
            boinc::fprintf(stderr, "result.insert(): %s\n", boincerror(retval));
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

// variant where input files are described by a list of names,
// for use by work generators
//
int create_work(
    DB_WORKUNIT& wu,
    const char* _wu_template,
    const char* result_template_filename,
    const char* result_template_filepath,
    const char** infiles,
    int ninfiles,
    SCHED_CONFIG& config_loc,
    const char* command_line,
    const char* additional_xml,
    char* query_string
) {
    vector<INFILE_DESC> infile_specs(ninfiles);
    for (int i=0; i<ninfiles; i++) {
        infile_specs[i].is_remote = false;
        safe_strcpy(infile_specs[i].name, infiles[i]);
    }
    return create_work2(
        wu,
        _wu_template,
        result_template_filename,
        result_template_filepath,
        infile_specs,
        config_loc,
        command_line,
        additional_xml,
        query_string
    );
}

// variant where input files are described by INFILE_DESCS,
// so you can have remote files etc.
//
// If query_string is present, don't actually create the job;
// instead, append to the query string.
// The caller is responsible for doing the query.
//
int create_work2(
    DB_WORKUNIT& wu,
    const char* _wu_template,
    const char* result_template_filename,
        // relative to project root; stored in DB
    const char* /* result_template_filepath*/,
        // deprecated
    vector<INFILE_DESC> &infiles,
    SCHED_CONFIG& config_loc,
    const char* command_line,
    const char* additional_xml,
    char* query_string
) {
    int retval;
    char wu_template[BLOB_SIZE];

#if 0
    retval = check_files(infiles, ninfiles, config_loc);
    if (retval) {
        boinc::fprintf(stderr, "Missing input file: %s\n", infiles[0]);
        return -1;
    }
#endif

    safe_strcpy(wu_template, _wu_template);
    wu.create_time = time(0);
    retval = process_input_template(
        wu, wu_template, infiles, config_loc, command_line, additional_xml
    );
    if (retval) {
        boinc::fprintf(stderr, "process_input_template(): %s\n", boincerror(retval));
        return retval;
    }

    // check for presence of result template.
    // we don't need to actually look at it.
    //
    const char* p = config_loc.project_path(result_template_filename);
    if (!boinc_file_exists(p)) {
        boinc::fprintf(stderr,
            "create_work: result template file %s doesn't exist\n", p
        );
        return retval;
    }

    if (strlen(result_template_filename) > sizeof(wu.result_template_file)-1) {
        boinc::fprintf(stderr,
            "result template filename is too big: %d bytes, max is %d\n",
            (int)strlen(result_template_filename),
            (int)sizeof(wu.result_template_file)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    strlcpy(wu.result_template_file, result_template_filename, sizeof(wu.result_template_file));

    if (wu.rsc_fpops_est == 0) {
        boinc::fprintf(stderr, "no rsc_fpops_est given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.rsc_fpops_bound == 0) {
        boinc::fprintf(stderr, "no rsc_fpops_bound given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.rsc_disk_bound == 0) {
        boinc::fprintf(stderr, "no rsc_disk_bound given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.target_nresults == 0) {
        boinc::fprintf(stderr, "no target_nresults given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_error_results == 0) {
        boinc::fprintf(stderr, "no max_error_results given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_total_results == 0) {
        boinc::fprintf(stderr, "no max_total_results given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_success_results == 0) {
        boinc::fprintf(stderr, "no max_success_results given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_success_results > wu.max_total_results) {
        boinc::fprintf(stderr, "max_success_results > max_total_results; can't create job\n");
        return ERR_INVALID_PARAM;
    }
    if (wu.max_error_results > wu.max_total_results) {
        boinc::fprintf(stderr, "max_error_results > max_total_results; can't create job\n");
        return ERR_INVALID_PARAM;
    }
    if (wu.target_nresults > wu.max_success_results) {
        boinc::fprintf(stderr,
            "target_nresults %d > max_success_results %d; can't create job\n",
            wu.target_nresults, wu.max_success_results
        );
        return ERR_INVALID_PARAM;
    }
    if (wu.transitioner_flags) {
        wu.transition_time = INT_MAX;
    } else {
        wu.transition_time = time(0);
    }
    if (query_string) {
        wu.db_print_values(query_string);
    } else if (wu.id) {
        retval = wu.update();
        if (retval) {
            boinc::fprintf(stderr,
                "create_work: workunit.update() %s\n", boincerror(retval)
            );
            return retval;
        }
    } else {
        retval = wu.insert();
        if (retval) {
            boinc::fprintf(stderr,
                "create_work: workunit.insert() %s\n", boincerror(retval)
            );
            return retval;
        }
        wu.id = wu.db->insert_id();
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
        boinc::fprintf(stderr, "msg_to_host.insert(): %s\n", boincerror(retval));
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
        boinc::fprintf(stderr, "msg_to_host.insert(): %s\n", boincerror(retval));
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
        boinc::fprintf(stderr, "msg_to_host.insert(): %s\n", boincerror(retval));
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
    sprintf(where_clause, "server_state<=%d and workunitid >=%d and workunitid<= %d",
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
    sprintf(where_clause, "server_state<=%d and workunitid=%lu",
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
int get_total_quota(double& total) {
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

// return total project FLOPS (based on recent credit)
//
int get_project_flops(double& total) {
    DB_APP_VERSION av;
    char buf[256];

    // compute credit per day
    //
    sprintf(buf, "where expavg_time > %f", dtime() - 30*86400);
    total = 0;
    while (1) {
        int retval = av.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) {
            return retval;
        }
        total += av.expavg_credit;
    }
    total /= COBBLESTONE_SCALE;     // convert to FLOPs per day
    total /= 86400;                 // convert to FLOPs per second
    return 0;
}

// compute delta to user.logical_start_time given the assumption
// that user did flop_count FLOPS of computing
//
double user_priority_delta(
    DB_USER_SUBMIT& us,
    double flop_count,
        // this should be wu.rsc_fpops_est * app.min_avg_pfc
        // to account for systematic errors in rsc_fpops_est
    double total_quota,
    double project_flops
) {
    if (total_quota == 0) return 0;
    if (project_flops == 0) return 0;

    double runtime = flop_count / project_flops;
    double share = us.quota / total_quota;
#if 0
    printf("  project flops %f\n", project_flops);
    printf("  quota %f, total %f, share %f\n", us.quota, total_quota, share);
    printf("  runtime %f\n", runtime);
    printf("  delta %f\n", runtime/share);
#endif
    return runtime/share;
}
