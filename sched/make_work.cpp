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

// make_work
//      --wu_name name
//      [ --wu_name name2 ... ]
//      [ --cushion n ]      // make work if fewer than N unsent results
//      [ --max_wus n ]      // don't make work if more than N total WUs
//      [ --one_pass ]       // quit after one pass
//
// Create WU and result records as needed to maintain a pool of work
// (for testing purposes).
// Clones the WU of the given name.
//

#include "config.h"
#include <sys/param.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <ctime>
#include <vector>
#include <string>

using std::vector;
using std::string;

#include "boinc_db.h"
#include "crypt.h"
#include "util.h"
#include "backend_lib.h"
#include "sched_config.h"
#include "parse.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "str_util.h"
#include "svn_version.h"

#define LOCKFILE            "make_work.out"
#define PIDFILE             "make_work.pid"

int max_wus = 0;
int cushion = 300;
bool one_pass = false;

// edit a WU XML doc, replacing one filename by another
// (should appear twice, within <file_info> and <file_ref>)
// Don't patch the URL; we'll download the same file
//
void replace_file_name(char* xml_doc, char* filename, char* new_filename) {
    char buf[BLOB_SIZE], temp[256];
    char * p;

    strcpy(buf, xml_doc);
    p = strtok(buf,"\n");
    while (p) {
        if (parse_str(p, "<name>", temp, sizeof(temp))) {
            if (!strcmp(filename, temp)) {
                replace_element_contents(
                    xml_doc + (p - buf),"<name>","</name>", new_filename
                );
            }
        } else if (parse_str(p, "<file_name>", temp, sizeof(temp))) {
            if (!strcmp(filename, temp)) {
                replace_element_contents(
                    xml_doc+(p-buf), "<file_name>","</file_name>", new_filename
                );
            }
        }
        p = strtok(0, "\n");
    }
}

void make_new_wu(DB_WORKUNIT& original_wu, char* starting_xml, int start_time) {
    char file_name[256], buf[BLOB_SIZE], new_file_name[256];
    char new_buf[BLOB_SIZE];
    char * p;
    int retval;
    DB_WORKUNIT wu = original_wu;
    static int file_seqno = 0, wu_seqno = 0;

    strcpy(buf, starting_xml);
    p = strtok(buf, "\n");
    strcpy(file_name, "");

    // make new names for the WU's input files,
    // so clients will download them.
    // (don't actually copy files; URL stays the same)
    //
    while (p) {
        if (parse_str(p, "<name>", file_name, sizeof(file_name))) {
            sprintf(
                new_file_name, "%s__%d_%d", file_name, start_time, file_seqno++
            );
            strcpy(new_buf, starting_xml);
            replace_file_name(new_buf, file_name, new_file_name);
            strcpy(wu.xml_doc, new_buf);
        }
        p = strtok(0, "\n");
    }

    // set various fields for new WU (all others are copied)
    //
    wu.id = 0;
    wu.create_time = time(0);

    // the name of the new WU cannot include the original WU name,
    // because the original one probably contains "nodelete",
    // but we want the copy to be eligible for file deletion
    //
    sprintf(wu.name, "wu_%d_%d", start_time, wu_seqno++);
    wu.need_validate = false;
    wu.canonical_resultid = 0;
    wu.canonical_credit = 0;
    wu.hr_class = 0;
    wu.transition_time = time(0);
    wu.error_mask = 0;
    wu.file_delete_state = FILE_DELETE_INIT;
    wu.assimilate_state = ASSIMILATE_INIT;
    retval = wu.insert();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Failed to created WU: %s; exiting\n", boincerror(retval)
        );
        exit(retval);
    }
    original_wu.id = boinc_db.insert_id();
    log_messages.printf(MSG_DEBUG,
        "Created %s, clone of %s\n", wu.name, original_wu.name
    );
}

// wait for the transitioner to create a result for the given WU.
// This keeps us from getting infinitely far ahead of the transitioner
// (e.g. if the transitioner isn't running)
//
void wait_for_results(int wu_id) {
    DB_RESULT result;
    int count, retval;
    char buf[256];

    sprintf(buf, "where workunitid=%d", wu_id);
    while (1) {
        retval = result.count(count, buf);
        log_messages.printf(MSG_DEBUG, "result.count for %d returned %d, error: %s\n",
            wu_id, count, boincerror(retval)
        );
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "result.count: %s\n", boincerror(retval));
            exit(1);
        }
        if (count > 0) return;
        daemon_sleep(10);
        check_stop_daemons();
    }
}

void make_work(vector<string> &wu_names) {
    int retval, start_time=time(0);
    char keypath[MAXPATHLEN];
    char buf[BLOB_SIZE];
    R_RSA_PRIVATE_KEY key;
    int nwu_names = wu_names.size();
    DB_WORKUNIT wus[nwu_names];
    int i;
    static int index=0;

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't parse config.xml: %s\n", boincerror(retval));
        exit(1);
    }

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    for (i=0; i<nwu_names; i++) {
        DB_WORKUNIT& wu = wus[i];
        sprintf(buf, "where name='%s'", wu_names[i].c_str());
        retval = wu.lookup(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "can't find wu %s\n", wu_names[i].c_str()
            );
            exit(1);
        }
    }

    sprintf(keypath, "%s/upload_private", config.key_dir);
    retval = read_key_file(keypath, key);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't read key\n");
        exit(1);
    }

    while (1) {
        check_stop_daemons();
        int unsent_results;

        retval = count_unsent_results(unsent_results, wus[0].appid);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "can't get result count\n"
            );
            exit(1);
        }
        int total_wus=0;
        if (max_wus) {
            retval = count_workunits(total_wus, "");
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "can't get wu count\n"
                );
                exit(1);
            }
        }
        log_messages.printf(
            MSG_DEBUG, "unsent: %d cushion: %d\n",
            unsent_results, cushion
        );
        if (unsent_results > cushion) {
            daemon_sleep(10);
            continue;
        }

        int results_needed = cushion - unsent_results;

        int new_wu_id = 0;
        while (1) {
            DB_WORKUNIT& wu = wus[index++];
            if (index == nwu_names) index=0;
            if (max_wus && total_wus >= max_wus) {
                log_messages.printf(MSG_NORMAL,
                    "Reached max_wus = %d\n", max_wus
                );
                exit(0);
                total_wus++;
            }
            make_new_wu(wu, wu.xml_doc, start_time);
            new_wu_id = wu.id;
            results_needed -= wu.target_nresults;
            if (results_needed <= 0) break;
        }

        if (one_pass) break;

        wait_for_results(new_wu_id);
    }
}

void usage(char *name) {
    fprintf(stderr,
        "Create WU and result records as needed to maintain a pool of work\n"
        "(for testing purposes).\n"
        "Clones the WU of the given name.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  --wu_name name                  the name for the WU\n"
        "                                  (can be repeated)\n"
        "  [ --cushion N ]                 make work if fewer than N unsent results\n"
        "  [ --max_wus n ]                 don't make work if more than N total WUs\n"
        "  [ --one_pass ]                  quit after one pass\n"
        "  [ --d X ]                       set debug level to X.\n"
        "  [ -h | --help ]                 shows this help text.\n"
        "  [ -v | --version ]              shows version information\n",
        name
    );
}

int main(int argc, char** argv) {
    int i;
    vector<string> wu_names;

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "cushion")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            cushion = atoi(argv[i]);
        } else if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (is_arg(argv[i], "wu_name")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            wu_names.push_back(string(argv[i]));
        } else if (is_arg(argv[i], "max_wus")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            max_wus = atoi(argv[i]);
        } else if (is_arg(argv[i], "one_pass")) {
            one_pass = true;
        } else if (is_arg(argv[1], "h") || is_arg(argv[1], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[1], "v") || is_arg(argv[1], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }
    check_stop_daemons();

    if (!wu_names.size()) {
        fprintf(stderr, "Must supply at least one WU name\n");
        exit(1);

    }

    log_messages.printf(MSG_NORMAL,
        "Starting: cushion %d, max_wus %d\n",
        cushion, max_wus
    );
    install_stop_signal_handler();

    srand48(getpid() + time(0));
    make_work(wu_names);
}

const char *BOINC_RCSID_d24265dc7f = "$Id$";
