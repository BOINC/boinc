// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

// feeder_user: a feeder that handles multiple job submitters fairly
//
// See usage() below.
// Implementation notes: https://github.com/BOINC/boinc/wiki/Multi-user-feeder

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <vector>
using std::vector;

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "shmem.h"
#include "str_util.h"
#include "synch.h"
#include "util.h"

#include "sched_config.h"
#include "sched_shmem.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define SLEEP_INTERVAL  5
    // failed to add new jobs, sleep for this
#define EMPTY_BACKOFF_TIME  15
    // if job stream had no new jobs, don't query for this interval
#define NJOBS_STARTUP   500
    // if an idle user submits a bunch of jobs,
    // at most this many will be picked before entering round robin

#define REREAD_DB_FILENAME      "reread_db"

// represents an enumeration of jobs from a given user
//
struct JOB_STREAM {
    DB_WORK_ITEM wi;
    int user_id;
    double inv_share;
    double usage;
        // # jobs added / share
    double pause_until;
    int num_left;

    JOB_STREAM(int u, double s) {
        user_id = u;
        inv_share = 1./s;
        usage = 0;
        pause_until = 0;
        num_left = 0;
    }
    bool scan_result_set(WU_RESULT&);
    bool get_job(WU_RESULT&);
    bool wi_is_usable();
};

vector<JOB_STREAM> job_streams;

SCHED_SHMEM* ssp;
key_t sema_key;
int sleep_interval = SLEEP_INTERVAL;
int num_work_items = MAX_WU_RESULTS;
int enum_limit = MAX_WU_RESULTS*2;
int purge_stale_time = 0;
double max_usage;

void cleanup_shmem() {
    ssp->ready = false;
    detach_shmem((void*)ssp);
    destroy_shmem(config.shmem_key);
}

int check_reread_trigger() {
    FILE* f;
    f = fopen(config.project_path(REREAD_DB_FILENAME), "r");
    if (f) {
        fclose(f);
        log_messages.printf(MSG_NORMAL,
            "Found trigger file %s; re-scanning database tables.\n",
            REREAD_DB_FILENAME
        );
        ssp->init(num_work_items);
        ssp->scan_tables();
        ssp->perf_info.get_from_db();
        int retval = unlink(config.project_path(REREAD_DB_FILENAME));
        if (retval) {
            // if we can't remove trigger file, exit to avoid infinite loop
            //
            log_messages.printf(MSG_CRITICAL,
                "Can't unlink trigger file; exiting\n"
            );
        }
        log_messages.printf(MSG_NORMAL,
            "Done re-scanning: trigger file removed.\n"
        );
    }
    return 0;
}

bool JOB_STREAM::wi_is_usable() {
    // Check for invalid application ID
    //
    if (!ssp->lookup_app(wi.wu.appid)) {
        log_messages.printf(MSG_CRITICAL,
            "result [RESULT#%lu] has bad appid %lu; quitting\n",
            wi.res_id, wi.wu.appid
        );
        exit(1);
    }

    // if the WU had an error, mark result as DIDNT_NEED
    //
    if (wi.wu.error_mask) {
        char buf[256];
        DB_RESULT result;
        result.id = wi.res_id;
        sprintf(buf, "server_state=%d, outcome=%d",
            RESULT_SERVER_STATE_OVER,
            RESULT_OUTCOME_DIDNT_NEED
        );
        result.update_field(buf);
        log_messages.printf(MSG_NORMAL,
            "[RESULT#%lu] WU had error, marking as DIDNT_NEED\n",
            wi.res_id
        );
        return false;
    }

    // Check for collision (i.e. this result already is in the array)
    //
    for (int j=0; j<ssp->max_wu_results; j++) {
        if (ssp->wu_results[j].state != WR_STATE_EMPTY && ssp->wu_results[j].resultid == wi.res_id) {
            log_messages.printf(MSG_DEBUG,
                "result [RESULT#%lu] already in array\n", wi.res_id
            );
            return false;
        }
    }
    return true;
}

JOB_STREAM* best_job_stream() {
    JOB_STREAM *best = NULL;
    double best_usage = 1e15;
    double now = dtime();
    for (JOB_STREAM &js: job_streams) {
        if (js.usage > best_usage) {
            continue;
        }
        if (js.pause_until > now) {
            continue;
        }
        best_usage = js.usage;
        best = &js;
    }
    return best;
}

// Scan through job streams, trying to fill the given slot.
// Return true if succeed
//
bool fill_slot(WU_RESULT &wr) {
    while (true) {
        JOB_STREAM *js = best_job_stream();
        if (!js) {
            log_messages.printf(MSG_DEBUG, "No active job streams\n");
            break;
        }
        log_messages.printf(MSG_DEBUG, "Best job stream: user %d\n",
            js->user_id
        );
        if (js->get_job(wr)) {
            return true;
        }
    }
    return false;
}

// scan through our result set, looking for a usable job.
// If find one, insert in slot and return true
//
bool JOB_STREAM::scan_result_set(WU_RESULT &wu_result) {
    while (num_left > 0) {
        int retval = wi.user_fetch_row();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "user_fetch_row() failed: %d", retval
            );
            exit(1);
        }
        num_left--;
        if (wi_is_usable()) {
            log_messages.printf(MSG_NORMAL,
                "adding result [RESULT#%lu]\n",
                wi.res_id
            );
            wu_result.resultid = wi.res_id;
            wu_result.res_priority = wi.res_priority;
            wu_result.res_server_state = wi.res_server_state;
            wu_result.res_report_deadline = wi.res_report_deadline;
            wu_result.workunit = wi.wu;
            wu_result.state = WR_STATE_PRESENT;
            wu_result.infeasible_count = 0;
            wu_result.time_added_to_shared_memory = time(0);
            usage += inv_share;
            if (usage > max_usage) {
                double d = usage - max_usage;
                for (JOB_STREAM &s: job_streams) {
                    if (s.usage > d) {
                        s.usage -= d;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

// try to fill the given slot using jobs from this stream.
// return true if succeed
//
bool JOB_STREAM::get_job(WU_RESULT& wr) {
    // first try our current result set
    if (scan_result_set(wr)) {
        return true;
    }
    // if that didn't work, get a new result set
    //
    int retval = wi.user_query(enum_limit, user_id);
    if (retval) {
        // If DB server dies, exit;
        // bin/start (run from crontab) will restart us eventually.
        //
        log_messages.printf(MSG_CRITICAL, "DB connection lost, exiting\n");
        exit(0);
    }
    num_left = wi.user_num_rows();
    if (scan_result_set(wr)) {
        return true;
    }
    // new result was either empty or entirely in shmem.
    // pause this stream
    //
    pause_until = time(0) + EMPTY_BACKOFF_TIME;
    log_messages.printf(MSG_DEBUG, "No jobs from %d; pausing\n", user_id);
    return false;
}

// Make one pass through the work array, filling in empty slots.
// Return true if we filled in any.
//
static bool scan_work_array() {
    bool action = false;
    int nadditions = 0;
    for (int i=0; i<ssp->max_wu_results; i++) {
        WU_RESULT& wu_result = ssp->wu_results[i];
        if (wu_result.state == WR_STATE_PRESENT) {
            if (purge_stale_time && wu_result.time_added_to_shared_memory < (time(0) - purge_stale_time)) {
                log_messages.printf(MSG_NORMAL,
                    "removing stale result [RESULT#%lu] from slot %d\n",
                    wu_result.resultid, i
                );
                wu_result.state = WR_STATE_EMPTY;
            } else {
                continue;
            }
        }
        if (wu_result.state == WR_STATE_EMPTY) {
            log_messages.printf(MSG_DEBUG, "trying to fill slot %d\n", i);
            if (fill_slot(wu_result)) {
                nadditions++;
                action = true;
            } else {
                break;
            }
        } else {
            // here the state is a PID; see if it's still alive
            //
            int pid = wu_result.state;
            struct stat s;
            char buf[256];
            sprintf(buf, "/proc/%d", pid);
            log_messages.printf(MSG_NORMAL, "checking pid %d\n", pid);
            if (stat(buf, &s)) {
                wu_result.state = WR_STATE_PRESENT;
                log_messages.printf(MSG_NORMAL,
                    "Result reserved by non-existent process PID %d; resetting\n",
                    pid
                );
            }
        }
    }
    log_messages.printf(MSG_DEBUG, "Added %d results to array\n", nadditions);
    return action;
}

void feeder_loop() {
    while (1) {
        if (!scan_work_array()) {
            log_messages.printf(MSG_DEBUG,
                "No action; sleeping %d sec\n", sleep_interval
            );
            daemon_sleep(sleep_interval);
        }
        fflush(stdout);
        check_stop_daemons();
        check_reread_trigger();
    }
}

void usage() {
    fprintf(stderr,
        "Usage: feeder_user options\n\n"
        "Options:\n"
        "  --user ID share              job submitter (can have multiple)\n"
        "  --purge_stale nsec           purge results not sent for this\n"
        "  -d X | --debug_level X       Set log verbosity to X (1..4)\n"
        "  -h | --help                  Shows this help text.\n"
        "  -v | --version               Shows version information.\n"
    );
}

void parse_cmdline(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (is_arg(argv[i], "d") || is_arg(argv[i], "debug_level")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage();
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) {
                g_print_queries = true;
            }
        } else if (is_arg(argv[i], "user")) {
            int user_id = atoi(argv[++i]);
            double share = atof(argv[++i]);
            JOB_STREAM js(user_id, share);
            job_streams.push_back(js);
        } else if (is_arg(argv[i], "purge_stale")) {
            purge_stale_time = atoi(argv[++i]);
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage();
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage();
            exit(1);
        }
    }
}

void show_init_state() {
    log_messages.printf(MSG_NORMAL, "feeder_user: starting\n");

    log_messages.printf(MSG_NORMAL,
        "read "
        "%d platforms, "
        "%d apps, "
        "%d app_versions, "
        "%d assignments\n",
        ssp->nplatforms,
        ssp->napps,
        ssp->napp_versions,
        ssp->nassignments
    );
    log_messages.printf(MSG_NORMAL,
        "Using %d job slots\n", ssp->max_wu_results
    );
    log_messages.printf(MSG_NORMAL, "Users:\n");
    for (JOB_STREAM js: job_streams) {
        log_messages.printf(MSG_NORMAL, "ID %d inv_share %f\n",
            js.user_id, js.inv_share
        );
    }
    log_messages.printf(MSG_NORMAL, "max_usage: %f\n", max_usage);
}

void feeder_init() {
    int retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    unlink(config.project_path(REREAD_DB_FILENAME));

    if (config.shmem_work_items) {
        num_work_items = config.shmem_work_items;
    }
    char path[1024];
    strlcpy(path, config.project_dir, sizeof(path));
    get_key(path, 'a', sema_key);
    destroy_semaphore(sema_key);
    create_semaphore(sema_key);

    retval = destroy_shmem(config.shmem_key);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't destroy shmem\n");
        exit(1);
    }

    int shmem_size = sizeof(SCHED_SHMEM) + num_work_items*sizeof(WU_RESULT);
    void *p;
    retval = create_shmem(config.shmem_key, shmem_size, 0 /* don't set GID */, &p);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't create shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    ssp->init(num_work_items);

    atexit(cleanup_shmem);

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        exit(1);
    }
    retval = boinc_db.set_isolation_level(READ_UNCOMMITTED);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.set_isolation_level: %d; %s\n", retval, boinc_db.error_string()
        );
    }
    ssp->scan_tables();

    retval = ssp->perf_info.get_from_db();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "PERF_INFO::get_from_db(): %d\n", retval
        );
    }

    ssp->ready = true;

    double x = 1e15;
    for (JOB_STREAM &s: job_streams) {
        if (s.inv_share < x) {
            x = s.inv_share;
        }
    }
    max_usage = NJOBS_STARTUP*x;

    show_init_state();
}

int main(int argc, char** argv) {
    parse_cmdline(argc, argv);
    if (job_streams.empty()) {
        log_messages.printf(MSG_CRITICAL, "No users specified\n");
        exit(1);
    }
    feeder_init();
    feeder_loop();
}
