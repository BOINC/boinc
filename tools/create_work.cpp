// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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

// Command-line program for creating jobs (workunits).
// Use directly for local job submission;
// run from PHP script for remote job submission.
//
// see https://github.com/BOINC/boinc/wiki/JobSubmission
//
// This program can be used in two ways:
// - to create a single job, with everything passed on the cmdline
// - to create multiple jobs, where per-job info is passed via stdin,
//      one line per job
//      available options here:
//      --command_line X
//      --wu_name X
//      --wu_template F
//      --result_template F
//      --remote_file url nbytes md5
//      --target_host ID
//      --target_user ID
//      --priority N
//      phys_name1 ...
//
// The input files must already be staged (i.e. in the download hierarchy).

#include "config.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <map>
#include <sys/param.h>
#include <unistd.h>

#include "boinc_db.h"
#include "common_defs.h"
#include "crypt.h"
#include "filesys.h"
#include "sched_config.h"
#include "str_replace.h"
#include "str_util.h"
#include "util.h"

#include "backend_lib.h"

// the max length of a job description line;
// also the max length of a job command line
//
#define CMD_SIZE    4096

using std::string;
using std::map;

bool verbose = false;
bool continue_on_error = false;

void usage() {
    fprintf(stderr,
        "usage: create_work [options] infile1 infile2 ...\n"
        "\n"
        "Options:\n"
        "   --appname name\n"
        "   [ --app_version_num N ]\n"
        "   [ --batch n ]\n"
        "   [ --broadcast ]\n"
        "   [ --broadcast_user ID ]\n"
        "   [ --broadcast_team ID ]\n"
        "   [ --command_line \"X\" ]\n"
        "   [ --config_dir path ]\n"
        "   [ --credit X ]\n"
        "   [ -d n ]\n"
        "   [ --delay_bound x ]\n"
        "   [ --hr_class n ]\n"
        "   [ --keywords 'n1 n2 ...' ]\n"
        "   [ --max_error_results n ]\n"
        "   [ --max_success_results n ]\n"
        "   [ --max_total_results n ]\n"
        "   [ --min_quorum n ]\n"
        "   [ --priority n ]\n"
        "   [ --result_template filename ]  default: appname_out\n"
        "   [ --rsc_disk_bound x ]\n"
        "   [ --rsc_fpops_est x ]\n"
        "   [ --rsc_fpops_bound x ]\n"
        "   [ --rsc_memory_bound x ]\n"
        "   [ --size_class n ]\n"
        "   [ --stdin ]\n"
        "   [ --sub_appname 'foo bar' ]\n"
        "   [ --target_host ID ]\n"
        "   [ --target_nresults n ]\n"
        "   [ --target_team ID ]\n"
        "   [ --target_user ID ]\n"
        "   [ --verbose ]\n"
        "   [ --wu_id ID ]   ID of existing workunit record (used by boinc_submit)\n"
        "   [ --wu_name name ]              default: generate a name based on app name\n"
        "   [ --wu_template filename ]      default: appname_in\n"
        "\nSee https://github.com/BOINC/boinc/wiki/JobSubmission\n"
    );
    exit(1);
}

bool arg(char** argv, int i, const char* name) {
    char buf[256];
    snprintf(buf, sizeof(buf), "-%s", name);
    if (!strcmp(argv[i], buf)) return true;
    snprintf(buf, sizeof(buf), "--%s", name);
    if (!strcmp(argv[i], buf)) return true;
    return false;
}

void check_assign_id(int x) {
    if (x == 0) {
        fprintf(stderr,
            "you must specify a nonzero database ID for assigning jobs to users, teams, or hosts.\n"
        );
        exit(1);
    }
}

// describes a job.
// Also used to store batch-level info such as template names
// and assignment info
//
struct JOB_DESC {
    DB_WORKUNIT wu;
    char wu_template[BLOB_SIZE];
    char wu_template_file[256];
    char result_template_file[256];
    char result_template_path[MAXPATHLEN];
    vector <INFILE_DESC> infiles;
    char command_line[CMD_SIZE];
    bool assign_flag;
    bool assign_multi;
    int assign_id;
    int assign_type;
    char sub_appname[256];

    JOB_DESC() {
        wu.clear();
        command_line[0] = 0;
        assign_flag = false;
        assign_multi = false;
        wu_template_file[0] = 0;
        result_template_file[0] = 0;
        assign_id = 0;
        assign_type = ASSIGN_NONE;
        sub_appname[0] = 0;

        // defaults (in case they're not in WU template)
        //
        wu.id = 0;
        wu.min_quorum = DEFAULT_MIN_QUORUM;
        wu.target_nresults = DEFAULT_TARGET_NRESULTS;
        wu.max_error_results = DEFAULT_MAX_ERROR_RESULTS;
        wu.max_total_results = DEFAULT_MAX_TOTAL_RESULTS;
        wu.max_success_results = DEFAULT_MAX_SUCCESS_RESULTS;
        wu.rsc_fpops_est = DEFAULT_RSC_FPOPS_EST;
        wu.rsc_fpops_bound =  DEFAULT_RSC_FPOPS_BOUND;
        wu.rsc_memory_bound = DEFAULT_RSC_MEMORY_BOUND;
        wu.rsc_disk_bound = DEFAULT_RSC_DISK_BOUND;
        wu.rsc_bandwidth_bound = 0.0;
            // Not used
        wu.delay_bound = DEFAULT_DELAY_BOUND;

    }
    void create();
    void parse_stdin_line(int, char**);
};

// parse additional job-specific info when using --stdin
//
void JOB_DESC::parse_stdin_line(int argc, char** argv) {
    for (int i=0; i<argc; i++) {
        if (arg(argv, i, (char*)"command_line")) {
            // concatenate per-job args to main args
            if (strlen(command_line)) {
                strcat(command_line, " ");
            }
            strcat(command_line, argv[++i]);
        } else if (arg(argv, i, (char*)"wu_name")) {
            safe_strcpy(wu.name, argv[++i]);
        } else if (arg(argv, i, (char*)"wu_template")) {
            safe_strcpy(wu_template_file, argv[++i]);
        } else if (arg(argv, i, (char*)"result_template")) {
            safe_strcpy(result_template_file, argv[++i]);
        } else if (arg(argv, i, (char*)"remote_file")) {
            INFILE_DESC id;
            id.is_remote = true;
            safe_strcpy(id.url, argv[++i]);
            id.nbytes = atof(argv[++i]);
            safe_strcpy(id.md5, argv[++i]);
            infiles.push_back(id);
        } else if (arg(argv, i, "target_host")) {
            assign_flag = true;
            assign_type = ASSIGN_HOST;
            assign_id = atoi(argv[++i]);
            check_assign_id(assign_id);
        } else if (arg(argv, i, "target_user")) {
            assign_flag = true;
            assign_type = ASSIGN_USER;
            assign_id = atoi(argv[++i]);
            check_assign_id(assign_id);
        } else if (arg(argv, i, (char*)"priority")) {
            wu.priority = atoi(argv[++i]);
        } else {
            if (!strncmp("-", argv[i], 1)) {
                fprintf(stderr, "create_work: bad stdin argument '%s'\n", argv[i]);
                exit(1);
            }
            INFILE_DESC id;
            id.is_remote = false;
            safe_strcpy(id.name, argv[i]);
            infiles.push_back(id);
        }
    }
}

// See if WU template was given for job.
// Many jobs may have the same ones.
// To avoid rereading files, cache them in a map.
// Get from cache if there, else read the file and add to cache
//
void get_wu_template(JOB_DESC& jd2) {
    // the jobs may specify WU templates.
    //
    static map<string, char*> wu_templates;

    string s = string(jd2.wu_template_file);
    if (wu_templates.count(s) == 0) {
        char* p;
        int retval = read_file_malloc(jd2.wu_template_file, p, 0, false);
        if (retval) {
            fprintf(
                stderr, "Can't read WU template %s\n", jd2.wu_template_file
            );
            exit(1);
        }
        wu_templates[s] = p;
    }
    strcpy(jd2.wu_template, wu_templates[s]);
}

// if a buffer is full after a fgets(), it was too small
//
void check_buffer(char *p, int len) {
    if (strlen(p) == len-1) {
        fprintf(stderr, "fgets() buffer was too small\n");
        exit(1);
    }
}

int main(int argc, char** argv) {
    DB_APP app;
    int retval;
    int i;
    char download_dir[256], db_name[256], db_passwd[256];
    char db_user[256],db_host[256];
    char buf[CMD_SIZE];
    JOB_DESC jd;
    bool show_wu_name = true;
    bool use_stdin = false;

    strcpy(app.name, "");
    strcpy(db_passwd, "");
    const char* config_dir = 0;
    i = 1;

    while (i < argc) {
        if (arg(argv, i, "appname")) {
            safe_strcpy(app.name, argv[++i]);
        } else if (arg(argv, i, "d")) {
            int dl = atoi(argv[++i]);
            log_messages.set_debug_level(dl);
            if (dl ==4) g_print_queries = true;
        } else if (arg(argv, i, "wu_name")) {
            show_wu_name = false;
            safe_strcpy(jd.wu.name, argv[++i]);
        } else if (arg(argv, i, "wu_template")) {
            safe_strcpy(jd.wu_template_file, argv[++i]);
        } else if (arg(argv, i, "result_template")) {
            safe_strcpy(jd.result_template_file, argv[++i]);
        } else if (arg(argv, i, "config_dir")) {
            config_dir = argv[++i];
        } else if (arg(argv, i, "batch")) {
            jd.wu.batch = atoi(argv[++i]);
        } else if (arg(argv, i, "priority")) {
            jd.wu.priority = atoi(argv[++i]);
        } else if (arg(argv, i, "credit")) {
            jd.wu.canonical_credit = atof(argv[++i]);
        } else if (arg(argv, i, "rsc_fpops_est")) {
            jd.wu.rsc_fpops_est = atof(argv[++i]);
        } else if (arg(argv, i, "rsc_fpops_bound")) {
            jd.wu.rsc_fpops_bound = atof(argv[++i]);
        } else if (arg(argv, i, "rsc_memory_bound")) {
            jd.wu.rsc_memory_bound = atof(argv[++i]);
        } else if (arg(argv, i, "size_class")) {
            jd.wu.size_class = atoi(argv[++i]);
        } else if (arg(argv, i, "app_version_num")) {
            jd.wu.app_version_num = atoi(argv[++i]);
        } else if (arg(argv, i, "rsc_disk_bound")) {
            jd.wu.rsc_disk_bound = atof(argv[++i]);
        } else if (arg(argv, i, "delay_bound")) {
            jd.wu.delay_bound = atoi(argv[++i]);
        } else if (arg(argv, i, "hr_class")) {
            jd.wu.hr_class = atoi(argv[++i]);
        } else if (arg(argv, i, "min_quorum")) {
            jd.wu.min_quorum = atoi(argv[++i]);
        } else if (arg(argv, i, "target_nresults")) {
            jd.wu.target_nresults = atoi(argv[++i]);
        } else if (arg(argv, i, "max_error_results")) {
            jd.wu.max_error_results = atoi(argv[++i]);
        } else if (arg(argv, i, "max_total_results")) {
            jd.wu.max_total_results = atoi(argv[++i]);
        } else if (arg(argv, i, "max_success_results")) {
            jd.wu.max_success_results = atoi(argv[++i]);
        } else if (arg(argv, i, "command_line")) {
            strcpy(jd.command_line, argv[++i]);
        } else if (arg(argv, i, "wu_id")) {
            jd.wu.id = atoi(argv[++i]);
        } else if (arg(argv, i, "broadcast")) {
            jd.assign_multi = true;
            jd.assign_flag = true;
            jd.assign_type = ASSIGN_NONE;
        } else if (arg(argv, i, "broadcast_user")) {
            jd.assign_flag = true;
            jd.assign_type = ASSIGN_USER;
            jd.assign_multi = true;
            jd.assign_id = atoi(argv[++i]);
            check_assign_id(jd.assign_id);
        } else if (arg(argv, i, "broadcast_team")) {
            jd.assign_flag = true;
            jd.assign_type = ASSIGN_TEAM;
            jd.assign_multi = true;
            jd.assign_id = atoi(argv[++i]);
            check_assign_id(jd.assign_id);
        } else if (arg(argv, i, "target_host")) {
            jd.assign_flag = true;
            jd.assign_type = ASSIGN_HOST;
            jd.assign_id = atoi(argv[++i]);
            check_assign_id(jd.assign_id);
        } else if (arg(argv, i, "target_user")) {
            jd.assign_flag = true;
            jd.assign_type = ASSIGN_USER;
            jd.assign_id = atoi(argv[++i]);
            check_assign_id(jd.assign_id);
        } else if (arg(argv, i, "target_team")) {
            jd.assign_flag = true;
            jd.assign_type = ASSIGN_TEAM;
            jd.assign_id = atoi(argv[++i]);
            check_assign_id(jd.assign_id);
        } else if (arg(argv, i, "help")) {
            usage();
            exit(0);
        } else if (arg(argv, i, "stdin")) {
            use_stdin = true;
        } else if (arg(argv, i, (char*)"remote_file")) {
            INFILE_DESC id;
            id.is_remote = true;
            safe_strcpy(id.url, argv[++i]);
            id.nbytes = atof(argv[++i]);
            safe_strcpy(id.md5, argv[++i]);
            jd.infiles.push_back(id);
        } else if (arg(argv, i, "verbose")) {
            verbose = true;
        } else if (arg(argv, i, "continue_on_error")) {
            continue_on_error = true;
        } else if (arg(argv, i, "keywords")) {
            strcpy(jd.wu.keywords, argv[++i]);
        } else if (arg(argv, i, "sub_appname")) {
            strcpy(jd.sub_appname, argv[++i]);
        } else {
            if (!strncmp("-", argv[i], 1)) {
                fprintf(stderr, "create_work: bad argument '%s'\n", argv[i]);
                exit(1);
            }
            INFILE_DESC id;
            id.is_remote = false;
            safe_strcpy(id.name, argv[i]);
            jd.infiles.push_back(id);
        }
        i++;
    }

    if (!strlen(app.name)) {
        usage();
    }
    if (!strlen(jd.wu.name)) {
        snprintf(jd.wu.name, sizeof(jd.wu.name), "%s_%d_%f", app.name, getpid(), dtime());
    }
    if (!strlen(jd.wu_template_file)) {
        snprintf(jd.wu_template_file, sizeof(jd.wu_template_file), "templates/%s_in", app.name);
    }
    if (!strlen(jd.result_template_file)) {
        snprintf(jd.result_template_file, sizeof(jd.result_template_file), "templates/%s_out", app.name);
    }

    retval = config.parse_file(config_dir);
    if (retval) {
        fprintf(stderr, "Can't parse config file: %s\n", boincerror(retval));
        exit(1);
    } else {
        strcpy(db_name, config.db_name);
        strcpy(db_passwd, config.db_passwd);
        strcpy(db_user, config.db_user);
        strcpy(db_host, config.db_host);
        strcpy(download_dir, config.download_dir);
    }

    retval = boinc_db.open(db_name, db_host, db_user, db_passwd);
    if (retval) {
        fprintf(stderr,
            "create_work: error opening database: %s\n", boincerror(retval)
        );
        exit(1);
    }
    boinc_db.set_isolation_level(READ_UNCOMMITTED);
    snprintf(buf, sizeof(buf), "where name='%s'", app.name);
    retval = app.lookup(buf);
    if (retval) {
        fprintf(stderr, "create_work: app not found\n");
        exit(1);
    }

    // read the WU template file.
    // this won't get used if we're creating a batch
    // with job-level WU templates
    //
    if (boinc_file_exists(jd.wu_template_file)) {
        retval = read_filename(
            jd.wu_template_file, jd.wu_template, sizeof(jd.wu_template)
        );
        if (retval) {
            fprintf(stderr,
                "create_work: can't open input template %s\n", jd.wu_template_file
            );
            exit(1);
        }
    } else {
        if (!use_stdin) {
            fprintf(stderr,
                "create_work: input template file %s doesn't exist\n",
                jd.wu_template_file
            );
            exit(1);
        }
    }

    jd.wu.appid = app.id;

    strcpy(jd.result_template_path, "./");
    strcat(jd.result_template_path, jd.result_template_file);

    if (use_stdin) {
        if (jd.assign_flag) {
            // if we're doing assignment we can't use the bulk-query method;
            // create the jobs one at a time.
            //
            int _argc;
            char* _argv[100];
            for (int j=0; ; j++) {
                char* p = fgets(buf, sizeof(buf), stdin);
                if (p == NULL) break;
                check_buffer(buf, sizeof(buf));
                JOB_DESC jd2 = jd;
                    // things default to what was passed on cmdline
                strcpy(jd2.wu.name, "");
                _argc = parse_command_line(buf, _argv);
                jd2.parse_stdin_line(_argc, _argv);
                    // get info from stdin line
                if (!strlen(jd2.wu.name)) {
                    snprintf(jd2.wu.name, sizeof(jd2.wu.name), "%s_%d", jd.wu.name, j);
                }
                if (strlen(jd2.wu_template_file)) {
                    get_wu_template(jd2);
                }
                if (!strlen(jd2.wu_template)) {
                    fprintf(stderr, "job is missing input template\n");
                    exit(1);
                }
                jd2.create();
            }
        } else {
            // stdin mode, unassigned.
            // for max efficiency, do them all in one big SQL query
            //
            string values;
            DB_WORKUNIT wu;
            int _argc;
            char* _argv[100], value_buf[MAX_QUERY_LEN];

            char additional_xml[256];
            additional_xml[0] = 0;
            if (strlen(jd.sub_appname)) {
                snprintf(additional_xml, sizeof(additional_xml),
                    "   <sub_appname>%s</sub_appname>",
                    jd.sub_appname
                );
            }

            for (int j=0; ; j++) {
                char* p = fgets(buf, sizeof(buf), stdin);
                if (p == NULL) break;
                check_buffer(buf, sizeof(buf));
                JOB_DESC jd2 = jd;
                strcpy(jd2.wu.name, "");
                _argc = parse_command_line(buf, _argv);
                jd2.parse_stdin_line(_argc, _argv);
                if (!strlen(jd2.wu.name)) {
                    snprintf(jd2.wu.name, sizeof(jd2.wu.name), "%s_%d", jd.wu.name, j);
                }
                // if the stdin line specified assignment,
                // create the job individually
                //
                if (jd2.assign_flag) {
                    jd2.create();
                    continue;
                }
                // otherwise accumulate a SQL query so that we can
                // create jobs en masse
                //
                if (strlen(jd2.wu_template_file)) {
                    get_wu_template(jd2);
                }
                if (!strlen(jd2.wu_template)) {
                    fprintf(stderr, "job is missing input template\n");
                    exit(1);
                }
                retval = create_work2(
                    jd2.wu,
                    jd2.wu_template,
                    jd2.result_template_file,
                    jd2.result_template_path,
                    jd2.infiles,
                    config,
                    jd2.command_line,
                    additional_xml,
                    value_buf
                );
                if (retval) {
                    fprintf(stderr, "create_work() failed: %d\n", retval);
                    if (continue_on_error) {
                        continue;
                    } else {
                        exit(1);
                    }
                }
                if (values.size()) {
                    values += ",";
                    values += value_buf;
                } else {
                    values = value_buf;
                }
                // MySQL can handles queries at least 1 MB
                //
                int n = strlen(value_buf);
                if (values.size() + 2*n > 1000000) {
                    retval = wu.insert_batch(values);
                    if (retval) {
                        fprintf(stderr,
                            "wu.insert_batch() failed: %d; size %d\n",
                            retval, (int)values.size()
                        );
                        fprintf(stderr,
                            "MySQL error: %s\n", boinc_db.error_string()
                        );
                        exit(1);
                    }
                    values.clear();
                }
            }
            if (values.size()) {
                retval = wu.insert_batch(values);
                if (retval) {
                    fprintf(stderr,
                        "wu.insert_batch() failed: %d\n", retval
                    );
                    fprintf(stderr,
                        "MySQL error: %s\n", boinc_db.error_string()
                    );
                    exit(1);
                }
            }
        }
    } else {
        jd.create();
        if (show_wu_name) {
            printf("workunit name: %s\n", jd.wu.name);
        }
    }
    boinc_db.close();
}

// create a single job
//
void JOB_DESC::create() {
    if (assign_flag) {
        wu.transitioner_flags = assign_multi?TRANSITION_NONE:TRANSITION_NO_NEW_RESULTS;
    }
    char additional_xml[256], kwbuf[256];
    additional_xml[0] = 0;
    if (strlen(sub_appname)) {
        snprintf(additional_xml, sizeof(additional_xml),
            "   <sub_appname>%s</sub_appname>",
            sub_appname
        );
    }
    int retval = create_work2(
        wu,
        wu_template,
        result_template_file,
        result_template_path,
        infiles,
        config,
        command_line,
        additional_xml
    );
    if (retval) {
        fprintf(stderr, "create_work: %s\n", boincerror(retval));
        exit(1);
    }
    if (verbose) {
        fprintf(stderr, "created workunit; name %s, ID %lu\n", wu.name, wu.id);
    }
    if (assign_flag) {
        DB_ASSIGNMENT assignment;
        assignment.clear();
        assignment.create_time = time(0);
        assignment.target_id = assign_id;
        assignment.target_type = assign_type;
        assignment.multi = assign_multi;
        assignment.workunitid = wu.id;
        retval = assignment.insert();
        if (retval) {
            fprintf(stderr,
                "assignment.insert() failed: %s\n", boincerror(retval)
            );
            exit(1);
        }
    }
}
