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

// -------------------------------
//
// feeder
//  [ -d x ]              debug level x
//  [ -random_order ]     order by "random" field of result
//  [ -priority_order ]   order by decreasing "priority" field of result
//  [ -priority_order_create_time ]
//                        order by priority, then by increasing WU create time
//  [ -mod n i ]          handle only results with (id mod n) == i
//  [ -sleep_interval x ]   sleep x seconds if nothing to do
//  [ -allapps ]  		  interleave results from all applications uniformly
//  [ -purge_stale x ]    remove work items from the shared memory segment
//                        that have been there for longer then x minutes
//                        but haven't been assigned
//
// Creates a shared memory segment containing DB info,
// including an array of work items (results/workunits to send).
//
// feeder tries to keep the work array filled.
// This is a little tricky.
// We use a DB enumerator.
// The inner loop scans the wu_result table,
// looking for empty slots and trying to fill them in.
// When the enumerator reaches the end, it is restarted;
// hopefully there will be some new workunits.
// There are two complications:
//
//  - An enumeration may return results already in the array.
//    So, for each result, we scan the entire array to make sure
//    it's not there already.  Can this be streamlined?
//
//  - We must avoid excessive re-enumeration,
//    especially when the number of results is less than the array size.
//    Crude approach: if a "collision" (as above) occurred on
//    a pass through the array, wait a long time (5 sec)
//
// Trigger files:
// The feeder program periodically checks for two trigger files:
//
// stop_server:  destroy shmem and exit
//               leave trigger file there (for other daemons)
// reread_db:    update DB contents in existing shmem
//               delete trigger file

// If -allapps is used, the work array is interleaved by application,
// based on their weights.
// slot_to_app[] maps slot (i.e. work array index) to app index.
// app_count[] is the number of slots per app
// (approximately proportional to its weight)

// If you get an "Invalid argument" error when trying to run the feeder,
// it is likely that you aren't able to allocate enough shared memory.
// Either increase the maximum shared memory segment size in the kernel
// configuration, or decrease the MAX_PLATFORMS, MAX_APPS
// MAX_APP_VERSIONS, and MAX_WU_RESULTS in sched_shmem.h

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
using std::vector;

#include "boinc_db.h"
#include "shmem.h"
#include "error_numbers.h"
#include "synch.h"
#include "util.h"
#include "str_util.h"

#include "sched_config.h"
#include "sched_shmem.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define DEFAULT_SLEEP_INTERVAL  5

#define REREAD_DB_FILENAME      "../reread_db"

SCHED_CONFIG config;
SCHED_SHMEM* ssp;
key_t sema_key;
const char* order_clause="";
char mod_select_clause[256];
double sleep_interval = DEFAULT_SLEEP_INTERVAL;
bool all_apps = false;
int purge_stale_time = 0;
int num_work_items = MAX_WU_RESULTS;
int enum_limit = MAX_WU_RESULTS*2;
int *enum_sizes;
int *app_indices;

void cleanup_shmem() {
    ssp->ready = false;
    detach_shmem((void*)ssp);
    destroy_shmem(config.shmem_key);
}

int check_reread_trigger() {
    FILE* f;
    f = fopen(REREAD_DB_FILENAME, "r");
    if (f) {
        fclose(f);
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "Found trigger file %s; re-scanning database tables.\n",
            REREAD_DB_FILENAME
        );
        ssp->init(num_work_items);
        ssp->scan_tables();
        unlink(REREAD_DB_FILENAME);
        log_messages.printf(
            SCHED_MSG_LOG::MSG_NORMAL,
            "Done re-scanning: trigger file removed.\n"
        );
    }
    return 0;
}

// Scan DB for work items until one found
// that is not already in the shared memory segment.
// Errors that can occur:
// 1) No valid work item found even after restarting the enumeration
//   ACTION: return false
// 2) The work item is for a app that doesn't exist in the database
//   ACTION: exit application
// 
static bool find_work_item(
    DB_WORK_ITEM wi,    // if -allapps, array of enumerators; else just one
    int app_index,      // if using -allapps, the app index
    bool& restarted_enum,
    int& ncollisions
) {
	bool in_second_pass = false;
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

	if (!wi.cursor.active) {
        if (restarted_enum) {
            return false;
        }
		in_second_pass = true;
	}
	do {
        // if we have restarted the enum then we are in the second pass
        //
        if (!in_second_pass && restarted_enum ) {
        	in_second_pass = true;
        	ncollisions = 0;
        }
   		retval = wi.enumerate(enum_size, select_clause, order_clause);
   		// if retval is nonzero, we have reached the end of the result set
        // and we need to requery the database
        //
    	if (retval) {
    		restarted_enum = true;
			log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
            	"restarted enumeration for appid %d\n",
                ssp->apps[app_index].id
            );
        } else {
        	// Check for a work item with an invalid application id
            //
            if (!ssp->lookup_app(wi.wu.appid)) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
                    "result [RESULT#%d] has bad appid %d; clean up your DB!\n",
                    wi.res_id, wi.wu.appid
                );
                exit(1);
            }
            
            // Check for collision
            // (i.e. this result already is in the array)
            // If collision, then advance to the next workitem
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
                    collision = false;
                    break;
                }
            }
            if (!collision) return true;
    	}
        // exit conditions
        // (in_second_pass && retval) means if we have looped a second time
        // and reached the end of the result set without finding a workitem.
        // This is an error.
        // found means that we identified a valid work item
        //
	} while ((!in_second_pass || !retval));
	return false;
}

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
    for (i=0; i<n; i++) count[i] = 0;
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

// Make one pass through the work array, filling in empty slots
//
static void scan_work_array(
    vector<DB_WORK_ITEM> &work_items, int& nadditions, int& ncollisions
) {
    int i;
    bool found;
    bool restarted_enum[ssp->napps];
    int app_index;
    int enum_size;
    
  	for (i=0; i < ssp->napps; i++) {
    	restarted_enum[i] = false;
    }

    for (i=0; i<ssp->max_wu_results; i++) {
        app_index = app_indices[i];
    	DB_WORK_ITEM& wi = work_items[app_index];
        WU_RESULT& wu_result = ssp->wu_results[i];
        switch (wu_result.state) {
        case WR_STATE_PRESENT:
       	  	if (purge_stale_time && wu_result.time_added_to_shared_memory < (time(0) - purge_stale_time)) {
    			wu_result.state = WR_STATE_EMPTY;
				log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
                    "remove result [RESULT#%d] from slot %d because it is stale\n",
                    wu_result.resultid, i
                );
            } else {
            	break;
        	}
        case WR_STATE_EMPTY:
            found = find_work_item(
                wi, app_index,
                restarted_enum[app_index], ncollisions
            );
            if (found) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "adding result [RESULT#%d] in slot %d\n",
                    wi.res_id, i
                );
                wu_result.resultid = wi.res_id;
                wu_result.result_priority = wi.res_priority;
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
                // If using the reliable mechanism, then set the results for
                // workunits older then the specificed time as needing a reliable
                // host
                wu_result.need_reliable = 0;
                if (config.reliable_time) {
                	if ((wu_result.workunit.create_time + config.reliable_time) <= time(0)) {
                		wu_result.need_reliable = true;
                	}
                }
                if (config.reliable_on_priority && wu_result.result_priority >= config.reliable_on_priority) {
                    wu_result.need_reliable = true;
                }
                wu_result.time_added_to_shared_memory = time(NULL);
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
            if (stat(buf, &s)) {
                wu_result.state = WR_STATE_PRESENT;
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "Result reserved by non-existent process PID %d; resetting\n",
                    pid
                );
            }
        }
    }
}

void feeder_loop() {
    int nadditions, ncollisions;
    vector<DB_WORK_ITEM> work_items;
    DB_WORK_ITEM* wi;
    
    if (all_apps) {   
    	for(int i=0; i<ssp->napps; i++) {
    		wi = new DB_WORK_ITEM();
    		work_items.push_back(*wi);
    	}
    } else {
    	wi = new DB_WORK_ITEM();
    	work_items.push_back(*wi);
    }

    while (1) {
        nadditions = 0;
        ncollisions = 0;

        scan_work_array(work_items, nadditions, ncollisions);

        ssp->ready = true;

        if (nadditions == 0) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG, "No results added; sleeping %.2f sec\n", sleep_interval);
            boinc_sleep(sleep_interval);
        } else {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG, "Added %d results to array\n", nadditions);
        }
        if (ncollisions) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG, "Some results already in array - sleeping %.2f sec\n", sleep_interval);
            boinc_sleep(sleep_interval);
        }
        fflush(stdout);
        check_stop_daemons();
        check_reread_trigger();
    }
}

int main(int argc, char** argv) {
    int i, retval;
    void* p;
    char path[256];

    unlink(REREAD_DB_FILENAME);

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "Can't parse ../config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-random_order")) {
            order_clause = "order by r1.random ";
        } else if (!strcmp(argv[i], "-allapps")) {
            all_apps = true;
        } else if (!strcmp(argv[i], "-priority_order")) {
            order_clause = "order by r1.priority desc ";
        } else if (!strcmp(argv[i], "-priority_order_create_time")) {
            order_clause = "order by r1.priority desc, workunit.create_time ";
        } else if (!strcmp(argv[i], "-purge_stale")) {
            purge_stale_time = atoi(argv[++i])*60;
        } else if (!strcmp(argv[i], "-mod")) {
            int n = atoi(argv[++i]);
            int j = atoi(argv[++i]);
            sprintf(mod_select_clause, "and r1.id %% %d = %d ", n, j);
        } else if (!strcmp(argv[i], "-sleep_interval")) {
            sleep_interval = atof(argv[++i]);
        } else {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "bad cmdline arg: %s\n", argv[i]
            );
            exit(1);
        }
    }

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "Starting\n");

    if (config.feeder_query_size) {
        enum_limit = config.feeder_query_size;
    }
    if (config.shmem_work_items) {
        num_work_items = config.shmem_work_items;
    }
    get_project_dir(path, sizeof(path));
    get_key(path, 'a', sema_key);
    destroy_semaphore(sema_key);
    create_semaphore(sema_key);

    retval = destroy_shmem(config.shmem_key);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't destroy shmem\n");
        exit(1);
    }

    int shmem_size = sizeof(SCHED_SHMEM) + num_work_items*sizeof(WU_RESULT);
    retval = create_shmem(config.shmem_key, shmem_size, 0 /* don't set GID */, &p);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "can't create shmem\n");
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
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "boinc_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        exit(1);
    }
    retval = boinc_db.set_isolation_level(READ_UNCOMMITTED);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "boinc_db.set_isolation_level: %d; %s\n", retval, boinc_db.error_string()
        );
    }
    ssp->scan_tables();

    log_messages.printf(
        SCHED_MSG_LOG::MSG_NORMAL,
        "feeder: read "
        "%d platforms, "
        "%d apps, "
        "%d app_versions\n",
        ssp->nplatforms,
        ssp->napps,
        ssp->napp_versions
    );

    app_indices = (int*) calloc(ssp->max_wu_results, sizeof(int));
    enum_sizes = (int*) calloc(ssp->napps, sizeof(int));
    double* weights = (double*) calloc(ssp->napps, sizeof(double));
    int* counts = (int*) calloc(ssp->napps, sizeof(int));

    // If all_apps is set, make an array saying which array slot
    // is associated with which app
    //
    if (all_apps) {
        if (ssp->app_weights == 0) {
            for (i=0; i<ssp->napps; i++) {
                ssp->apps[i].weight = 1;
            }
            ssp->app_weights = ssp->napps;
        }
        for (i=0; i<ssp->napps; i++) {
            weights[i] = ssp->apps[i].weight;
        }
        for (i=0; i<ssp->napps; i++) {
            enum_sizes[i] = (int) floor(0.5 + enum_limit*(weights[i])/(ssp->app_weights));
        }
        weighted_interleave(
            weights, ssp->napps, ssp->max_wu_results, app_indices, counts
        );
    }
    feeder_loop();
}

const char *BOINC_RCSID_57c87aa242 = "$Id$";
