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

//------------------------------------
//
// send_file [options]
// --host_id N           ID of host to upload from
// --file_name name      name of file
//
// Create a result entries, initialized to sent, and corresponding
// messages to the get the files.

#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string>
#include <time.h>

#include "boinc_db.h"
#include "util.h"
#include "str_util.h"
#include "md5_file.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"

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
    char result_xml[BLOB_SIZE];
    sprintf(result_xml,
        "<result>\n"
        "    <wu_name>%s</wu_name>\n"
        "    <name>%s</name>\n"
        "</result>\n",
        result.name, result.name
    );
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

int create_download_message(
    DB_RESULT& result, int host_id, const char* file_name
) {;
    DB_MSG_TO_HOST mth;
    int retval;
    double nbytes;
    char dirpath[256], urlpath[256], path[256], md5[33];
    strcpy(dirpath, config.download_dir);
    strcpy(urlpath, config.download_url);
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
        nbytes, result.name, FILE_MOVER, file_name
    );
    retval = mth.insert();
    if (retval) {
        fprintf(stderr, "msg_to_host.insert(): %d\n", retval);
        return retval;
    }
    return 0;
}

int send_file(int host_id, const char* file_name) {
    DB_RESULT result;
    int retval;
    result.clear();
    long int my_time = time(0);
    init_xfer_result(result);
    sprintf(result.name, "send_%s_%d_%ld", file_name, host_id, my_time);
    result.hostid = host_id;
    retval = create_download_result(result, host_id);
    retval = create_download_message(result, host_id, file_name);
    return retval;
}


void usage(char *name) {
    fprintf(stderr,
        "Create a result entries, initialized to sent, and corresponding\n"
        "messages to the get the files.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  -host_id id                    id of host to upload from\n"
        "  -file_name name                name of specific file, dominates workunit\n"
        "  [ -h | -help | --help ]        Show this help text.\n"
        "  [ -v | -version | --version ]  Show version information.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char file_name[256];
    int host_id;

    // initialize argument strings to empty
    strcpy(file_name, "");
    host_id = 0;

    check_stop_daemons();

    // get arguments
    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "host_id")) {
            if (!argv[++i]) {
                fprintf(stderr, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            host_id = atoi(argv[i]);
        } else if (is_arg(argv[i], "file_name")) {
            if (!argv[++i]) {
                fprintf(stderr, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            strcpy(file_name, argv[i]);
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            exit(0);
        } else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            fprintf(stderr, "unknowen command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    if (!strlen(file_name)) {
        fprintf(stderr,
            "send_file: bad command line, requires a valid host_id and file_name\n"
        );
        exit(1);
    }
    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        fprintf(stderr, "boinc_db.open failed: %d\n", retval);
        exit(1);
    }

    retval = send_file(host_id, file_name);

    boinc_db.close();
    return retval;
}

const char *BOINC_RCSID_f3c3c4b892 = "$Id$";
