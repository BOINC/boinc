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

//------------------------------------
//
// send_file [-host_id host_id] [-file_name file_name] [-num_copies]
// -host_id            name of host to upload from
// -file_name          name of specific file, dominates workunit
//
// Create a result entries, initialized to sent, and corresponding
// messages to the get the files.


#if HAVE_UNISTD_H
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <time.h>
#include "boinc_db.h"
#include "sched_config.h"
#include "sched_util.h"
#include "md5_file.h"
#include "util.h"

SCHED_CONFIG config;

void init_xfer_result(DB_RESULT& result) {
    result.id = 0;
    result.create_time = time(0);
    result.workunitid = 0;
    result.server_state = RESULT_SERVER_STATE_IN_PROGRESS;
    result.hostid = 0;
    result.report_deadline = 0;
    result.sent_time = 0;
    result.received_time = 0;
    result.client_state = 0;
    result.cpu_time = 0;
    strcpy(result.xml_doc_out, "");
    strcpy(result.stderr_out, "");
    result.outcome = RESULT_OUTCOME_INIT;
    result.file_delete_state = ASSIMILATE_DONE;
    result.validate_state = VALIDATE_STATE_NO_CHECK;
    result.claimed_credit = 0;
    result.granted_credit = 0;
    result.appid = 0;
}

int create_download_result(DB_RESULT& result, int host_id) {
    int retval;
    char result_xml[LARGE_BLOB_SIZE];
    sprintf(result_xml,
            "<result>\n"
            "    <wu_name>%s</wu_name>\n"
            "    <name>%s</name>\n"
            "</result>\n",
            result.name, result.name);
    strcpy(result.xml_doc_in, result_xml);
    result.sent_time = time(0);
    result.report_deadline = 0;
    result.hostid = host_id;
    retval = result.insert();
    if (retval) {
        fprintf(stderr, "result.insert(): %d\n", retval);
        return retval;
    }
    return 0;
}

int create_download_message(DB_RESULT& result, int host_id, const char* file_name, int priority, unsigned long exp_days) {;
    DB_MSG_TO_HOST mth;
    int retval;
    double nbytes;
    char dirpath[256], urlpath[256], path[256], md5[33];
    dirpath = config.download_dir;
    urlpath = config.download_url;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = host_id;
    strcpy(mth.variety, "file_xfer");
    mth.handled = false;
    sprintf(path, "%s/%s", dirpath, file_name);
    retval = md5_file(path, md5, nbytes);
    if (retval) {
        fprintf(stderr, "process_wu_template: md5_file %d\n", retval);
        return retval;
    }
    sprintf(mth.xml,
            "<app>\n"
            "    <name>%s</name>\n"
            "</app>\n"
            "<app_version>\n"
            "    <app_name>%s</app_name>\n"
            "    <version_num>%d00</version_num>\n"
            "</app_version>\n"
            "%s"
             "<file_info>\n"
            "    <name>%s</name>\n"
            "    <url>%s/%s</url>\n"
            "    <md5_cksum>%s</md5_cksum>\n"
            "    <nbytes>%.0f</nbytes>\n"
            "    <sticky/>\n"
            "    <priority>%d</priority>\n"
            "    <exp_days>%d</exp_days>\n"
            "</file_info>\n"
            "<workunit>\n"
            "    <name>%s</name>\n"
            "    <app_name>%s</app_name>\n"
            "    <file_ref>\n"
            "      <file_name>%s</file_name>\n"
            "    </file_ref>\n"
            "</workunit>",
            FILE_MOVER, FILE_MOVER, BOINC_MAJOR_VERSION, result.xml_doc_in,
            file_name, urlpath, file_name, md5,
            nbytes, priority, exp_days, result.name, FILE_MOVER, file_name);
    retval = mth.insert();
    if (retval) {
        fprintf(stderr, "msg_to_host.insert(): %d\n", retval);
        return retval;
    }
    return 0;
}

int send_file(int host_id, const char* file_name, int priority, unsigned long exp_date) {
    DB_RESULT result;
    int retval;
    result.clear();
    long int my_time = time(0);
    init_xfer_result(result);
    sprintf(result.name, "send_%s_%d_%ul", file_name, host_id, my_time);
    result.hostid = host_id;
    retval = create_download_result(result, host_id);
    retval = create_download_message(result, host_id, file_name, priority, exp_date);
    return retval;
}


int main(int argc, char** argv) {
    int i, retval;
    char file_name[256];
    int host_id;
    int priority = 1;
    unsigned long exp_days;
    int num_copies;

    // initialize argument strings to empty
    strcpy(file_name, "");
    host_id = 0;
    num_copies = 0;
    priority = 1;
    exp_days = 60;

    check_stop_daemons();

    // get arguments
    for(i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-host_id")) {
            host_id = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-file_name")) {
            strcpy(file_name, argv[++i]);
        } else if (!strcmp(argv[i], "-priority")) {
            priority = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-exp_days")) {
            exp_days = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-help")) {
            fprintf(stdout,
                    "send_file: sends a file to a specific host\n\n",
                    "It takes the following arguments and types:\n",
                    "-hostid (int); the number of the host\n",
                    "-file_name (string); the name of the file to send\n",
                    "-priority (int); the priority of the file, (low=1, high=5)\n",
                    "-exp_days (int); the number of days until the file should expire\n");
        } else {
            if (!strncmp("-",argv[i],1)) {
                fprintf(stderr, "send_file: bad argument '%s'\n", argv[i]);
                fprintf(stderr, "type send_file -help for more information\n");
                exit(1);
            }
        }
    }

    // if no arguments are given, error and exit
    if (!strlen(file_name)) {
        fprintf(stderr, "send_file: bad command line, requires a valid host_id and file_name\n");
        exit(1);
    }
    // parse the configuration file to get database information
    retval = config.parse_file("..");
    if (retval) {
        fprintf(stderr, "Can't parse config file: %d\n", retval);
        exit(1);
    }

    // open the database
    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        fprintf(stderr, "boinc_db.open failed: %d\n", retval);
        exit(1);
    }

    // run the get file routine
    retval = send_file(host_id, file_name, priority, exp_days);

    // close the database
    boinc_db.close();
    // return with error code if any
    return retval;
}
