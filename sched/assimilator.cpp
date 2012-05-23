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

// This is a framework for an assimilator.
// You need to link this with an (application-specific) function
// assimilate_handler()
// in order to make a complete program.
//

#include "config.h"
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <vector>

#include "boinc_db.h"
#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "str_util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "assimilate_handler.h"

using std::vector;

#define LOCKFILE "assimilator.out"
#define PIDFILE  "assimilator.pid"
#define SLEEP_INTERVAL 10

bool update_db = true;
bool noinsert = false;
int wu_id_modulus=0, wu_id_remainder=0;
int sleep_interval = SLEEP_INTERVAL;
int one_pass_N_WU=0;
int g_argc;
char** g_argv;
char* results_prefix = NULL;
char* transcripts_prefix = NULL;

void usage(char** argv) {
    fprintf(stderr,
        "This program is an 'assimilator'; it handles completed jobs.\n"
        "Normally it is run as a daemon from config.xml.\n"
        "See: http://boinc.berkeley.edu/trac/wiki/BackendPrograms\n\n"
    );

    fprintf(stderr, "usage: %s [options]\n"
        "    Options:\n"
        "    --app name            Process jobs for the given application\n"
        "    [--sleep_interval X]  Sleep X seconds if no jobs to process (default 10)\n"
        "    [--mod N R]           Process jobs with mod(ID, N) == R\n"
        "    [--one_pass]          Do one DB enumeration, then exit\n"
        "    [--one_pass_N_WU N]   Process at most N jobs\n"
        "    [-d | --debug_level N]       Set verbosity level (1 to 4)\n"
        "    [--dont_update_db]    Don't update DB (for testing)\n"
        "    [--noinsert]          Don't insert records in app-specific DB\n"
        "    [-h | --help]                 Show this\n"
        "    [-v | --version]      Show version information\n",
        argv[0]
    );
    exit(0);
}

// assimilate all WUs that need it
// return nonzero (true) if did anything
//
bool do_pass(APP& app) {
    DB_WORKUNIT wu;
    DB_RESULT canonical_result, result;
    bool did_something = false;
    char buf[256];
    char mod_clause[256];
    int retval;
    int num_assimilated=0;

    if (wu_id_modulus) {
        sprintf(mod_clause, " and workunit.id %% %d = %d ",
                wu_id_modulus, wu_id_remainder
        );
    } else {
        strcpy(mod_clause, "");
    }

    sprintf(buf,
        "where appid=%d and assimilate_state=%d %s limit %d",
        app.id, ASSIMILATE_READY, mod_clause,
        one_pass_N_WU ? one_pass_N_WU : 1000
    );
    while (1) {
        retval = wu.enumerate(buf);
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_DEBUG,
                    "DB connection lost, exiting\n"
                );
                exit(0);
            }
            break;
        }
        vector<RESULT> results;     // must be inside while()!

        // for testing purposes, pretend we did nothing
        //
        if (update_db) {
            did_something = true;
        }

        log_messages.printf(MSG_DEBUG,
            "[%s] assimilating WU %d; state=%d\n", wu.name, wu.id, wu.assimilate_state
        );

        sprintf(buf, "where workunitid=%d", wu.id);
        canonical_result.clear();
        bool found = false;
        while (1) {
            retval = result.enumerate(buf);
            if (retval) {
                if (retval != ERR_DB_NOT_FOUND) {
                    log_messages.printf(MSG_DEBUG,
                        "DB connection lost, exiting\n"
                    );
                    exit(0);
                }
                break;
            }
            results.push_back(result);
            if (result.id == wu.canonical_resultid) {
                canonical_result = result;
                found = true;
            }
        }

        // If no canonical result found and WU had no other errors,
        // something is wrong, e.g. result records got deleted prematurely.
        // This is probably unrecoverable, so mark the WU as having
        // an assimilation error and keep going.
        //
        if (!found && !wu.error_mask) {
            log_messages.printf(MSG_CRITICAL,
                "[%s] no canonical result\n", wu.name
            );
            wu.error_mask = WU_ERROR_NO_CANONICAL_RESULT;
            sprintf(buf, "error_mask=%d", wu.error_mask);
            wu.update_field(buf);
        }

        retval = assimilate_handler(wu, results, canonical_result);
        if (retval && retval != DEFER_ASSIMILATION) {
            log_messages.printf(MSG_CRITICAL,
                "[%s] handler error: %s; exiting\n", wu.name, boincerror(retval)
            );
            exit(retval);
        }

        if (update_db) {
            // Defer assimilation until next result is returned
            int assimilate_state = ASSIMILATE_DONE;
            if (retval == DEFER_ASSIMILATION) {
                assimilate_state = ASSIMILATE_INIT;
            }
            sprintf(
                buf, "assimilate_state=%d, transition_time=%d",
                assimilate_state, (int)time(0)
            );
            retval = wu.update_field(buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[%s] update failed: %s\n", wu.name, boincerror(retval)
                );
                exit(1);
            }
        }

        num_assimilated++;

    }

    if (did_something) {
        boinc_db.commit_transaction();
    }

    if (num_assimilated)  {
        log_messages.printf(MSG_NORMAL,
            "Assimilated %d workunits.\n", num_assimilated
        );
    }

    return did_something;
}

int main(int argc, char** argv) {
    int retval;
    bool one_pass = false;
    DB_APP app;
    int i;
    char buf[256];

    strcpy(app.name, "");
    check_stop_daemons();
    g_argc = argc;
    g_argv = argv;
    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "one_pass_N_WU")) {
            one_pass_N_WU = atoi(argv[++i]);
            one_pass = true;
        } else if (is_arg(argv[i], "sleep_interval")) {
            sleep_interval = atoi(argv[++i]);
        } else if (is_arg(argv[i], "one_pass")) {
            one_pass = true;
        } else if (is_arg(argv[i], "d") || is_arg(argv[i], "debug_level")) {
            int dl = atoi(argv[++i]);
            log_messages.set_debug_level(dl);
            if (dl ==4) g_print_queries = true;
        } else if (is_arg(argv[i], "app")) {
            strcpy(app.name, argv[++i]);
        } else if (is_arg(argv[i], "dont_update_db")) {
            // This option is for testing your assimilator.  When set,
            // it ensures that the assimilator does not actually modify
            // the assimilate_state of the workunits, so you can run
            // your assimilator over and over again without affecting
            // your project.
            update_db = false;
        } else if (is_arg(argv[i], "noinsert")) {
            // This option is also for testing and is used to
            // prevent the inserting of results into the *backend*
            // (as opposed to the boinc) DB.
            noinsert = true;
        } else if (is_arg(argv[i], "mod")) {
            wu_id_modulus   = atoi(argv[++i]);
            wu_id_remainder = atoi(argv[++i]);
        } else if (is_arg(argv[i], "help") || is_arg(argv[i], "h")) {
            usage(argv);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
	} else if (is_arg(argv[i], "results_prefix")) {
	    results_prefix=argv[++i];
	} else if (is_arg(argv[i], "transcripts_prefix")) {
            transcripts_prefix=argv[++i];
        } else {
            log_messages.printf(MSG_CRITICAL, "Unrecognized arg: %s\n", argv[i]);
            usage(argv);
        }
    }

    if (!strlen(app.name)) {
        usage(argv);
    }

    if (wu_id_modulus) {
        log_messages.printf(MSG_DEBUG,
            "Using mod'ed WU enumeration.  modulus = %d  remainder = %d\n",
            wu_id_modulus, wu_id_remainder
        );
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    log_messages.printf(MSG_NORMAL, "Starting\n");

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't open DB\n");
        exit(1);
    }
    sprintf(buf, "where name='%s'", app.name);
    retval = app.lookup(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't find app\n");
        exit(1);
    }
    install_stop_signal_handler();
    do {
        if (!do_pass(app)) {
            if (!one_pass) {
                daemon_sleep(sleep_interval);
            }
        }
        check_stop_daemons();
    } while (!one_pass);
}


const char *BOINC_RCSID_7841370789 = "$Id$";
