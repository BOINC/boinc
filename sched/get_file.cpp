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

// get_file [options]
// --host_id N              ID of host to upload from
// --file_name name         name of specific file, dominates workunit
//
// Create a result entries, initialized to sent, and corresponding
// messages to the host that is assumed to have the file.
//
// Run from the project root dir.

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

#include "sched_config.h"
#include "sched_util.h"
#include "md5_file.h"
#include "svn_version.h"

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

int create_upload_result(
    DB_RESULT& result, int host_id, const char * file_name
) {
    int retval;
    char result_xml[BLOB_SIZE];
    sprintf(result_xml,
        "<result>\n"
        "    <wu_name>%s</wu_name>\n"
        "    <name>%s</wu_name>\n"
        "    <file_ref>\n"
        "      <file_name>%s</file_name>\n"
        "    </file_ref>\n"
        "</result>\n",
        result.name, result.name, file_name
    );
    strcpy(result.xml_doc_in, result_xml);
    result.sent_time = time(0);
    result.report_deadline = 0;
    result.hostid = host_id;
    retval = result.insert();
    if (retval) {
        fprintf(stderr, "result.insert(): %s\n", boincerror(retval));
        return retval;
    }
    return 0;
}

int create_upload_message(
    DB_RESULT& result, int host_id, const char* file_name
) {;
    DB_MSG_TO_HOST mth;
    int retval;
    mth.clear();
    mth.create_time = time(0);
    mth.hostid = host_id;
    strcpy(mth.variety, "file_xfer");
    mth.handled = false;
    sprintf(mth.xml,
        "<app>\n"
        "    <name>%s</name>\n"
        "</app>\n"
        "<app_version>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d00</version_num>\n"
        "</app_version>\n"
        "<file_info>\n"
        "    <name>%s</name>\n"
        "    <url>%s</url>\n"
        "    <max_nbytes>%.0f</max_nbytes>\n"
        "    <upload_when_present/>\n"
        "</file_info>\n"
        "%s"
        "<workunit>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n"
        "</workunit>",
        FILE_MOVER, FILE_MOVER, BOINC_MAJOR_VERSION,
        file_name, config.upload_url,
        1e10, result.xml_doc_in, result.name, FILE_MOVER
    );
    retval = mth.insert();
    if (retval) {
        fprintf(stderr, "msg_to_host.insert(): %s\n", boincerror(retval));
        return retval;
    }
    return 0;
}

int get_file(int host_id, const char* file_name) {
    DB_RESULT result;
    long int my_time = time(0);
    int retval;
    result.clear();
    init_xfer_result(result);
    sprintf(result.name, "get_%s_%d_%ld", file_name, host_id, my_time);
    result.hostid = host_id;
    retval = create_upload_result(result, host_id, file_name);
    retval = create_upload_message(result, host_id, file_name);
    return retval;
}

void usage(char *name) {
    fprintf(stderr, "Gets a file from a specific host.\n"
        "Creates a result entry, initialized to sent, and corresponding\n"
        "messages to the host that is assumed to have the file.\n"
        "Run from the project root dir.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  --host_id id                    "
        "Specify numerical id of host to upload from.\n"
        "  --file_name name                "
        "Specify name of file, dominates workunit.\n"
        "  [ -v | --version ]     Show version information.\n"
        "  [ -h | --help ]        Show this help text.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char file_name[256];
    int host_id;

    strcpy(file_name, "");
    host_id = 0;

    check_stop_daemons();

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
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            fprintf(stderr, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    if (!strlen(file_name) || host_id == 0) {
        fprintf(stderr,
            "get_file: bad command line, requires a valid host_id and file_name\n"
        );
        exit(1);
    }

    retval = config.parse_file();
    if (retval) {
        fprintf(stderr, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        fprintf(stderr, "boinc_db.open failed: %s\n", boincerror(retval));
        exit(1);
    }

    retval = get_file(host_id, file_name);
    boinc_db.close();
    return retval;
}

const char *BOINC_RCSID_37238a0141 = "$Id$";
