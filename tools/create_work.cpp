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

// Create a workunit.
// Input files must be in the download dir.
// See the docs for a description of WU and result template files
// This program must be run in the project's root directory
//
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

#include "backend_lib.h"
#include "boinc_db.h"
#include "common_defs.h"
#include "crypt.h"
#include "sched_config.h"
#include "util.h"

void usage() {
    fprintf(stderr,
        "usage: create_work [options] infile1 infile2 ...\n"
        "\n"
        "Options:\n"
        "   --appname name\n"
        "   [ --wu_name name ]              default: generate a name based on app name\n"
        "   [ --wu_template filename ]      default: appname_in\n"
        "   [ --result_template filename ]  default: appname_out\n"
        "   [ --config_dir path ]\n"
        "   [ --command_line \"X\" ]\n"
        "   [ --batch n ]\n"
        "   [ --rsc_fpops_est n ]\n"
        "   [ --rsc_fpops_bound n ]\n"
        "   [ --rsc_memory_bound n ]\n"
        "   [ --rsc_disk_bound n ]\n"
        "   [ --delay_bound x ]\n"
        "   [ --min_quorum x ]\n"
        "   [ --target_nresults x ]\n"
        "   [ --max_error_results x ]\n"
        "   [ --max_total_results x ]\n"
        "   [ --max_success_results x ]\n"
        "   [ --additional_xml x ]\n"
        "   [ --broadcast ]\n"
        "   [ --broadcast_user ID ]\n"
        "   [ --broadcast_team ID ]\n"
        "   [ --target_host ID ]\n"
        "   [ --target_user ID ]\n"
        "   [ --target_team ID ]\n"
        "   [ --wu_id N ]   ID of existing workunit record (used by boinc_submit)\n"
    );
    exit(1);
}

bool arg(const char** argv, int i, const char* name) {
    char buf[256];
    sprintf(buf, "-%s", name);
    if (!strcmp(argv[i], buf)) return true;
    sprintf(buf, "--%s", name);
    if (!strcmp(argv[i], buf)) return true;
    return false;
}

int main(int argc, const char** argv) {
    DB_APP app;
    DB_WORKUNIT wu;
    int retval;
    char wu_template[BLOB_SIZE];
    char wu_template_file[256], result_template_file[256], result_template_path[1024];
    const char* command_line=NULL;
    const char** infiles = NULL;
    int i, ninfiles;
    char download_dir[256], db_name[256], db_passwd[256];
    char db_user[256],db_host[256];
    char buf[256];
    char additional_xml[256];
    bool show_wu_name = true;
    bool assign_flag = false;
    bool assign_multi = false;
    int assign_id = 0;
    int assign_type = ASSIGN_NONE;

    strcpy(wu_template_file, "");
    strcpy(result_template_file, "");
    strcpy(app.name, "");
    strcpy(db_passwd, "");
    strcpy(additional_xml, "");
    const char* config_dir = 0;
    i = 1;
    ninfiles = 0;
    wu.clear();

    // defaults (in case they're not in WU template)

    wu.id = 0;
    wu.min_quorum = 2;
    wu.target_nresults = 2;
    wu.max_error_results = 3;
    wu.max_total_results = 10;
    wu.max_success_results = 6;
    wu.rsc_fpops_est = 3600e9;
    wu.rsc_fpops_bound =  86400e9;
    wu.rsc_memory_bound = 5e8;
    wu.rsc_disk_bound = 1e9;
    wu.rsc_bandwidth_bound = 0.0;
    wu.delay_bound = 7*86400;

    while (i < argc) {
        if (arg(argv, i, "appname")) {
            strcpy(app.name, argv[++i]);
        } else if (arg(argv, i, "d")) {
            int dl = atoi(argv[++i]);
            log_messages.set_debug_level(dl);
            if (dl ==4) g_print_queries = true;
        } else if (arg(argv, i, "wu_name")) {
            show_wu_name = false;
            strcpy(wu.name, argv[++i]);
        } else if (arg(argv, i, "wu_template")) {
            strcpy(wu_template_file, argv[++i]);
        } else if (arg(argv, i, "result_template")) {
            strcpy(result_template_file, argv[++i]);
        } else if (arg(argv, i, "batch")) {
            wu.batch = atoi(argv[++i]);
        } else if (arg(argv, i, "config_dir")) {
            config_dir = argv[++i];
        } else if (arg(argv, i, "batch")) {
            wu.batch = atoi(argv[++i]);
        } else if (arg(argv, i, "priority")) {
            wu.priority = atoi(argv[++i]);
        } else if (arg(argv, i, "rsc_fpops_est")) {
            wu.rsc_fpops_est = atof(argv[++i]);
        } else if (arg(argv, i, "rsc_fpops_bound")) {
            wu.rsc_fpops_bound = atof(argv[++i]);
        } else if (arg(argv, i, "rsc_memory_bound")) {
            wu.rsc_memory_bound = atof(argv[++i]);
        } else if (arg(argv, i, "rsc_disk_bound")) {
            wu.rsc_disk_bound = atof(argv[++i]);
        } else if (arg(argv, i, "delay_bound")) {
            wu.delay_bound = atoi(argv[++i]);
        } else if (arg(argv, i, "min_quorum")) {
            wu.min_quorum = atoi(argv[++i]);
        } else if (arg(argv, i, "target_nresults")) {
            wu.target_nresults = atoi(argv[++i]);
        } else if (arg(argv, i, "max_error_results")) {
            wu.max_error_results = atoi(argv[++i]);
        } else if (arg(argv, i, "max_total_results")) {
            wu.max_total_results = atoi(argv[++i]);
        } else if (arg(argv, i, "max_success_results")) {
            wu.max_success_results = atoi(argv[++i]);
        } else if (arg(argv, i, "opaque")) {
            wu.opaque = atoi(argv[++i]);
        } else if (arg(argv, i, "command_line")) {
            command_line= argv[++i];
        } else if (arg(argv, i, "additional_xml")) {
            strcpy(additional_xml, argv[++i]);
        } else if (arg(argv, i, "wu_id")) {
            wu.id = atoi(argv[++i]);
        } else if (arg(argv, i, "broadcast")) {
            assign_multi = true;
            assign_flag = true;
            assign_type = ASSIGN_NONE;
        } else if (arg(argv, i, "broadcast_user")) {
            assign_flag = true;
            assign_type = ASSIGN_USER;
            assign_multi = true;
            assign_id = atoi(argv[++i]);
        } else if (arg(argv, i, "broadcast_team")) {
            assign_flag = true;
            assign_type = ASSIGN_TEAM;
            assign_multi = true;
            assign_id = atoi(argv[++i]);
        } else if (arg(argv, i, "target_host")) {
            assign_flag = true;
            assign_type = ASSIGN_HOST;
            assign_id = atoi(argv[++i]);
        } else if (arg(argv, i, "target_user")) {
            assign_flag = true;
            assign_type = ASSIGN_USER;
            assign_id = atoi(argv[++i]);
        } else if (arg(argv, i, "target_team")) {
            assign_flag = true;
            assign_type = ASSIGN_TEAM;
            assign_id = atoi(argv[++i]);
        } else if (arg(argv, i, "help")) {
            usage();
            exit(0);
        } else {
            if (!strncmp("-", argv[i], 1)) {
                fprintf(stderr, "create_work: bad argument '%s'\n", argv[i]);
                exit(1);
            }
            infiles = argv+i;
            ninfiles = argc - i;
            break;
        }
        i++;
    }

    if (!strlen(app.name)) {
        usage();
    }
    if (!strlen(wu.name)) {
        sprintf(wu.name, "%s_%d_%f", app.name, getpid(), dtime());
    }
    if (!strlen(wu_template_file)) {
        sprintf(wu_template_file, "templates/%s_in", app.name);
    }
    if (!strlen(result_template_file)) {
        sprintf(result_template_file, "templates/%s_out", app.name);
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
    sprintf(buf, "where name='%s'", app.name);
    retval = app.lookup(buf);
    if (retval) {
        fprintf(stderr, "create_work: app not found\n");
        exit(1);
    }

    retval = read_filename(wu_template_file, wu_template, sizeof(wu_template));
    if (retval) {
        fprintf(stderr,
            "create_work: can't open input template %s\n", wu_template_file
        );
        exit(1);
    }

    wu.appid = app.id;

    strcpy(result_template_path, "./");
    strcat(result_template_path, result_template_file);
    retval = create_work(
        wu,
        wu_template,
        result_template_file,
        result_template_path,
        const_cast<const char **>(infiles),
        ninfiles,
        config,
        command_line,
        additional_xml
    );
    if (retval) {
        fprintf(stderr, "create_work: %s\n", boincerror(retval));
        exit(1);
    } else {
        if (show_wu_name) {
            printf("workunit name: %s\n", wu.name);
        }
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
        sprintf(buf, "transitioner_flags=%d",
            assign_multi?TRANSITION_NONE:TRANSITION_NO_NEW_RESULTS
        );
        retval = wu.update_field(buf);
        if (retval) {
            fprintf(stderr, "wu.update() failed: %s\n", boincerror(retval));
            exit(1);
        }
    }
    boinc_db.close();
}

const char *BOINC_RCSID_3865dbbf46 = "$Id$";
