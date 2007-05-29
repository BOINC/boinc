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
// including the work array (results/workunits to send).
//
// Try to keep the work array filled.
// This is a little tricky.
// We use an enumerator.
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
char select_clause[256];
double sleep_interval = DEFAULT_SLEEP_INTERVAL;
bool all_apps = false;
int purge_stale_time = 0;
int num_work_items = MAX_WU_RESULTS;
int enum_limit = MAX_WU_RESULTS*2;

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

// Scan work items for a given appp until one found
// that is not already on the shared memory segment.
// Errors that can occur:
// 1) No valid work item found even after restarting the enumeration
//   ACTION: return false
// 2) The work item can be for a app that doesn't exist in the database
//   ACTION: exit application
// 
static bool find_work_item(
    DB_WORK_ITEM *wi, bool& restarted_enum, int& ncollisions,
    int work_item_index, int enum_size, char* mod_select_clause
) {
	bool in_second_pass = false;
	bool found = false;
	int retval, j;
	
	if (!wi->cursor.active && restarted_enum) {
		return false;
	} else if (!wi->cursor.active) {
		in_second_pass = true;
	}
	do {
        // if we have restarted the enum then we are in the second pass
        //
        if (!in_second_pass && restarted_enum ) {
        	in_second_pass = true;
        	ncollisions = 0;
        }
   		retval = wi->enumerate(enum_size, mod_select_clause, order_clause);
   		// if retval is not 0 (i.e. true),
        // then we have reached the end of the result
        // and we need to requery the database
        //
    	if (retval) {
    		restarted_enum = true;
			log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
            	"restarted enumeration for appid %d\n",
                ssp->apps[work_item_index].id);
        } else {
        	// Check for a work item with an invalid application id
            //
            if (!ssp->lookup_app(wi->wu.appid)) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
                    "result [RESULT#%d] has bad appid %d; clean up your DB!\n",
                    wi->res_id, wi->wu.appid
                );
                exit(1);
            }
            
            // Check for collision
            // (i.e. this result already is in the array)
            // If collision, then advance to the next workitem
            //
            found = true;
            for (j=0; j<ssp->max_wu_results; j++) {
                if (ssp->wu_results[j].state != WR_STATE_EMPTY && ssp->wu_results[j].resultid == wi->res_id) {
                	// If the result is already in shared mem,
                	// and another instance of the WU has been sent,
                	// bump the infeasible count to encourage
                	// it to get sent more quickly
                	//
                	if (ssp->wu_results[j].infeasible_count == 0) {
                		if (wi->wu.hr_class > 0) {
                			ssp->wu_results[j].infeasible_count++;
                		}
                	}
                    ncollisions++;
                    found = false;  // not found if it is a collision
                    break;
                }
            }    		
    	}
        // exit conditions
        // (in_second_pass && retval) means if we have looped a second time
        // and reached the end of the result set without finding a workitem.
        // This is an error.
        // found means that we identified a valid work item
        //
	} while ((!in_second_pass || !retval) && !found);
	return found;
}

static int find_work_item_index(int slot_pos) {
	if (ssp->app_weights == 0) return 0;
	int mod = slot_pos % (int)(ssp->app_weights);
	int work_item_index = -1;
	for (int i=0; i<ssp->napps; i++) {
		if (ssp->apps[i].weight < 1) continue;
		if (mod < ssp->apps[i].weight) {
			work_item_index = i;
			break;
		} else {
			mod = mod - (int)ssp->apps[i].weight;
		}
	}
	// The condition below will occur if all projects have a weight of 0
	if ( work_item_index == -1 ) work_item_index = 0;
	return work_item_index;
}

static void scan_work_array(
    vector<DB_WORK_ITEM>* work_items, int& nadditions, int& ncollisions
) {
    int i;
    bool found;
    bool restarted_enum[ssp->napps];
    DB_WORK_ITEM* wi;
    char mod_select_clause[256];
    int work_item_index;
    int enum_size;
    
  	for (i=0; i < ssp->napps; i++) {
    	restarted_enum[i] = false;
    }

    for (i=0; i<ssp->max_wu_results; i++) {
   	    // If all_apps is set then every nth item in the shared memory segment
        // will be assigned to the application stored in that index in ssp->apps
        //
    	if (all_apps) {
    		if (ssp->app_weights > 0) {
    			work_item_index = find_work_item_index(i);
    			enum_size = (int) floor(0.5 + enum_limit*(ssp->apps[work_item_index].weight)/(ssp->app_weights));
    		} else {
    			// If all apps have a weight of zero then evenly
                // distribute the slots
                //
    			work_item_index = i % ssp->napps;
    			enum_size = (int) floor(0.5 + ((double)enum_limit)/ssp->napps);
    		}
    		sprintf(mod_select_clause, "%s and r1.appid=%d",
    		    select_clause, ssp->apps[work_item_index].id
    		);
    	} else {
    		work_item_index = 0;
    		enum_size = enum_limit;
    		strcpy(mod_select_clause, select_clause);
    	}
    	wi = &((*work_items).at(work_item_index));
    	
        WU_RESULT& wu_result = ssp->wu_results[i];
        switch (wu_result.state) {
        case WR_STATE_PRESENT:
       	  	if (purge_stale_time && wu_result.time_added_to_shared_memory < (time(0) - purge_stale_time)) {
    			wu_result.state = WR_STATE_EMPTY;
				log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
                    "remove result [RESULT#%d] from slot %d because it is stale\n",
                    wu_result.resultid, i);
            } else {
            	break;
        	}
        case WR_STATE_EMPTY:
            found = find_work_item(
                wi, restarted_enum[work_item_index], ncollisions,
                work_item_index, enum_size, mod_select_clause
            );
            if (found) {
                log_messages.printf(
                    SCHED_MSG_LOG::MSG_NORMAL,
                    "adding result [RESULT#%d] in slot %d\n",
                    wi->res_id, i
                );
                wu_result.resultid = wi->res_id;
                wu_result.result_priority = wi->res_priority;
                wu_result.workunit = wi->wu;
                wu_result.state = WR_STATE_PRESENT;
                // If the workunit has already been allocated to a certain
                // OS then it should be assigned quickly,
                // so we set its infeasible_count to 1
                //
                if (wi->wu.hr_class > 0) {
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

        scan_work_array(&work_items, nadditions, ncollisions);

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
            sprintf(select_clause, "and r1.id %% %d = %d ", n, j);
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

    feeder_loop();
}

const char *BOINC_RCSID_57c87aa242 = "$Id$";
