// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// make_work
//      -wu_name name
//      [ -cushion n ]      // make work if fewer than N unsent results
//      [ -max_wus n ]      // don't make work if more than N total WUs
//      [ -one_pass ]       // quit after one pass
//
// Create WU and result records as needed to maintain a pool of work
// (for testing purposes).
// Clones the WU of the given name.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <ctime>

#include "boinc_db.h"
#include "crypt.h"
#include "util.h"
#include "backend_lib.h"
#include "sched_config.h"
#include "parse.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define LOCKFILE            "make_work.out"
#define PIDFILE             "make_work.pid"

int max_wus = 0;
int cushion = 300;
bool one_pass = false;

char wu_name[256];

// edit a WU XML doc, replacing one filename by another
// (should appear twice, within <file_info> and <file_ref>)
// Also patch the download URL (redundant)
//
void replace_file_name(
    char* xml_doc, char* filename, char* new_filename, char* download_url
) {
    char buf[LARGE_BLOB_SIZE], temp[256], download_path[256],
    new_download_path[256];
    char * p;

    sprintf(download_path,"%s/%s", download_url, filename);
    sprintf(new_download_path,"%s/%s", download_url, new_filename);
    strcpy(buf, xml_doc);
    p = strtok(buf,"\n");
    while (p) {
        if (parse_str(p, "<name>", temp, sizeof(temp))) {
            if(!strcmp(filename, temp)) {
                replace_element_contents(
                    xml_doc + (p - buf),"<name>","</name>", new_filename
                );
            }
        } else if (parse_str(p, "<file_name>", temp, sizeof(temp))) {
            if(!strcmp(filename, temp)) {
                replace_element_contents(
                    xml_doc+(p-buf), "<file_name>","</file_name>", new_filename
                );
            }
        } else if (parse_str(p, "<url>", temp, sizeof(temp))) {
            if(!strcmp(temp, download_path)) {
                replace_element_contents(
                    xml_doc + (p - buf),"<url>","</url>", new_download_path
                );
            }
        }
        p = strtok(0, "\n");
    }
}

int count_results(char* query) {
    int n;
    DB_RESULT result;
    int retval = result.count(n, query);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't count results\n");
        exit(1);
    }
    return n;
}

int count_workunits(const char* query="") {
    int n;
    DB_WORKUNIT workunit;
    int retval = workunit.count(n, const_cast<char*>(query));
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't count workunits\n");
        exit(1);
    }
    return n;
}

void make_new_wu(
    DB_WORKUNIT& wu, char* starting_xml, int start_time, int& seqno,
    SCHED_CONFIG& config
) {
    char file_name[256], buf[LARGE_BLOB_SIZE], pathname[256];
    char new_file_name[256], new_pathname[256];
    char new_buf[LARGE_BLOB_SIZE];
    char * p;
    int retval;

    strcpy(buf, starting_xml);
    p = strtok(buf, "\n");
    strcpy(file_name, "");

    // make new hard links to the WU's input files
    // (don't actually copy files)
    //
    while (p) {
        if (parse_str(p, "<name>", file_name, sizeof(file_name))) {
            sprintf(
                new_file_name, "%s__%d_%d", file_name, start_time, seqno++
            );
            dir_hier_path(
                file_name, config.download_dir, config.uldl_dir_fanout, true,
                pathname
            );
            dir_hier_path(
                new_file_name, config.download_dir, config.uldl_dir_fanout, true,
                new_pathname, true
            );
            retval = link(pathname, new_pathname);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL, "link() error %d\n", retval
                );
                fprintf(stderr, "link: %d %d\n", errno, retval);
                perror("link");
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

    // set various fields for new WU (all others are copied)
    //
    wu.id = 0;
    wu.create_time = time(0);
    sprintf(wu.name, "wu_%d_%d", start_time, seqno++);
    wu.need_validate = false;
    wu.canonical_resultid = 0;
    wu.canonical_credit = 0;
    wu.transition_time = time(0);
    wu.error_mask = 0;
    wu.file_delete_state = FILE_DELETE_INIT;
    wu.assimilate_state = ASSIMILATE_INIT;
    retval = wu.insert();
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "Failed to created WU, error %d; exiting\n", retval
        );
        exit(retval);
    }
    wu.id = boinc_db.insert_id();
    log_messages.printf(SCHED_MSG_LOG::DEBUG, "[%s] Created new WU\n", wu.name);
}

void make_work() {
    SCHED_CONFIG config;
    int retval, start_time=time(0);
    char keypath[256];
    char buf[LARGE_BLOB_SIZE];
    char starting_xml[LARGE_BLOB_SIZE];
    R_RSA_PRIVATE_KEY key;
    DB_WORKUNIT wu;
    int seqno = 0;

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't read config file\n");
        exit(1);
    }

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't open db\n");
        exit(1);
    }

    sprintf(buf, "where name='%s'", wu_name);
    retval = wu.lookup(buf);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't find wu %s\n", wu_name);
        exit(1);
    }

    strcpy(starting_xml, wu.xml_doc);

    sprintf(keypath, "%s/upload_private", config.key_dir);
    retval = read_key_file(keypath, key);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't read key\n");
        exit(1);
    }

    while (1) {
        check_stop_daemons();

        sprintf(buf,
            "where appid=%d and server_state=%d",
            wu.appid, RESULT_SERVER_STATE_UNSENT
        );
        int unsent_results = count_results(buf);
        int total_wus = count_workunits();
        if (max_wus && total_wus >= max_wus) {
            log_messages.printf(SCHED_MSG_LOG::NORMAL, "Reached max_wus = %d\n", max_wus);
            exit(0);
        }
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG, "unsent: %d cushion: %d\n",
            unsent_results, cushion
        );
        if (unsent_results > cushion) {
            sleep(10);
            continue;
        }

        // decide how many WUs to create based on cushion
        // and (if present) max_wus
        //
        int nwus = (cushion-unsent_results)/wu.target_nresults+1;
        if (max_wus) {
            int mwlimit = max_wus - total_wus;
            if (nwus > mwlimit) nwus = mwlimit;
        }

        for (int i=0; i<nwus; i++) {
            make_new_wu(wu, starting_xml, start_time, seqno, config);
        }

        if (one_pass) break;
        // wait a while for the transitioner to make results
        //
        sleep(60);
    }
}

int main(int argc, char** argv) {
    bool asynch = false;
    int i;

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-cushion")) {
            cushion = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-wu_name")) {
            strcpy(wu_name, argv[++i]);
        } else if (!strcmp(argv[i], "-max_wus")) {
            max_wus = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL, "unknown argument: %s\n", argv[i]
            );
        }
    }
    check_stop_daemons();

#define CHKARG(x,m) do { if (!(x)) { fprintf(stderr, "make_work: bad command line: "m"\n"); exit(1); } } while (0)
#define CHKARG_STR(v,m) CHKARG(strlen(v),m)
    CHKARG_STR(wu_name              , "need -wu_name");
#undef CHKARG
#undef CHKARG_STR

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // // Call lock_file after fork(), because file locks are not always inherited
    // if (lock_file(LOCKFILE)) {
    //     log_messages.printf(SCHED_MSG_LOG::NORMAL, "Another copy of make_work is already running\n");
    //     exit(1);
    // }
    // write_pid_file(PIDFILE);
    log_messages.printf(
        SCHED_MSG_LOG::NORMAL,
        "Starting: cushion %d, max_wus %d\n",
        cushion, max_wus
    );
    install_stop_signal_handler();

    srand48(getpid() + time(0));
    make_work();
}

const char *BOINC_RCSID_d24265dc7f = "$Id$";
