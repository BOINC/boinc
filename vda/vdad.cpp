// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// vdad - volunteer data archival daemon
//
// Enumerates files needing updating from the DB.
// Creates the corresponding tree of META_CHUNKs, CHUNKs,
// and VDA_CHUNK_HOSTs.
// Calls the recovery routines to initiate transfers,
// update the DB, etc.

#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "boinc_db.h"
#include "sched_config.h"
#include "sched_util.h"

#include "error_numbers.h"
#include "util.h"
#include "filesys.h"

#include "vda_lib.h"

using std::vector;
using std::set;

void show_msg(char* msg) {
    printf("%s", msg);
}

int handle_file(VDA_FILE_AUX& vf, DB_VDA_FILE& dvf) {
    int retval;
    char buf[1024];

    log_messages.printf(MSG_NORMAL, "processing file %s\n", vf.file_name);

    // read the policy file
    //
    sprintf(buf, "%s/boinc_meta.txt", vf.dir);
    retval = vf.policy.parse(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't parse policy file %s\n", buf);
        return retval;
    }
    if (vf.initialized) {
        log_messages.printf(MSG_NORMAL, "Getting state\n");
        retval = vf.get_state();
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "vf.get_state failed %d\n", retval);
            return retval;
        }
    } else {
        log_messages.printf(MSG_NORMAL, "Initializing\n");
        retval = vf.init();
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "vf.init failed %d\n", retval);
            return retval;
        }
        sprintf(buf, "initialized=1, chunk_size=%.0f", vf.policy.chunk_size());
        dvf.update_field(buf);
    }
    log_messages.printf(MSG_NORMAL, "Recovery plan:\n");
    retval = vf.meta_chunk->recovery_plan();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "vf.recovery_plan failed %d\n", retval);
        return retval;
    }
    retval = vf.meta_chunk->decide_reconstruct();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "vf.decide_reconstruct failed %d\n", retval);
        return retval;
    }
    retval = vf.meta_chunk->reconstruct_and_cleanup();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "vf.reconstruct_and_cleanup failed %d\n", retval);
        return retval;
    }
    log_messages.printf(MSG_NORMAL, "Recovery action:\n");
    retval = vf.meta_chunk->recovery_action(dtime());
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "vf.recovery_action failed %d\n", retval);
        return retval;
    }
    return 0;
}

// handle files
//
bool scan_files() {
    DB_VDA_FILE vf;
    bool found = false;
    int retval;

    while (1) {
        retval = vf.enumerate("where need_update<>0");
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "VDA_FILE enumerate failed\n");
            exit(1);
        }
        VDA_FILE_AUX vfa(vf);
        found = true;
        retval = handle_file(vfa, vf);
        if (retval) {
            log_messages.printf(
                MSG_CRITICAL, "handle_file() failed: %d\n", retval
            );
            exit(1);
        } else {
            retval = vf.update_field("need_update=0");
            if (retval) {
                log_messages.printf(
                    MSG_CRITICAL, "update_field() failed: %d\n", retval
                );
                exit(1);
            }
        }
    }
    return found;
}

// this host is declared dead; deal with the loss of data
//
int handle_host(DB_HOST& h) {
    DB_VDA_CHUNK_HOST ch;
    char buf[256];
    int retval;

    log_messages.printf(MSG_NORMAL, "processing dead host %d\n", h.id);

    sprintf(buf, "where host_id=%d", h.id);
    while (1) {
        retval = ch.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) return retval;
        log_messages.printf(MSG_NORMAL, "   updating file%d\n", ch.vda_file_id);
        DB_VDA_FILE vf;
        retval = vf.lookup_id(ch.vda_file_id);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "   file lookup failed%d\n", ch.vda_file_id
            );
            return retval;
        }
        retval = vf.update_field("need_update=1");
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "   file update failed%d\n", ch.vda_file_id
            );
            return retval;
        }
    }
    return 0;
}

// handle timed-out hosts
//
bool scan_hosts() {
    DB_HOST h;
    char buf[256];
    int retval;
    bool found = false;

    sprintf(buf, "where cpu_efficiency < %f", dtime());
    while (1) {
        retval = h.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "host.enumerate() failed\n");
            exit(1);
        }
        found = true;
        retval = handle_host(h);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "handle_host() failed: %d\n", retval);
            exit(1);
        }
        retval = h.update_field("cpu_efficiency=1e12");
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "h.update_field() failed: %d\n", retval);
            exit(1);
        }

    }
    return found;
}

int main(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug_level")) {
            int dl = atoi(argv[++i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        }
    }
    int retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't parse config file\n");
        exit(1);
    }
    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open DB\n");
        exit(1);
    }

#if 0
    VDA_FILE_AUX vf;
    memset(&vf, 0, sizeof(vf));
    strcpy(vf.dir, "/mydisks/b/users/boincadm/vda_test");
    strcpy(vf.name, "file.ext");
    handle_file(vf);
    exit(0);
#endif
    while(1) {
        bool action = scan_files();
        action |= scan_hosts();
        if (!action) boinc_sleep(5.);
    }
}
