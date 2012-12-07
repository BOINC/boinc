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

// Feeder: create a shared memory segment containing DB info,
// including an array of work items (results/workunits to send).
//
// Usage: feeder [ options ]
//  [ -d x ]                debug level x
//  [ --allapps ]           interleave results from all applications uniformly
//  [ --by_batch ]          interleave results from all batches uniformly
//  [ --random_order ]      order by "random" field of result
//  [ --priority_order ]    order by decreasing "priority" field of result
//  [ --priority_asc ]      order by increasing "priority" field of result
//  [ --priority_order_create_time ]
//                          order by priority, then by increasing WU create time
//  [ --mod n i ]           handle only results with (id mod n) == i
//  [ --wmod n i ]          handle only workunits with (id mod n) == i
//                          recommended if using HR with multiple schedulers
//  [ --sleep_interval x ]  sleep x seconds if nothing to do
//  [ --appids a1{,a2} ]    get work only for appids a1,...
//                          (comma-separated list)
//  [ --purge_stale x ]     remove work items from the shared memory segment
//                          that have been there for longer then x minutes
//                          but haven't been assigned
//
// The feeder tries to keep the work array filled.
// It maintains a DB enumerator (DB_WORK_ITEM).
// scan_work_array() scans the work array.
// looking for empty slots and trying to fill them in.
// The enumeration may return results already in the array.
// So, for each result, we scan the entire array to make sure
// it's not there already (can this be streamlined?)
//
// The length of the enum (max and actual) and the number of empty
// slots may differ; either one may be larger.
// New jobs may arrive (from the transitioner at any time).
// So we use the following policies:
//
// - Restart the enum at most once during a given array scan
// - If a scan doesn't add anything (i.e. array is full, or nothing in DB)
//   sleep for N seconds
// - If an enumerated job was already in the array,
//   stop the scan and sleep for N seconds
// - Otherwise immediately start another scan

// If --allapps is used:
// - there are separate DB enumerators for each app
// - the work array is interleaved by application, based on their weights.
//   slot_to_app[] maps slot (i.e. work array index) to app index.
//   app_count[] is the number of slots per app
//   (approximately proportional to its weight)

// Homogeneous redundancy (HR):
// If HR is used, jobs can either be "uncommitted"
// (can send to any HR class)
// or "committed" (can send only to one HR class).
// The feeder tries to maintain a ratio of committed to uncommitted
// (generally 50/50) and, of committed jobs, ratios between HR classes
// (proportional to the total RAC of hosts in that class).
// This is to maximize the likelihood of having work for an average host.
//
// If you use different HR types between apps, you must use --allapps.
// Otherwise we wouldn't know how many slots to reserve for each HR type.
//
// It's OK to use HR for some apps and not others.

// Trigger files:
// The feeder program periodically checks for two trigger files:
//
// stop_server:  destroy shmem and exit
//               leave trigger file there (for other daemons)
// reread_db:    update DB contents in existing shmem
//               delete trigger file

// If you get an "Invalid argument" error when trying to run the feeder,
// it is likely that you aren't able to allocate enough shared memory.
// Either increase the maximum shared memory segment size in the kernel
// configuration, or decrease the MAX_PLATFORMS, MAX_APPS
// MAX_APP_VERSIONS, and MAX_WU_RESULTS in sched_shmem.h

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <vector>
using std::vector;

#include "version.h"
#include "boinc_db.h"
#include "shmem.h"
#include "error_numbers.h"
#include "synch.h"
#include "util.h"
#include "str_util.h"
#include "svn_version.h"

#include "credit.h"
#include "sched_config.h"
#include "sched_shmem.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "hr_info.h"
#ifdef GCL_SIMULATOR
#include "gcl_simulator.h"
#endif

#define DEFAULT_SLEEP_INTERVAL  5
#define AV_UPDATE_PERIOD      600

#define REREAD_DB_FILENAME      "reread_db"

#define ENUM_FIRST_PASS     0
#define ENUM_SECOND_PASS    1
#define ENUM_OVER           2

SCHED_SHMEM* ssp;
key_t sema_key;
const char* order_clause="";
char mod_select_clause[256];
int sleep_interval = DEFAULT_SLEEP_INTERVAL;
bool all_apps = false;
int purge_stale_time = 0;
int num_work_items = MAX_WU_RESULTS;
int enum_limit = MAX_WU_RESULTS*2;

// The following defined if --allapps:
int *enum_sizes;
    // the enum size per app; else not used
int *app_indices;
    // maps slot number to app index, else all zero
int napps;
    // number of apps, else one

HR_INFO hr_info;
bool using_hr;
    // true iff any app is using HR
bool is_main_feeder = true;
    // false if using --mod or --wmod and this one isn't 0

void signal_handler(int) {
    log_messages.printf(MSG_NORMAL, "Signaled by simulator\n");
    return;
}

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

// Count the # of slots used by HR classes.
// This is done at the start of each array scan,
// and doesn't reflect slots that have been emptied out by the scheduler
//
void hr_count_slots() {
    int i, j;

    for (i=1; i<HR_NTYPES; i++) {
        if (!hr_info.type_being_used[i]) continue;
        for (j=0; j<hr_nclasses[i]; j++) {
            hr_info.cur_slots[i][j] = 0;
        }
    }
    for (i=0; i<ssp->max_wu_results; i++) {
        int app_index = app_indices[i];
        int hrt = ssp->apps[app_index].homogeneous_redundancy;
        if (!hrt) continue;

        WU_RESULT& wu_result = ssp->wu_results[i];
        if (wu_result.state == WR_STATE_PRESENT) {
            int hrc = wu_result.workunit.hr_class;
            if (hrc < 0 || hrc >= hr_nclasses[hrt]) {
                log_messages.printf(MSG_CRITICAL,
                    "HR class %d is out of range\n", hrc
                );
                continue;
            }
            hr_info.cur_slots[hrt][hrc]++;
        }
    }
}

// Enumerate jobs from DB until find one that is not already in the work array.
// If find one, return true.
// If reach end of enum for second time on this array scan, return false
// 
static bool get_job_from_db(
    DB_WORK_ITEM& wi,    // enumerator to get job from
    int app_index,       // if using --allapps, the app index
    int& enum_phase,
    int& ncollisions
) {
    bool collision;
    int retval, j, enum_size;
    char select_clause[256];
    
    if (all_apps) {
        sprintf(select_clause, "%s and r1.appid=%d",
            mod_select_clause, ssp->apps[app_index].id
        );
        enum_size = enum_sizes[app_index];
    } else {
        strcpy(select_clause, mod_select_clause);
        enum_size = enum_limit;
    }
    int hrt = ssp->apps[app_index].homogeneous_redundancy;

    while (1) {
        if (hrt && config.hr_allocate_slots) {
            retval = wi.enumerate_all(enum_size, select_clause);
        } else {
            retval = wi.enumerate(enum_size, select_clause, order_clause);
        }
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                // If DB server dies, exit;
                // so /start (run from crontab) will restart us eventually.
                //
                log_messages.printf(MSG_CRITICAL,
                    "DB connection lost, exiting\n"
                );
                exit(0);
            }

            // we've reach the end of the result set
            //
            switch (enum_phase) {
            case ENUM_FIRST_PASS:
                enum_phase = ENUM_SECOND_PASS;
                ncollisions = 0;
                    // disregard collisions - maybe we'll find new jobs
                break;
            case ENUM_SECOND_PASS:
                enum_phase = ENUM_OVER;
                return false;
            }
            log_messages.printf(MSG_NORMAL,
                "restarted enumeration for appid %d\n",
                ssp->apps[app_index].id
            );
        } else {
            // Check for invalid application ID
            //
            if (!ssp->lookup_app(wi.wu.appid)) {
#if 0
                log_messages.printf(MSG_CRITICAL,
                    "result [RESULT#%u] has bad appid %d; clean up your DB!\n",
                    wi.res_id, wi.wu.appid
                );
#endif
                continue;
            }
            
            // Check for collision (i.e. this result already is in the array)
            //
            collision = false;
            for (j=0; j<ssp->max_wu_results; j++) {
                if (ssp->wu_results[j].state != WR_STATE_EMPTY && ssp->wu_results[j].resultid == wi.res_id) {
                    // If the result is already in shared mem,
                    // and another instance of the WU has been sent,
                    // bump the infeasible count to encourage
                    // it to get sent more quickly
                    //
                    if (ssp->wu_results[j].infeasible_count == 0) {
                        if (wi.wu.hr_class > 0) {
                            ssp->wu_results[j].infeasible_count++;
                        }
                    }
                    ncollisions++;
                    collision = true;
                    log_messages.printf(MSG_DEBUG,
                        "result [RESULT#%u] already in array\n", wi.res_id
                    );
                    break;
                }
            }
            if (collision) {
                continue;
            }

            // if using HR, check whether we've exceeded quota for this class
            //
            if (hrt && config.hr_allocate_slots) {
                if (!hr_info.accept(hrt, wi.wu.hr_class)) {
                    log_messages.printf(MSG_DEBUG,
                        "rejecting [RESULT#%u] because HR class %d/%d over quota\n",
                        wi.res_id, hrt, wi.wu.hr_class
                    );
                    continue;
                }
            }
            return true;
        }
    }
    return false;   // never reached
}

// This function decides the interleaving used for --allapps.
// Inputs:
//   n (number of weights)
//   k (length of vector)
//   a set of weights w(0)..w(n-1)
// Outputs:
//   a vector v(0)..v(k-1) with values 0..n-1,
//     where each value occurs with the given weight,
//     and values are interleaved as much as possible.
//   a vector count(0)..count(n-1) saying how many times
//     each value occurs in v
//
void weighted_interleave(double* weights, int n, int k, int* v, int* count) {
    double *x = (double*) calloc(n, sizeof(double));
    int i;
    for (i=0; i<n; i++) {
        // make sure apps with no weight get no slots
        if (weights[i] == 0) {
            x[i] = 1e-100;
        }
        count[i] = 0;
    }
    for (i=0; i<k; i++) {
        int best = 0;
        for (int j=1; j<n; j++) {
            if (x[j] > x[best]) {
                best = j;
            }
        }
        v[i] = best;
        x[best] -= 1/weights[best];
        count[best]++;
    }
    free(x);
}

// update the job size statistics fields of array entries
//
static void update_job_stats() {
    int i, n=0;
    double sum=0, sum_sqr=0;

    for (i=0; i<ssp->max_wu_results; i++) {
        WU_RESULT& wu_result = ssp->wu_results[i];
        if (wu_result.state != WR_STATE_PRESENT) continue;
        n++;
        double e = wu_result.workunit.rsc_fpops_est;
        sum += e;
        sum_sqr += e*e;
    }
    double mean = sum/n;
    double stdev = sqrt((sum_sqr - sum*mean)/n);
    for (i=0; i<ssp->max_wu_results; i++) {
        WU_RESULT& wu_result = ssp->wu_results[i];
        if (wu_result.state != WR_STATE_PRESENT) continue;
        double e = wu_result.workunit.rsc_fpops_est;
        double diff = e - mean;
        wu_result.fpops_size = diff/stdev;
    }
}

// We're purging this item because it's been in shared mem too long.
// In general it will get added again soon.
// But if it's committed to an HR class,
// it could be because it got sent to a rare host.
// Un-commit it by zeroing out the WU's hr class,
// and incrementing target_nresults
//
static void purge_stale(WU_RESULT& wu_result) {
    DB_WORKUNIT wu;
    wu.id = wu_result.workunit.id;
    if (wu_result.workunit.hr_class) {
        char buf[256];
        sprintf(buf,
            "hr_class=0, target_nresults=target_nresults+1, transition_time=%ld",
            time(0)
        );
        wu.update_field(buf);
    }
}

// Make one pass through the work array, filling in empty slots.
// Return true if we filled in any.
//
static bool scan_work_array(vector<DB_WORK_ITEM> &work_items) {
    int i;
    bool found;
    int enum_phase[napps];
    int app_index;
    int nadditions=0, ncollisions=0;
    
      for (i=0; i<napps; i++) {
        if (work_items[i].cursor.active) {
            enum_phase[i] = ENUM_FIRST_PASS;
        } else {
            enum_phase[i] = ENUM_SECOND_PASS;
        }
    }

    if (using_hr && config.hr_allocate_slots) {
        hr_count_slots();
    }

    for (i=0; i<ssp->max_wu_results; i++) {
        app_index = app_indices[i];

        DB_WORK_ITEM& wi = work_items[app_index];
        WU_RESULT& wu_result = ssp->wu_results[i];
        switch (wu_result.state) {
        case WR_STATE_PRESENT:
            if (purge_stale_time && wu_result.time_added_to_shared_memory < (time(0) - purge_stale_time)) {
                log_messages.printf(MSG_NORMAL,
                    "remove result [RESULT#%d] from slot %d because it is stale\n",
                    wu_result.resultid, i
                );
                purge_stale(wu_result);
                wu_result.state = WR_STATE_EMPTY;
                // fall through, refill this array slot
            } else {
                break;
            }
        case WR_STATE_EMPTY:
            if (enum_phase[app_index] == ENUM_OVER) continue;
            found = get_job_from_db(
                wi, app_index, enum_phase[app_index], ncollisions
            );
            if (found) {
                log_messages.printf(MSG_NORMAL,
                    "adding result [RESULT#%u] in slot %d\n",
                    wi.res_id, i
                );
                wu_result.resultid = wi.res_id;
                wu_result.res_priority = wi.res_priority;
                wu_result.res_server_state = wi.res_server_state;
                wu_result.res_report_deadline = wi.res_report_deadline;
                wu_result.workunit = wi.wu;
                wu_result.state = WR_STATE_PRESENT;
                // If the workunit has already been allocated to a certain
                // OS then it should be assigned quickly,
                // so we set its infeasible_count to 1
                //
                if (wi.wu.hr_class > 0) {
                    wu_result.infeasible_count = 1;
                } else {
                    wu_result.infeasible_count = 0;
                }
                // set the need_reliable flag if needed
                //
                wu_result.need_reliable = false;
                if (config.reliable_on_priority && wu_result.res_priority >= config.reliable_on_priority) {
                    wu_result.need_reliable = true;
                }
                wu_result.time_added_to_shared_memory = time(0);
                nadditions++;
            }
            break;
        default:
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
    if (ncollisions) {
        log_messages.printf(MSG_DEBUG,
            "%d results already in array\n", ncollisions
        );
        return false;
    }
    if (nadditions == 0) {
        return false;
    }
    return true;
}

void feeder_loop() {
    vector<DB_WORK_ITEM> work_items;
    double next_av_update_time=0;
    
    // may need one enumeration per app; create vector
    //
    for (int i=0; i<napps; i++) {
        DB_WORK_ITEM* wi = new DB_WORK_ITEM();
        work_items.push_back(*wi);
    }

    while (1) {
        bool action;
        if (config.dont_send_jobs) {
            action = false;
        } else {
            action = scan_work_array(work_items);
        }
        ssp->ready = true;
        if (!action) {
#ifdef GCL_SIMULATOR
            continue_simulation("feeder");
            log_messages.printf(MSG_DEBUG, "Waiting for signal\n");
            signal(SIGUSR2, simulator_signal_handler);
            pause();
#else
            log_messages.printf(MSG_DEBUG,
                "No action; sleeping %d sec\n", sleep_interval
            );
            daemon_sleep(sleep_interval);
#endif
        } else {
            if (config.job_size_matching) {
                update_job_stats();
            }
        }

        double now = dtime();
        if (is_main_feeder && now > next_av_update_time) {
            int retval = update_av_scales(ssp);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "update_av_scales failed: %s\n", boincerror(retval)
                );
                exit(1);
            }
            next_av_update_time = now + AV_UPDATE_PERIOD;
        }
        fflush(stdout);
        check_stop_daemons();
        check_reread_trigger();
    }
}

// see if we're using HR, and if so initialize the necessary data structures
//
void hr_init() {
    int i, retval;
    bool apps_differ = false;
    bool some_app_uses_hr = false;
    int hrt, hr_type0 = ssp->apps[0].homogeneous_redundancy;

    using_hr = false;

    for (i=0; i<ssp->napps; i++) {
        hrt = ssp->apps[i].homogeneous_redundancy;
        if (hrt <0 || hrt >= HR_NTYPES) {
            log_messages.printf(MSG_CRITICAL,
                "HR type %d out of range for app %d\n", hrt, i
            );
            exit(1);
        }
        if (hrt) some_app_uses_hr = true;
        if (hrt != hr_type0) apps_differ = true;
    }
    if (config.homogeneous_redundancy) {
        log_messages.printf(MSG_NORMAL,
            "config HR is %d\n", config.homogeneous_redundancy
        );
        hrt = config.homogeneous_redundancy;
        if (hrt < 0 || hrt >= HR_NTYPES) {
            log_messages.printf(MSG_CRITICAL,
                "Main HR type %d out of range\n", hrt
            );
            exit(1);
        }
        if (some_app_uses_hr) {
            log_messages.printf(MSG_CRITICAL,
                "You can specify HR at global or app level, but not both\n"
            );
            exit(1);
        }
        for (i=0; i<ssp->napps; i++) {
            ssp->apps[i].homogeneous_redundancy = config.homogeneous_redundancy;
            ssp->apps[i].weight = 1;
        }

    } else {
        if (some_app_uses_hr) {
            if (apps_differ && !all_apps) {
                log_messages.printf(MSG_CRITICAL,
                    "You must use --allapps if apps have different HR\n"
                );
                exit(1);
            }
        } else {
            return;     // HR not being used
        }
    }
    using_hr = true;
    if (config.hr_allocate_slots) {
        hr_info.init();
        retval = hr_info.read_file();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "Can't read HR info file: %s\n", boincerror(retval)
            );
            exit(1);
        }

        // find the weight for each HR type
        //
        for (i=0; i<ssp->napps; i++) {
            hrt = ssp->apps[i].homogeneous_redundancy;
            hr_info.type_weights[hrt] += ssp->apps[i].weight;
            hr_info.type_being_used[hrt] = true;
        }

        // compute the slot allocations for HR classes
        //
        hr_info.allocate(ssp->max_wu_results);
        hr_info.show(stderr);
    }
}

// write a summary of feeder state to stderr
//
void show_state(int) {
    ssp->show(stderr);
    if (config.hr_allocate_slots) {
        hr_info.show(stderr);
    }
}

void show_version() {
    log_messages.printf(MSG_NORMAL, "%s\n", SVN_VERSION);
}

void usage(char *name) {
    fprintf(stderr,
        "%s creates a shared memory segment containing DB info,\n"
        "including an array of work items (results/workunits to send).\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ -d X | --debug_level X]        Set log verbosity to X (1..4)\n"
        "  [ --allapps ]                    Interleave results from all applications uniformly.\n"
        "  [ --random_order ]               order by \"random\" field of result\n"
        "  [ --priority_asc ]               order by increasing \"priority\" field of result\n"
        "  [ --priority_order ]             order by decreasing \"priority\" field of result\n"
        "  [ --priority_order_create_time ] order by priority, then by increasing WU create time\n"
        "  [ --purge_stale x ]              remove work items from the shared memory segment after x secs\n"
        "                                   that have been there for longer then x minutes\n"
        "                                   but haven't been assigned\n"
        "  [ --appids a1{,a2} ]             get work only for appids a1,... (comma-separated list)\n"
        "  [ --mod n i ]                    handle only results with (id mod n) == i\n"
        "  [ --wmod n i ]                   handle only workunits with (id mod n) == i\n"
        "  [ --sleep_interval x ]           sleep x seconds if nothing to do\n"
        "  [ -h | --help ]                  Shows this help text.\n"
        "  [ -v | --version ]               Shows version information.\n",
        name, name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    void* p;
    char path[MAXPATHLEN], order_buf[1024];

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "d") || is_arg(argv[i], "debug_level")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (is_arg(argv[i], "random_order")) {
            order_clause = "order by r1.random ";
        } else if (is_arg(argv[i], "allapps")) {
            all_apps = true;
        } else if (is_arg(argv[i], "priority_asc")) {
            order_clause = "order by r1.priority asc ";
        } else if (is_arg(argv[i], "priority_order")) {
            order_clause = "order by r1.priority desc ";
        } else if (is_arg(argv[i], "priority_order_create_time")) {
            order_clause = "order by r1.priority desc, r1.workunitid";
        } else if (is_arg(argv[i], "by_batch")) {
            // Evenly distribute work among batches
            // The 0=1 causes anything before the union statement
            // to result in an empty set,
            // and the '#' at the end comments out anthing following our query
            // This has allowed us to inject a more customizable query
            //
            sprintf(order_buf, "and 0=1 union (SELECT r1.id, r1.priority, r1.server_state, r1.report_deadline, workunit.* FROM workunit JOIN ("
                "SELECT *, CASE WHEN @batch != t.batch THEN @rownum := 0 WHEN @batch = t.batch THEN @rownum := @rownum + 1 END AS rank, @batch := t.batch "
                "FROM (SELECT @rownum := 0, @batch := 0, r.* FROM result r WHERE r.server_state=2 ORDER BY batch) t) r1 ON workunit.id=r1.workunitid "
                "ORDER BY rank LIMIT %d)#",
                enum_limit
            );
            order_clause = order_buf;      
        } else if (is_arg(argv[i], "purge_stale")) {
            purge_stale_time = atoi(argv[++i])*60;
        } else if (is_arg(argv[i], "appids")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            strcat(mod_select_clause, " and workunit.appid in (");
            strcat(mod_select_clause, argv[i]);
            strcat(mod_select_clause, ")");
        } else if (is_arg(argv[i], "mod")) {
            if (!argv[i+1] || !argv[i+2]) {
                log_messages.printf(MSG_CRITICAL, "%s requires two arguments\n\n", argv[i]);
                usage(argv[0]);
                exit(1);
            }
            int n = atoi(argv[++i]);
            int j = atoi(argv[++i]);
            sprintf(mod_select_clause, "and r1.id %% %d = %d ", n, j);
            is_main_feeder = (j==0);
        } else if (is_arg(argv[i], "wmod")) {
            if (!argv[i+1] || !argv[i+2]) {
                log_messages.printf(MSG_CRITICAL, "%s requires two arguments\n\n", argv[i]);
                usage(argv[0]);
                exit(1);
            }
            int n = atoi(argv[++i]);
            int j = atoi(argv[++i]);
            sprintf(mod_select_clause, "and workunit.id %% %d = %d ", n, j);
            is_main_feeder = (j==0);
        } else if (is_arg(argv[i], "sleep_interval")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            sleep_interval = atoi(argv[i]);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            show_version();
            exit(0);
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    unlink(config.project_path(REREAD_DB_FILENAME));

    log_messages.printf(MSG_NORMAL, "Starting\n");
    show_version();

    if (config.feeder_query_size) {
        enum_limit = config.feeder_query_size;
    }
    if (config.shmem_work_items) {
        num_work_items = config.shmem_work_items;
    }
    strncpy(path, config.project_dir, sizeof(path));
    get_key(path, 'a', sema_key);
    destroy_semaphore(sema_key);
    create_semaphore(sema_key);

    retval = destroy_shmem(config.shmem_key);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't destroy shmem\n");
        exit(1);
    }

    int shmem_size = sizeof(SCHED_SHMEM) + num_work_items*sizeof(WU_RESULT);
    retval = create_shmem(config.shmem_key, shmem_size, 0 /* don't set GID */, &p);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't create shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    ssp->init(num_work_items);

    atexit(cleanup_shmem);
    install_stop_signal_handler();

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

    app_indices = (int*) calloc(ssp->max_wu_results, sizeof(int));

    // If all_apps is set, make an array saying which array slot
    // is associated with which app
    //
    if (all_apps) {
        napps = ssp->napps;
        enum_sizes = (int*) calloc(ssp->napps, sizeof(int));
        double* weights = (double*) calloc(ssp->napps, sizeof(double));
        int* counts = (int*) calloc(ssp->napps, sizeof(int));
        if (ssp->app_weight_sum == 0) {
            for (i=0; i<ssp->napps; i++) {
                ssp->apps[i].weight = 1;
            }
            ssp->app_weight_sum = ssp->napps;
        }
        for (i=0; i<ssp->napps; i++) {
            weights[i] = ssp->apps[i].weight;
        }
        for (i=0; i<ssp->napps; i++) {
            enum_sizes[i] = (int) floor(0.5 + enum_limit*(weights[i])/(ssp->app_weight_sum));
        }
        weighted_interleave(
            weights, ssp->napps, ssp->max_wu_results, app_indices, counts
        );
        free(weights);
        free(counts);
    } else {
        napps = 1;
    }

    hr_init();

    if (using_hr && strlen(order_clause)) {
        log_messages.printf(MSG_CRITICAL,
            "Note: ordering options will not apply to apps for which homogeneous redundancy is used\n"
        );
    }

    retval = ssp->perf_info.get_from_db();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "PERF_INFO::get_from_db(): %d\n", retval
        );
    }

    signal(SIGUSR1, show_state);

    feeder_loop();
}

const char *BOINC_RCSID_57c87aa242 = "$Id$";
