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

// make_work
//      -wu_name name
//      -result_template filename
//      [ -cushion n ]
//      [ -max_wus n ]
//
// Create WU and result records as needed to maintain a pool of work (for
// testing purposes).
// Clones the WU of the given name.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "boinc_db.h"
#include "crypt.h"
#include "util.h"
#include "backend_lib.h"
#include "sched_config.h"
#include "parse.h"
#include "sched_util.h"

#define LOCKFILE            "make_work.out"
#define PIDFILE             "make_work.pid"

int max_wus = 0;
int cushion = 30;
int min_quorum = 5;
int target_nresults = 10;
int max_error_results = 5;
int max_total_results = 20;
int max_success_results = 10;

char wu_name[256], result_template_file[256];

// edit a WU XML doc, replacing one filename by another
// (should appear twice, within <file_info> and <file_ref>)
// Also patch the download URL (redundant)
//
void replace_file_name(
    char* xml_doc, char* filename, char* new_filename, char* download_url
) {
    char buf[MAX_BLOB_SIZE], temp[256], download_path[256],
    new_download_path[256];
    char * p;

    sprintf(download_path,"%s/%s", download_url, filename);
    sprintf(new_download_path,"%s/%s", download_url, new_filename);
    strcpy(buf, xml_doc);
    p = strtok(buf,"\n");
    while (p) {
        if (parse_str(p, "<name>", temp, sizeof(temp))) {
            if(!strcmp(filename, temp)) {
                replace_element(xml_doc + (p - buf),"<name>","</name>",new_filename);
            }
        } else if (parse_str(p, "<file_name>", temp, sizeof(temp))) {
            if(!strcmp(filename, temp)) {
                replace_element(xml_doc + (p - buf),"<file_name>","</file_name>",new_filename);
            }
        } else if (parse_str(p, "<url>", temp, sizeof(temp))) {
            if(!strcmp(temp, download_path)) {
                replace_element(xml_doc + (p - buf),"<url>","</url>",new_download_path);
            }
        }
        p = strtok(0, "\n");
    }
}

char query_unsent_results[256];
inline const char* get_query_unsent_results()
{
    if (query_unsent_results[0] == 0) {
        sprintf(query_unsent_results, "where server_state=%d", RESULT_SERVER_STATE_UNSENT);
    }
    return query_unsent_results;
}

inline int count_results(const char* query="")
{
    int n;
    DB_RESULT result;
    int retval = result.count(n, const_cast<char*>(query));
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't count results\n");
        exit(1);
    }
    return n;
}

inline int count_workunits(const char* query="")
{
    int n;
    DB_WORKUNIT workunit;
    int retval = workunit.count(n, const_cast<char*>(query));
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't count workunits\n");
        exit(1);
    }
    return n;
}

void make_work() {
    CONFIG config;
    char * p;
    int retval, start_time=time(0);
    char keypath[256], result_template[MAX_BLOB_SIZE];
    char file_name[256], buf[MAX_BLOB_SIZE], pathname[256];
    char new_file_name[256], new_pathname[256], command[256];
    char starting_xml[MAX_BLOB_SIZE], new_buf[MAX_BLOB_SIZE];
    R_RSA_PRIVATE_KEY key;
    DB_WORKUNIT wu;
    int seqno = 0;

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't read config file\n");
        exit(1);
    }

    retval = boinc_db_open(config.db_name, config.db_passwd);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't open db\n");
        exit(1);
    }

    sprintf(buf, "where name='%s'", wu_name);
    retval = wu.lookup(buf);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't find wu %s\n", wu_name);
        exit(1);
    }

    strcpy(starting_xml,wu.xml_doc);

    sprintf(keypath, "%s/upload_private", config.key_dir);
    retval = read_key_file(keypath, key);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't read key\n");
        exit(1);
    }

    retval = read_filename(result_template_file, result_template);
    if (retval) {
        log_messages.printf(SchedMessages::CRITICAL, "can't open result template\n");
        exit(1);
    }
    while (1) {
        check_stop_trigger();

        int unsent_results = count_results(get_query_unsent_results());
        int total_wus = count_workunits();
        if (max_wus && total_wus >= max_wus) {
            log_messages.printf(SchedMessages::NORMAL, "Reached max_wus = %d\n", max_wus);
            exit(0);
        }
        if (unsent_results > cushion) {
            sleep(1);
            continue;
        }

        strcpy(buf, starting_xml);
        p = strtok(buf, "\n");
        strcpy(file_name, "");

        // make new copies of all the WU's input files
        //
        while (p) {
            if (parse_str(p, "<name>", file_name, sizeof(file_name))) {
                sprintf(
                    new_file_name, "%s__%d_%d", file_name, start_time, seqno++
                );
                sprintf(pathname, "%s/%s", config.download_dir, file_name);
                sprintf(
                    new_pathname, "%s/%s",config.download_dir, new_file_name
                );
                sprintf(command,"ln %s %s", pathname, new_pathname);
                log_messages.printf(SchedMessages::DEBUG, "executing command: %s\n", command);
                if (system(command)) {
                    log_messages.printf(SchedMessages::CRITICAL, "system() error\n");
                    perror(command);
                    exit(1);
                }
                strcpy(new_buf, starting_xml);
                replace_file_name(
                    new_buf, file_name, new_file_name, config.download_url
                );
                strcpy(wu.xml_doc, new_buf);
            }
            p = strtok(0, "\n");
        }
        sprintf(wu.name, "wu_%d_%d", start_time, seqno++);
        wu.id = 0;
        wu.create_time = time(0);
        wu.min_quorum = min_quorum;
        wu.target_nresults = target_nresults;
        wu.max_error_results = max_error_results;
        wu.max_total_results = max_total_results;
        wu.max_success_results = max_success_results;
        strcpy(wu.result_template, result_template);
        process_result_template_upload_url_only(wu.result_template, config.upload_url);
        retval = wu.insert();
        wu.id = boinc_db_insert_id();
        log_messages.printf(SchedMessages::DEBUG, "[%s] Created new WU\n", wu.name);
    }
}

int main(int argc, char** argv) {
    bool asynch = false;
    int i;

    check_stop_trigger();
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-cushion")) {
            cushion = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-result_template")) {
            strcpy(result_template_file, argv[++i]);
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-wu_name")) {
            strcpy(wu_name, argv[++i]);
        } else if (!strcmp(argv[i], "-max_wus")) {
            max_wus = atoi(argv[++i]);
        }
    }

    if (!strlen(result_template_file)) {
        log_messages.printf(SchedMessages::CRITICAL, "missing -result_template\n");
        exit(1);
    }
    if (!strlen(wu_name)) {
        log_messages.printf(SchedMessages::CRITICAL, "missing -wu_name\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // // Call lock_file after fork(), because file locks are not always inherited
    // if (lock_file(LOCKFILE)) {
    //     log_messages.printf(SchedMessages::NORMAL, "Another copy of make_work is already running\n");
    //     exit(1);
    // }
    // write_pid_file(PIDFILE);
    log_messages.printf(SchedMessages::NORMAL, "Starting\n");

    install_sigint_handler();

    srand48(getpid() + time(0));
    make_work();
}
