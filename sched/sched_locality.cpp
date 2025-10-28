// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// Locality scheduling: assign jobs to clients based on the data files
// the client already has.
//
// Currently this is specific to Einstein@home and is not generally usable.
// There's a generic but more limited version, "limited locality scheduling":
// https://github.com/BOINC/boinc/wiki/LocalityScheduling

#include "config.h"

#include <algorithm>
#include <climits>
#include <ctime>
#include <vector>

#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <glob.h>
#include <sys/stat.h>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"

#include "sched_check.h"
#include "sched_config.h"
#include "sched_locality.h"
#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_shmem.h"
#include "sched_types.h"
#include "sched_util.h"
#include "sched_version.h"

#define VERBOSE_DEBUG

#define EINSTEIN_AT_HOME

// get filename from result name
//

static int extract_filename(char* in, char* out, int len) {
    strlcpy(out, in, len);
    char* p = strstr(out, "__");
    if (!p) return -1;
    *p = 0;
    return 0;
}

// returns zero if there is a file we can delete.
//
int delete_file_from_host() {

#ifdef EINSTEIN_AT_HOME
    // append the list of deletion candidates to the file list
    int ndelete_candidates = (int)g_request->file_delete_candidates.size();
    for (int j=0; j<ndelete_candidates; j++) {
        FILE_INFO& fi = g_request->file_delete_candidates[j];
        g_request->file_infos.push_back(fi);
    }
    g_request->file_delete_candidates.clear();
#endif

    int nfiles = (int)g_request->file_infos.size();
    char buf[1024];
    if (!nfiles) {

        double maxdisk = max_allowable_disk();

        log_messages.printf(MSG_CRITICAL,
            "[HOST#%lu]: no disk space but no files we can delete!\n",
            g_reply->host.id
        );

        if (maxdisk > 0) {
            sprintf(buf,
                "Not enough disk space (only %.1f MB free for BOINC). ",
                maxdisk/1.e6
            );
        } else {
            sprintf(buf,
                "No disk space (YOU must free %.1f MB before BOINC gets space). ",
                -1*maxdisk/1.e6
            );
        }

        if (g_reply->disk_limits.max_used != 0.0) {
            strcat(buf, "Review preferences for maximum disk space used.");
        } else if (g_reply->disk_limits.max_frac != 0.0) {
            strcat(buf, "Review preferences for maximum disk percentage used.");
        } else if (g_reply->disk_limits.min_free != 0.0) {
            strcat(buf, "Review preferences for minimum disk free space allowed.");
        }
        g_reply->insert_message(buf, "notice");
        g_reply->set_delay(DELAY_DISK_SPACE);
        return 1;
    }

    // pick a data file to delete.
    // Do this deterministically so that we always tell host
    // to delete the same file.
    // But to prevent all hosts from removing 'the same' file,
    // choose a file which depends upon the hostid.
    //
    // Assumption is that if nothing has changed on the host,
    // the order in which it reports files is fixed.
    // If this is false, we need to sort files into order by name!
    //
    int j = g_reply->host.id % nfiles;
    FILE_INFO& fi = g_request->file_infos[j];
    g_reply->file_deletes.push_back(fi);
    if (config.debug_locality) {
        log_messages.printf(MSG_NORMAL,
            "[locality] [HOST#%lu]: delete file %s (make space)\n", g_reply->host.id, fi.name
        );
    }

    // give host 4 hours to nuke the file and come back.
    // This might in general be too soon, since host needs to complete any work
    // that depends upon this file, before it will be removed by core client.
    //
    sprintf(buf, "BOINC will delete file %s when no longer needed", fi.name);
    g_reply->insert_message(buf, "low");
    g_reply->set_delay(DELAY_DELETE_FILE);
    return 0;
}

// returns true if the host already has the file, or if the file is
// included with a previous result being sent to this host.
//
bool host_has_file(char *filename, bool skip_last_wu) {
    int i, uplim;
    bool has_file=false;

    // loop over files already on host to see if host already has the
    // file
    //
    for (i=0; i<(int)g_request->file_infos.size(); i++) {
        FILE_INFO& fi = g_request->file_infos[i];
        if (!strcmp(filename, fi.name)) {
            has_file=true;
            break;
        }
    }

    if (has_file) {
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] [HOST#%lu] Already has file %s\n", g_reply->host.id, filename
            );
        }
        return true;
    }

    // loop over files being sent to host to see if this file has
    // already been counted.
    //
    uplim=(int)g_reply->wus.size();
    if (skip_last_wu) {
        uplim--;
    }

    for (i=0; i<uplim; i++) {
        char wu_filename[256];

        if (extract_filename(g_reply->wus[i].name, wu_filename, sizeof(wu_filename))) {
            // work unit does not appear to contain a file name
            continue;
        }

        if (!strcmp(filename, wu_filename)) {
            // work unit is based on the file that we are looking for
            has_file=true;
            break;
        }
    }

    if (has_file) {
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] [HOST#%lu] file %s already in scheduler reply(%d)\n", g_reply->host.id, filename, i
            );
        }
        return true;
    }

    return false;
}

// If using locality scheduling, there are probably many result that
// use same file, so decrement available space ONLY if the host
// doesn't yet have this file. Note: this gets the file size from the
// download dir.
//
// Return value 0 means that this routine was successful in adjusting
// the available disk space in the work request.  Return value <0
// means that it was not successful, and that something went wrong.
// Return values >0 mean that the host does not contain the file, and
// that no previously assigned work includes the file, and so the disk
// space in the work request should be adjusted by the calling
// routine, in the same way as if there was no scheduling locality.
//
int decrement_disk_space_locality( WORKUNIT& wu) {
    char filename[256], path[MAXPATHLEN];
    int filesize;
    struct stat buf;

    // get filename from WU name
    //
    if (extract_filename(wu.name, filename, sizeof(filename))) {
        log_messages.printf(MSG_CRITICAL,
            "No filename found in [WU#%lu %s]\n", wu.id, wu.name
        );
        return -1;
    }

    // when checking to see if the host has the file, we need to
    // ignore the last WU included at the end of the reply, since it
    // corresponds to the one that we are (possibly) going to send!
    // So make a copy and pop the current WU off the end.

    if (!host_has_file(filename, true))
        return 1;

    // If we are here, then the host ALREADY has the file, or its size
    // has already been accounted for in a previous WU.  In this case,
    // don't count the file size again in computing the disk
    // requirements of this request.

    // Get path to file, and determine its size
    dir_hier_path(
        filename, config.download_dir, config.uldl_dir_fanout, path, false
    );
    if (stat(path, &buf)) {
        log_messages.printf(MSG_CRITICAL,
            "Unable to find file %s at path %s\n", filename, path
        );
        return -1;
    }

    filesize=buf.st_size;

    if (filesize<wu.rsc_disk_bound) {
        g_wreq->disk_available -= (wu.rsc_disk_bound-filesize);
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] [HOST#%lu] reducing disk needed for WU by %u bytes (length of %s)\n",
                g_reply->host.id, filesize, filename
            );
        }
        return 0;
    }

    log_messages.printf(MSG_CRITICAL,
        "File %s size %u bytes > wu.rsc_disk_bound for WU#%lu (%s)\n",
        path, filesize, wu.id, wu.name
    );
    return -1;
}

// Try to send the client this result
// This can fail because:
// - result needs more disk/mem/speed than host has
// - already sent a result for this WU
// - no app_version available
//
static int possibly_send_result(SCHED_DB_RESULT& result) {
    DB_WORKUNIT wu;
    SCHED_DB_RESULT result2;
    int retval;
    long count;
    char buf[256];
    BEST_APP_VERSION* bavp;

    g_wreq->no_jobs_available = false;

    retval = wu.lookup_id(result.workunitid);
    if (retval) return ERR_DB_NOT_FOUND;

    // This doesn't take into account g_wreq->allow_non_preferred_apps,
    // however Einstein@Home, which is the only project that currently uses
    // this locality scheduler, doesn't support the respective project-specific
    // preference setting
    //
    if (app_not_selected(wu.appid)) return ERR_NO_APP_VERSION;

    bavp = get_app_version(wu, true, false);

    if (!config.locality_scheduler_fraction && !bavp && is_anonymous(g_request->platforms.list[0])) {
        char help_msg_buf[512];
        sprintf(help_msg_buf,
            "To get more %s work, finish current work, stop BOINC, remove app_info.xml file, and restart.",
            config.long_name
        );
        g_reply->insert_message(help_msg_buf, "notice");
        g_reply->set_delay(DELAY_ANONYMOUS);
    }

    if (!bavp) return ERR_NO_APP_VERSION;

    APP* app = ssp->lookup_app(wu.appid);
    retval = wu_is_infeasible_fast(
        wu, result.server_state, result.report_deadline, result.priority,
        *app, *bavp
    );
    if (retval) return retval;

    if (config.one_result_per_user_per_wu) {
        sprintf(buf, "where userid=%lu and workunitid=%lu", g_reply->user.id, wu.id);
        retval = result2.count(count, buf);
        if (retval) return ERR_DB_NOT_FOUND;
        if (count > 0) return ERR_WU_USER_RULE;
    }

    HOST_USAGE hu;
    BUDA_VARIANT *bvp = NULL;
    if (is_buda(wu)) {
        if (!choose_buda_variant(wu, -1, &bvp, hu)) {
            return -1;
        }
    } else {
        hu = bavp->host_usage;
    }
    return add_result_to_reply(result, wu, bavp, hu, bvp, false);
}

// Retrieves and returns a trigger instance identified by the given
// fileset name.
//
static bool retrieve_single_trigger_by_fileset_name(char *fileset_name, DB_SCHED_TRIGGER& trigger) {
    int retval = 0;

    // retrieve trigger
    retval = trigger.select_unique_by_fileset_name(fileset_name);
    if(!retval) {
        if (config.debug_locality) {
            log_messages.printf(MSG_DEBUG,
                    "[locality] trigger %s state after retrieval: nw=%i wa=%i nwa=%i wsr=%i\n",
                    fileset_name,
                    trigger.need_work,
                    trigger.work_available,
                    trigger.no_work_available,
                    trigger.working_set_removal
            );
        }

        // successful retrieval
        return true;
    }
    else if(retval == ERR_DB_NOT_FOUND) {
        log_messages.printf(MSG_NORMAL,
                "[locality] trigger retrieval for filename %s returned empty set\n", fileset_name
        );
        return false;
    }
    else {
        log_messages.printf(MSG_CRITICAL,
            "[locality] trigger retrieval for filename %s failed with error %s\n",
            fileset_name, boincerror(retval)
        );
        return false;
    }
}

// Ask the WU generator to make more WUs for this file.
// Returns nonzero if can't make more work.
// Returns zero if it *might* have made more work
// (no way to be sure if it succeeded).
//
int make_more_work_for_file(char* filename) {
    int retval = 0;
    DB_SCHED_TRIGGER trigger;


    if (!retrieve_single_trigger_by_fileset_name(filename, trigger)) {
        // trigger retrieval failed (message logged by previous method)
        return -1;
    }

    // Check if there's remaining work for this WU
    if (trigger.no_work_available) {
        // Give up trying to interact with the WU generator.
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] work generator says no work remaining for trigger %s\n", filename
            );
        }
        return -1;
    }

//    // FIXME: should we reset these? The old code didn't do any consistency checks...
//    trigger.work_available = false;
//    trigger.no_work_available = false;
//    trigger.working_set_removal = false;

    // set trigger state to need_work as a way of indicating that we need work
    // for this fileset. If this operation fails, don't worry or tarry!
    retval = trigger.update_single_state(DB_SCHED_TRIGGER::state_need_work, true);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "unable to set need_work state for trigger %s (error: %s)\n",
            filename, boincerror(retval)
        );
        return -1;
    }

    return 0;
}

// Get a randomly-chosen filename in the working set.
//
// We store a static list to prevent duplicate filename returns
// and to cut down on DB queries
//
//
std::vector<std::string> filenamelist;
int list_type = 0; // 0: none, 1: slowhost, 2: fasthost

static void build_working_set_namelist(bool slowhost) {
    int retval = 0;
    unsigned int i;
    const char *pattern = ".*";
    bool use_pattern = false;
    const char *errtype = "unrecognized error";
    const char *hosttype = "fasthost";
    DB_FILESET_SCHED_TRIGGER_ITEM_SET filesets;

#ifdef EINSTEIN_AT_HOME
    if (slowhost) {
        hosttype = "slowhost";
        pattern = ".*_0[0-3].*";
        use_pattern = true;
    }
#endif

    if(use_pattern) {
        retval = filesets.select_by_name_state(pattern, true, DB_SCHED_TRIGGER::state_work_available, true);
    }
    else {
        retval = filesets.select_by_name_state(NULL, false, DB_SCHED_TRIGGER::state_work_available, true);
    }

    if (retval == ERR_DB_NOT_FOUND) {
        errtype = "empty directory";
    }
    else if(!retval) {
        for (i=0; i<filesets.items.size(); i++) {
            filenamelist.push_back(filesets.items[i].fileset.name);
        }
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] build_working_set_namelist(%s): pattern %s has %d matches\n",
                hosttype, pattern, (int)filesets.items.size()
            );
        }
        return;
    }

    log_messages.printf(MSG_CRITICAL,
        "build_working_set_namelist(%s): pattern %s not found (%s)\n", hosttype, pattern, errtype
    );

    return;
}

static int get_working_set_filename(char *filename, int len, bool slowhost) {

    const char* errtype = NULL;

    if (!list_type) {
        build_working_set_namelist(slowhost);
        list_type = slowhost ? 1 : 2;
    }

    if (list_type == 1 && filenamelist.size() == 0) {
        slowhost = false;
        build_working_set_namelist(slowhost);
        list_type = 2;
    }

    if (list_type == 1 && !slowhost) {
        filenamelist.clear();
        build_working_set_namelist(slowhost);
        list_type = 2;
    }

    const char *hosttype = slowhost ? "slowhost" : "fasthost" ;

    if (filenamelist.size() == 0) {
        errtype = "file list empty";
    } else {

        // take out a random file and remove it from the vector
        //
        int random_file_num = rand() % filenamelist.size();
        std::string thisname = filenamelist[random_file_num];
        filenamelist[random_file_num] = filenamelist.back();
        filenamelist.pop_back();

        // final check
        if (thisname.length() < 1) {
            errtype = "zero length filename";
        } else {
            strlcpy(filename, thisname.c_str(), len);
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] get_working_set_filename(%s): returning %s\n",
                    hosttype, filename
                );
            }
            return 0;
        }
    }

    log_messages.printf(MSG_CRITICAL,
        "get_working_set_filename(%s): pattern not found (%s)\n", hosttype, errtype
    );
    return 1;
}


static void flag_for_possible_removal(char* fileset_name) {
    int retval = 0;
    DB_SCHED_TRIGGER trigger;

    if (!retrieve_single_trigger_by_fileset_name(fileset_name, trigger)) {
        // trigger retrieval failed (message logged by previous method)
        return;
    }

//    // FIXME: should we reset these? The old code didn't do any consistency checks...
//    trigger.need_work = false;
//    trigger.work_available = false;
//    trigger.no_work_available = false;

    // set trigger state to working_set_removal
    retval = trigger.update_single_state(DB_SCHED_TRIGGER::state_working_set_removal, true);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "unable to set working_set_removal state for trigger %s (error: %s)\n",
            fileset_name, boincerror(retval)
        );
    }
}

// The client has (or will soon have) the given file.
// Try to send it results that use that file.
// If don't get any the first time,
// trigger the work generator, then try again.
//
static int send_results_for_file(
    char* filename,
    int& nsent,
    bool /*in_working_set*/
) {
    SCHED_DB_RESULT result, prev_result;
    char buf[256], query[1024];
    int i, retval_max, retval_lookup, sleep_made_no_work=0;
    DB_ID_TYPE maxid;

    nsent = 0;

    if (!work_needed(true)) {
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] send_results_for_file(): No work needed\n"
            );
        }
        return 0;
    } else {
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] send_results_for_file(%s)\n",
                filename
            );
        }
    }


    // find largest ID of results already sent to this user for this
    // file, if any.  Any result that is sent will have userid field
    // set, so unsent results can not be returned by this query.
    //
#ifdef USE_REGEXP
    char pattern[256], escaped_pattern[256];
    sprintf(pattern, "%s__", filename);
    escape_mysql_like_pattern(pattern, escaped_pattern);
    sprintf(buf, "where userid=%lu and name like binary '%s%%'",
        g_reply->user.id, escaped_pattern
    );
#else
    sprintf(buf, "where userid=%lu and name>binary '%s__' and name<binary '%s__~'",
        g_reply->user.id, filename, filename
    );
#endif
    retval_max = result.max_id(maxid, buf);
    if (retval_max) {
        prev_result.id = 0;
    } else {
        retval_lookup = prev_result.lookup_id(maxid);
        if (retval_lookup) return ERR_DB_NOT_FOUND;
    }

    for (i=0; i<100; i++) {     // avoid infinite loop
        int query_retval;

        if (!work_needed(true)) break;

        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] in_send_results_for_file(%s, %d) prev_result.id=%lu\n",
                filename, i, prev_result.id
            );
        }

        // find unsent result with next larger ID than previous largest ID
        //
        if (config.one_result_per_user_per_wu && prev_result.id) {

            // if one result per user per WU, insist on different WUID too
            //
#ifdef USE_REGEXP
            sprintf(query,
                "INNER JOIN (SELECT id FROM result WHERE name like binary '%s%%' and id>%d and workunitid<>%d and server_state=%d order by id limit 1) AS single USING (id) ",
                escaped_pattern, prev_result.id, prev_result.workunitid, RESULT_SERVER_STATE_UNSENT
            );
#else
            sprintf(query,
                "INNER JOIN (SELECT id FROM result WHERE name>binary '%s__' and name<binary '%s__~' and id>%lu and workunitid<>%lu and server_state=%d order by id limit 1) AS single USING (id) ",
                filename, filename, prev_result.id, prev_result.workunitid, RESULT_SERVER_STATE_UNSENT
            );
#endif
        } else {
#ifdef USE_REGEXP
            sprintf(query,
                "INNER JOIN (SELECT id FROM result WHERE name like binary '%s%%' and id>%d and server_state=%d order by id limit 1) AS single USING (id) ",
                escaped_pattern, prev_result.id, RESULT_SERVER_STATE_UNSENT
            );
#else
            sprintf(query,
                "INNER JOIN (SELECT id FROM result WHERE name>binary '%s__' and name<binary '%s__~' and id>%lu and server_state=%d order by id limit 1) AS single USING (id) ",
                filename, filename, prev_result.id, RESULT_SERVER_STATE_UNSENT
            );
#endif
        }

        // Use a transaction so that if we get a result,
        // someone else doesn't send it before we do
        //
        boinc_db.start_transaction();

        query_retval = result.lookup(query);

        if (query_retval) {
            int make_work_retval;

            // no unsent results are available for this file
            //
            boinc_db.commit_transaction();

            // see if no more work remains to be made for this file,
            // or if an attempt to make more work fails.
            //
            make_work_retval=make_more_work_for_file(filename);
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] make_more_work_for_file(%s, %d)=%d\n", filename, i, make_work_retval
                );
            }

            if (make_work_retval) {
                // can't make any more work for this file

                if (config.one_result_per_user_per_wu) {

                    // do an EXPENSIVE db query
#ifdef USE_REGEXP
                    sprintf(query,
                        "INNER JOIN (SELECT id FROM result WHERE server_state=%d and name like binary '%s%%' limit 1) AS single USING (id)",
                        RESULT_SERVER_STATE_UNSENT, escaped_pattern
                    );
#else
                    sprintf(query,
                        "INNER JOIN (SELECT id FROM result WHERE server_state=%d and name>binary '%s__' and name<binary '%s__~' limit 1) AS single USING (id)",
                        RESULT_SERVER_STATE_UNSENT, filename, filename
                    );
#endif

                    // re-using result -- do I need to clear it?
                    if (!result.lookup(query)) {
                        // some results remain -- but they are not suitable
                        // for us because they must be for a WU that we have
                        // already looked at.
                        break;
                    }
                } // config.one_result_per_user_per_wu

                // arrive here if and only if there exist no further
                // unsent results for this file.
                flag_for_possible_removal(filename);
                if (config.debug_locality) {
                    log_messages.printf(MSG_NORMAL,
                        "[locality] No remaining work for file %s (%d), flagging for removal\n", filename, i
                    );
                }
                break;
            } // make_work_retval

            // If the user has not configured us to wait and try
            // again, or we have already tried to find work for this
            // file, we are finished.
            //
            if (!config.locality_scheduling_wait_period || sleep_made_no_work) {
                break;
            }

            // wait a bit and try again to find a suitable unsent result
            sleep(config.locality_scheduling_wait_period);
            sleep_made_no_work=1;

        } // query_retval
        else {
            int retval_send;

            // we found an unsent result, so try sending it.
            // This *should* always work.
            //
            retval_send = possibly_send_result(result);
            boinc_db.commit_transaction();

            // if no app version or not enough resources, give up completely
            //
            if (retval_send == ERR_NO_APP_VERSION || retval_send==ERR_INSUFFICIENT_RESOURCE) return retval_send;

            // if we couldn't send it for other reason, something's wacky;
            // print a message, but keep on looking.

            // David, this is NOT wacky.  Consider the following
            // scenario: WU A has result 1 and WU B has result 2.
            // These are both sent to a host.  Some time later, result
            // 1 fails and the transitioner creates a new result,
            // result 3 for WU A.  Then the host requests a new
            // result.  The maximum result already sent to the host is
            // 2.  The next unsent result (sorted by ID) is #3.  But
            // since it is for WU A, and since the host has already
            // gotten a result for WU A, it's infeasible.  So I think
            // this is only wacky if !one_wu_per_result_per_host.
            if (!retval_send) {
                nsent++;
                sleep_made_no_work=0;
            } else if (!config.one_result_per_user_per_wu) {
                log_messages.printf(MSG_CRITICAL,
                    "Database inconsistency?  possibly_send_result(%d) failed for [RESULT#%lu], returning %d\n",
                    i, result.id, retval_send
                );
            // If another scheduler instance 'snatched' the result
            // from under our noses, then possibly_send_result()
            // will return ERR_DB_NOT_FOUND
            //
            } else if (retval_send != ERR_DB_NOT_FOUND) {
                if (config.debug_locality) {
                    log_messages.printf(MSG_NORMAL,
                        "[locality] possibly_send_result [RESULT#%lu]: %s\n",
                        result.id, boincerror(retval_send)
                    );
                }
            }

            prev_result = result;

        } // query_retval

    } // loop over 0<i<100
    return 0;
}


// Find a file with work, and send.
// This is guaranteed to send work if ANY is available for this user.
// However, it ignores the working set,
// and should be done only if we fail to send work from the working set.
//
// logic:
// min_resultname = ""
// loop
//    R = first unsent result where filename>min_resultname order by filename
//        // order by filename implies order by ID
//    send_results_for_file(R.filename)
//        //  this skips disqualified results
//    min_resultname = R.filename;
//
static int send_new_file_work_deterministic_seeded(
    int& nsent, const char *start_f, const char *end_f
) {
    SCHED_DB_RESULT result;
    char filename[256], min_resultname[256], query[1024];
    int retval;

    if (config.debug_locality) {
        log_messages.printf(MSG_NORMAL,
            "[locality] send_new_file_work_deterministic_seeded() start=%s end=%s\n",
            start_f, end_f?end_f:"+Inf"
        );
    }

    safe_strcpy(min_resultname, start_f);
    while (1) {

        // are we done with the search yet?
        if (end_f && strcmp(min_resultname, end_f)>=0)
          break;

#if 0
        // an alternative here is to add ANOTHER index on name, server_state
        // to the result table.
        sprintf(query,
            "INNER JOIN (SELECT id FROM result WHERE server_state=%d and name>'%s' order by name limit 1) AS single USING (id)",
            RESULT_SERVER_STATE_UNSENT, min_resultname
        );
#endif

        sprintf(query,
            "INNER JOIN (SELECT id FROM result WHERE name>'%s' order by name limit 1) AS single USING (id)",
            min_resultname
        );

        retval = result.lookup(query);
        if (retval) break; // no more unsent results or at the end of the filenames, return -1
        retval = extract_filename(result.name, filename, sizeof(filename));
        if (retval) return retval; // not locality scheduled, now what???

        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] send_new_file_work_deterministic will try filename %s\n", filename
            );
        }

        retval = send_results_for_file(filename, nsent, false);

        if (retval==ERR_NO_APP_VERSION || retval==ERR_INSUFFICIENT_RESOURCE) return retval;

        if (nsent>0 || !work_needed(true)) break;
        // construct a name which is lexically greater than the name of any result
        // which uses this file.
        sprintf(min_resultname, "%s__~", filename);
    }
    return 0;
}


static bool is_host_slow() {
#if 0
    // 0.0013 defines about the slowest 20% of E@H hosts.
    // should make this a config parameter in the future,
    // if this idea works.
    //

    static int speed_not_printed = 1;
    double hostspeed = g_request->host.claimed_credit_per_cpu_sec;

    if (speed_not_printed) {
        speed_not_printed = 0;
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] Host speed %f\n", hostspeed
            );
        }
    }
    if (hostspeed < 0.0013) return true;
#endif
    return false;
}

// Returns 0 if this has sent additional new work.  Returns non-zero
// if it has not sent any new work.
//
static int send_new_file_work_deterministic() {
    char start_filename[256];
    int getfile_retval, nsent=0;

    // get random filename as starting point for deterministic search
    // If at this point, we have probably failed to find a suitable file
    // for a slow host, so ignore speed of host.
    if ((getfile_retval = get_working_set_filename(start_filename, sizeof(start_filename), /* is_host_slow() */ false))) {
        safe_strcpy(start_filename, "");
    }

    // start deterministic search with randomly chosen filename, go to
    // lexical maximum
    send_new_file_work_deterministic_seeded(nsent, start_filename, NULL);
    if (nsent) {
        return 0;
    }

    // continue deterministic search at lexically first possible
    // filename, continue to randomly choosen one
    if (!getfile_retval && work_needed(true)) {
        send_new_file_work_deterministic_seeded(
            nsent, "", start_filename
        );
        if (nsent) {
            return 0;
        }
    }

    return 1;
}


static int send_new_file_work_working_set() {
    char filename[256];
    int retval, nsent;

    retval = get_working_set_filename(filename, sizeof(filename), is_host_slow());
    if (retval) return retval;

    if (config.debug_locality) {
        log_messages.printf(MSG_NORMAL,
            "[locality] send_new_file_working_set will try filename %s\n", filename
        );
    }

    return send_results_for_file(filename, nsent, true);
}

// prototype
static int send_old_work(int t_min, int t_max, bool locality_work_only=false);

// The host doesn't have any files for which work is available.
// Pick new file to send.  Returns nonzero if no work is available.
//
static int send_new_file_work() {

    while (work_needed(true)) {
        int retval_sow, retval_snfwws;
        double frac=((double)rand())/(double)RAND_MAX;
        int now   = time(0);
        int end   = now - config.locality_scheduling_send_timeout/2;
        int start = end - (int)(0.5*frac*config.locality_scheduling_send_timeout);
        int retry=0;

        // send work that's been hanging around the queue for an
        // interval that which (1) starts at a random time between
        // timeout and timeout/2 ago, and (2) continues until
        // timeout/2 ago.  We might consider enclosing this in a while
        // loop and trying several times.
        //
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] send_new_file_work(): try to send old work\n"
            );
        }

        retval_sow=send_old_work(start, end);

        if (retval_sow==ERR_NO_APP_VERSION || retval_sow==ERR_INSUFFICIENT_RESOURCE) return retval_sow;


        while (work_needed(true) && retry<5) {
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] send_new_file_work(%d): try to send from working set\n", retry
                );
            }
            retry++;
            retval_snfwws=send_new_file_work_working_set();
            if (retval_snfwws==ERR_NO_APP_VERSION || retval_snfwws==ERR_INSUFFICIENT_RESOURCE) return retval_snfwws;

        }

        if (work_needed(true)) {
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] send_new_file_work(): try deterministic method\n"
                );
            }
            if (send_new_file_work_deterministic()) {
                // if no work remains at all,
                // we learn it here and return nonzero.
                //
                return 1;
            }
        }
    } // while g_reply->work_needed(true)
    return 0;
}


// DAVID, this is missing a return value!  Am I right that this will
// also eventually move 'non locality' work through and out of the
// system?
//
// Yes, Bruce, it will. BM
//
// This looks for work created in the range t_min < t < t_max.  Use
// t_min=INT_MIN if you wish to leave off the left constraint.
//
static int send_old_work(int t_min, int t_max, bool locality_work_only) {
    char buf[1024], filename[256];
    int retval, extract_retval, nsent;
    SCHED_DB_RESULT result;
    int now=time(0);

    if (!work_needed(true)) {
        return 0;
    }

    // restrict values to full hours;
    // this allows the DB to cache query results in some cases
    //
    t_max = (t_max/3600)*3600;

    boinc_db.start_transaction();

    // Note: the following queries look convoluted.
    // But apparently the simpler versions (without the inner join)
    // are a lot slower.
    //
    if (t_min != INT_MIN) {
        sprintf(buf,
#ifdef EINSTEIN_AT_HOME
            "INNER JOIN (SELECT id FROM result USE INDEX (res_create_server_state) WHERE server_state=%d and %d<create_time and create_time<%d %s limit 1) AS single USING (id)",
            RESULT_SERVER_STATE_UNSENT, t_min, t_max, locality_work_only?"and name>binary '%s__' and name<binary '%s__~'":""
#else
            "INNER JOIN (SELECT id FROM result WHERE server_state=%d and %d<create_time and create_time<%d limit 1) AS single USING (id)",
            RESULT_SERVER_STATE_UNSENT, t_min, t_max
#endif
        );
    }
    else {
        sprintf(buf,
#ifdef EINSTEIN_AT_HOME
            "INNER JOIN (SELECT id FROM result USE INDEX (res_create_server_state) WHERE server_state=%d and create_time<%d %s limit 1) AS single USING (id)",
            RESULT_SERVER_STATE_UNSENT, t_max, locality_work_only?"and name>binary '%s__' and name<binary '%s__~'":""
#else
            "INNER JOIN (SELECT id FROM result WHERE server_state=%d and create_time<%d limit 1) AS single USING (id)",
            RESULT_SERVER_STATE_UNSENT, t_max
#endif
        );
    }

    retval = result.lookup(buf);
    if (!retval) {
        retval = possibly_send_result(result);
        boinc_db.commit_transaction();
        if (!retval) {
            double age=(now-result.create_time)/3600.0;
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] send_old_work(%s) sent result created %.1f hours ago [RESULT#%lu]\n",
                    result.name, age, result.id
                );
            }
            extract_retval=extract_filename(result.name, filename, sizeof(filename));
            if (!extract_retval) {
                send_results_for_file(filename, nsent, false);
            } else {
                // David, is this right?  Is this the only place in
                // the locality scheduler that non-locality work //
                // gets done?
                if (config.debug_locality) {
                    log_messages.printf(MSG_NORMAL,
                        "[locality] Note: sent NON-LOCALITY result %s\n", result.name
                    );
                }
            }
        } else if (retval == ERR_NO_APP_VERSION || retval==ERR_INSUFFICIENT_RESOURCE) {
            // if no app version found or no resources, give up completely!
            return retval;
        }

    } else {
        boinc_db.commit_transaction();
    }

    if (retval) {
        double older=(now-t_max)/3600.0;
        if (t_min != INT_MIN) {
            double young=(now-t_min)/3600.0;
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] send_old_work() no feasible result younger than %.1f hours and older than %.1f hours\n",
                    young, older
                );
            }
        }
        else {
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] send_old_work() no feasible result older than %.1f hours\n",
                    older
                );
            }
        }
    }

    // DAVID, YOU CHANGED THIS FROM VOID TO INT.  IS THIS THE RIGHT
    // RETURN VAL?  You should probably use the return value from
    // sent_results_for_file as well.
    return retval;
}

bool file_info_order(const FILE_INFO& fi1, const FILE_INFO& fi2) {
    if (strncmp(fi1.name, fi2.name, 256) < 0) return true;
    return false;
}

bool is_sticky_file(char*fname) {
    for (unsigned int i=0; i<config.locality_scheduling_sticky_file->size(); i++) {
        if (!regexec(&((*config.locality_scheduling_sticky_file)[i]), fname, 0, NULL, 0)) {
            return true;
        }
    }
    return false;
}

bool is_workunit_file(char*fname) {
    for (unsigned int i=0; i<config.locality_scheduling_workunit_file->size(); i++) {
        if (!regexec(&((*config.locality_scheduling_workunit_file)[i]), fname, 0, NULL, 0)) {
            return true;
        }
    }
    return false;
}

void send_work_locality() {
    int i, nsent, nfiles, j;

    // seed the random number generator
    unsigned int seed=time(0)+getpid();
    srand(seed);

    // file names are used in SQL queries throughout; escape them now
    // (this breaks things if file names legitimately contain ', but they don't)
    //
    for (unsigned int k=0; k<g_request->file_infos.size(); k++) {
        FILE_INFO& fi = g_request->file_infos[k];
        escape_string(fi.name, sizeof(fi.name));
    }

#ifdef EINSTEIN_AT_HOME
    std::vector<FILE_INFO> eah_copy = g_request->file_infos;
    g_request->file_infos.clear();
    g_request->files_not_needed.clear();
    nfiles = (int) eah_copy.size();
    for (i=0; i<nfiles; i++) {
        char *fname = eah_copy[i].name;

        if (is_workunit_file(fname)) {
            // these are files that we will use for locality scheduling and
            // to search for work
            //
            g_request->file_infos.push_back(eah_copy[i]);
        } else if (is_sticky_file(fname)) {  // was if(!data_files)
            // these files MIGHT be deleted from host if we need to make
            // disk space there
            //
            g_request->file_delete_candidates.push_back(eah_copy[i]);
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] [HOST#%lu] removing file %s from file_infos list\n",
                    g_reply->host.id, fname
                );
            }
        } else {
            // these files WILL be deleted from the host
            //
            g_request->files_not_needed.push_back(eah_copy[i]);
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] [HOST#%lu] adding file %s to files_not_needed list\n",
                    g_reply->host.id, fname
                );
            }
        }
    }
#endif // EINSTEIN_AT_HOME

    nfiles = (int) g_request->file_infos.size();
    for (i=0; i<nfiles; i++)
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] [HOST#%lu] has file %s\n",
                g_reply->host.id, g_request->file_infos[i].name
            );
        }

    // send old work if there is any. send this only to hosts which have
    // high-bandwidth connections, since asking dial-up users to upload
    // (presumably large) data files is onerous.
    //
    if (config.locality_scheduling_send_timeout && g_request->host.n_bwdown>100000) {
        int until=time(0)-config.locality_scheduling_send_timeout;
        int retval_sow=send_old_work(INT_MIN, until);
        if (retval_sow) {
            log_messages.printf(MSG_NORMAL,
                "[locality] send_old_work() returned %d\n", retval_sow
            );
        }
        if (!work_needed(true)) return;
    }

    // Look for work in order of increasing file name, or randomly?
    //
    if (config.locality_scheduling_sorted_order) {
        sort(g_request->file_infos.begin(), g_request->file_infos.end(), file_info_order);
        j = 0;
    } else {
        if (!nfiles) nfiles = 1;
        j = rand()%nfiles;
    }

    // send work for existing files
    //
    for (i=0; i<(int)g_request->file_infos.size(); i++) {
        int k = (i+j)%nfiles;
        int retval_srff;

        if (!work_needed(true)) break;
        FILE_INFO& fi = g_request->file_infos[k];
        retval_srff = send_results_for_file(
            fi.name, nsent, false
        );

        if (retval_srff==ERR_NO_APP_VERSION || retval_srff==ERR_INSUFFICIENT_RESOURCE) return;

        // if we couldn't send any work for this file, and we STILL need work,
        // then it must be that there was no additional work remaining for this
        // file which is feasible for this host.  In this case, delete the file.
        // If the work was not sent for other (dynamic) reason such as insufficient
        // cpu, then DON'T delete the file.
        //
        if (nsent == 0 && work_needed(true) && config.file_deletion_strategy == 1) {
            g_reply->file_deletes.push_back(fi);
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[locality] [HOST#%lu]: delete file %s (not needed)\n",
                    g_reply->host.id, fi.name
                );
            }
#ifdef EINSTEIN_AT_HOME
            // For name matching patterns h1_
            // generate corresponding l1_ patterns and delete these also
            //
            if (   /* files like h1_0340.30_S6GC1 */
                   (   strlen(fi.name) == 16 &&
                       !strncmp("h1_", fi.name, 3) &&
                       !strncmp("_S6GC1", fi.name + 10, 6)
                   ) ||
                   /* files like h1_0000.00_S6Directed */
                   (   strlen(fi.name) == 21 &&
                       !strncmp("h1_", fi.name, 3) &&
                       !strncmp("_S6Directed", fi.name + 10, 11)
                   ) ||
                   /* files like h1_0000.00_S6Direct */
                   (   strlen(fi.name) == 19 &&
                       !strncmp("h1_", fi.name, 3) &&
                       !strncmp("_S6Direct", fi.name + 10, 9)
                   )
               ) {
                FILE_INFO fil;
                fil=fi;
                fil.name[0]='l';
                g_reply->file_deletes.push_back(fil);
                if (config.debug_locality) {
                    log_messages.printf(MSG_NORMAL,
                        "[locality] [HOST#%lu]: delete file %s (accompanies %s)\n",
                        g_reply->host.id, fil.name, fi.name
                    );
                }
            } else if ( /* for files like h1_XXXX.XX_S5R4 */
                   (   strlen(fi.name) == 15 &&
                       !strncmp("h1_", fi.name, 3) &&
                       !strncmp("_S5R4", fi.name + 10, 5)
                   )
               ) {
                FILE_INFO fil4,fil7,fih7;
                fil4=fi;
                fil4.name[0]='l';
                fil7=fil4;
                fil7.name[14]='7';
                fih7=fi;
                fih7.name[14]='7';
                g_reply->file_deletes.push_back(fil4);
                g_reply->file_deletes.push_back(fil7);
                g_reply->file_deletes.push_back(fih7);
                if (config.debug_locality) {
                    log_messages.printf(MSG_NORMAL,
                        "[locality] [HOST#%lu]: delete files %s,%s,%s (accompanies %s)\n",
                        g_reply->host.id, fil4.name,fil7.name,fih7.name, fi.name
                    );
                }
            }
#endif
        } // nsent==0
    } // loop over files already on the host

    // send new files if needed
    //
    if (work_needed(true)) {
        send_new_file_work();
    }
}

// send instructions to delete useless files
//
void send_file_deletes() {
    int num_useless = g_request->files_not_needed.size();
    int i;
    for (i=0; i<num_useless; i++) {
        char buf[1024];
        FILE_INFO& fi = g_request->files_not_needed[i];
        g_reply->file_deletes.push_back(fi);
        if (config.debug_locality) {
            log_messages.printf(MSG_NORMAL,
                "[locality] [HOST#%lu]: delete file %s (not needed)\n",
                g_reply->host.id, fi.name
            );
        }
        sprintf(buf, "BOINC will delete file %s (no longer needed)", fi.name);
        g_reply->insert_message(buf, "low");
     }

    // if we got no work, and we have no file space, delete some files
    //
    if (g_reply->results.size()==0 && (g_reply->wreq.disk.insufficient || g_reply->wreq.disk_available<0)) {
        // try to delete a file to make more space.
        // Also give some hints to the user about what's going wrong
        // (lack of disk space).
        //
        delete_file_from_host();
    }

    if (g_reply->results.size()==0 && g_reply->hostid && g_request->work_req_seconds>1.0) {
        debug_sched("debug_sched");
    } else if (max_allowable_disk()<0 || (g_reply->wreq.disk.insufficient || g_reply->wreq.disk_available<0)) {
        debug_sched("debug_sched");
    }
}

// Explanation of the logic of this scheduler:

// (1) If there is an (one) unsent result which is older than
// (1) config.locality_scheduling_send_timeout (7 days) and is
// (1) feasible for the host, and host has a fast network
// (1) connection (>100kb/s) then send it.

// (2) If we did send a result in the previous step, then send any
// (2) additional results that are feasible for the same input file.
// (2) Note that step 1 above is the ONLY place in the code where we
// (2) can send a result that is NOT of the locality name-type
// (2) FILENAME__other_stuff.

// (3) If additional results are needed, step through input files on
// (3) the host.  For each, if there are results that are feasible for
// (3) the host, send them.  If there are no results that are feasible
// (3) for the host, delete the input file from the host.

// (4) If additional results are needed, send the oldest result
// (4) created between times A and B, where
// (4) A=random time between locality_scheduling_send timeout and
// (4) locality_timeout/2 in the past, and B=locality_timeout/2 in
// (4) the past.

// (5) If we did send a result in the previous step, then send any
// (5) additional results that are feasible for the same input file.

// (6) If additional results are needed, select an input file name at
// (6) random from the current input file working set advertised by
// (6) the WU generator.  If there are results for this input file
// (6) that are feasible for this host, send them.  If no results
// (6) were found for this file, then repeat this step 6 another nine
// (6) times.

// (7) If additional results are needed, carry out an expensive,
// (7) deterministic search for ANY results that are feasible for the
// (7) host.  This search starts from a random filename advertised by
// (7) the WU generator, but continues cyclicly to cover ALL results
// (7) for ALL files. If a feasible result is found, send it.  Then
// (7) send any additional results that use the same input file.  If
// (7) there are no feasible results for the host, we are finished:
// (7) exit.

// (8) If additional results are needed, return to step 4 above.
