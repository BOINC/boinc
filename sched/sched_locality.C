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

// Locality scheduling: see doc/sched_locality.php

#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <glob.h>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"

#include "main.h"
#include "server_types.h"
#include "sched_shmem.h"
#include "sched_send.h"
#include "sched_msgs.h"
#include "sched_locality.h"
#include "sched_util.h"

#define VERBOSE_DEBUG

// Find the app and app_version for the client's platform.
//
static int get_app_version(
    WORKUNIT& wu, APP* &app, APP_VERSION* &avp,
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    bool found;
    if (anonymous(platform)) {
        app = ss.lookup_app(wu.appid);
        found = sreq.has_version(*app);
        if (!found) {
            return ERR_NO_APP_VERSION;
        }
        avp = NULL;
    } else {
        found = find_app_version(reply.wreq, wu, platform, ss, app, avp);
        if (!found) {
            return ERR_NO_APP_VERSION;
        }

        // see if the core client is too old.
        //
        if (!app_core_compatible(reply.wreq, *avp)) {
            return ERR_NO_APP_VERSION;
        }
    }
    return 0;
}

// Try to send the client this result
// This can fail because:
// - result needs more disk/mem/speed than host has
// - already sent a result for this WU
// - no app_version available
//
static int possibly_send_result(
    DB_RESULT& result,
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    DB_WORKUNIT wu;
    DB_RESULT result2;
    int retval, count;
    char buf[256];
    APP* app;
    APP_VERSION* avp;

    retval = wu.lookup_id(result.workunitid);
    if (retval) return retval;

    if (wu_is_infeasible(wu, sreq, reply)) {
        return ERR_INSUFFICIENT_RESOURCE;
    }

    if (config.one_result_per_user_per_wu) {
        sprintf(buf, "where userid=%d and workunitid=%d", reply.user.id, wu.id);
        retval = result2.count(count, buf);
        if (retval) return retval;
        if (count > 0) return ERR_WU_USER_RULE;
    }

    retval = get_app_version(
        wu, app, avp, sreq, reply, platform, ss
    );
    if (retval) return retval;

    return add_result_to_reply(result, wu, sreq, reply, platform, app, avp);
}

// returns true if the work generator can not make more work for this
// file, false if it can.
//
static bool work_generation_over(char *filename) {
    char fullpath[512];
    sprintf(fullpath, "../locality_scheduling/no_work_available/%s", filename);
    return boinc_file_exists(fullpath);
}

// returns zero on success, nonzero if didn't touch file
//
int touch_file(char *path) {
    FILE *fp;

    if (boinc_file_exists(path)) {
        return 0;
    }

    if ((fp=fopen(path, "w"))) {
        fclose(fp);
        return 0;
    }

    return -1;
}


// Ask the WU generator to make more WUs for this file.
// Returns nonzero if can't make more work.
// Returns zero if it *might* have made more work
// (no way to be sure if it suceeded).
//
int make_more_work_for_file(char* filename) {
    char fullpath[512];

    if (work_generation_over(filename)) {
        // since we found this file, it means that no work remains for this WU.
        // So give up trying to interact with the WU generator.
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG,
            "work generator says no work remaining for file %s\n", filename
        );
        return -1;
    }
        
    // open and touch a file in the need_work/
    // directory as a way of indicating that we need work for this file.
    // If this operation fails, don't worry or tarry!
    //
    sprintf(fullpath, "../locality_scheduling/need_work/%s", filename);
    if (touch_file(fullpath)) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "unable to touch %s\n", fullpath
        );
        return -1;
    }

    log_messages.printf(
        SCHED_MSG_LOG::DEBUG,
        "touched %s: need work for file %s\n", fullpath, filename
    );
    return 0;
}

// Get a randomly-chosen filename in the working set.
//
static int get_working_set_filename(char *filename) {
    glob_t globbuf;
    int retglob, random_file;
    char *last_slash;
    const char *pattern = "../locality_scheduling/work_available/*";

    retglob=glob(pattern, GLOB_ERR|GLOB_NOSORT|GLOB_NOCHECK, NULL, &globbuf);
    
    if (retglob || !globbuf.gl_pathc) {
        // directory did not exist or is not readable
        goto error_exit;
    }

    if (globbuf.gl_pathc==1 && !strcmp(pattern, globbuf.gl_pathv[0])) {
        // directory was empty
        goto error_exit;
    }

    // Choose a file at random.
    random_file = rand() % globbuf.gl_pathc;

    // remove trailing slash from randomly-selected file path
    last_slash = rindex(globbuf.gl_pathv[random_file], '/');
    if (!last_slash || *last_slash=='\0' || *(++last_slash)=='\0') {
        // no trailing slash found, or it's a directory name
        goto error_exit;
    }

    strcpy(filename, last_slash);
    globfree(&globbuf);
    
    log_messages.printf(SCHED_MSG_LOG::DEBUG,
        "get_working_set_filename(): returning %s\n", filename
    );

    return 0;

error_exit:
    log_messages.printf(SCHED_MSG_LOG::CRITICAL,
        "get_working_set_filename(): pattern %s not found\n", pattern
    );

    globfree(&globbuf);        
    return 1;
}

static void flag_for_possible_removal(char* filename) {
    char path[256];
    sprintf(path, "../locality_scheduling/working_set_removal/%s", filename);
    touch_file(path);
    return;
}

// The client has (or will soon have) the given file.
// Try to send it results that use that file.
// If don't get any the first time,
// trigger the work generator, then try again.
//
static int send_results_for_file(
    char* filename,
    int& nsent,
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss,
    bool in_working_set
) {
    DB_RESULT result, prev_result;
    char buf[256], query[1024];
    int i, maxid, retval_max, retval_lookup;

    // find largest ID of results already sent to this user for this
    // file, if any.  Any result that is sent will have userid field
    // set, so unsent results can not be returned by this query.
    //
#ifdef USE_REGEXP
    sprintf(buf, "where userid=%d and name like '%s__%%'",
        reply.user.id, filename
    );
#else
    sprintf(buf, "where userid=%d and name>'%s__' and name<'%s__~'",
        reply.user.id, filename, filename
    );
#endif
    retval_max = result.max_id(maxid, buf);
    if (retval_max) {
        prev_result.id = 0;
    } else {
        retval_lookup = prev_result.lookup_id(maxid);
        if (retval_lookup) return retval_lookup;
    }

    nsent = 0;
    for (i=0; i<100; i++) {     // avoid infinite loop
        int query_retval;

        if (!reply.work_needed(true)) break;

        log_messages.printf(SCHED_MSG_LOG::DEBUG,
            "in_send_results_for_file(%s, %d) prev_result.id=%d\n", filename, i, prev_result.id
        );

        // find unsent result with next larger ID than previous largest ID
        //
        if (config.one_result_per_user_per_wu && prev_result.id) {

            // if one result per user per WU, insist on different WUID too
            //
#ifdef USE_REGEXP
            sprintf(query,
                "where name like '%s__%%' and id>%d and workunitid<>%d and server_state=%d order by id limit 1 ",
                filename, prev_result.id, prev_result.workunitid, RESULT_SERVER_STATE_UNSENT
            );
#else
            sprintf(query,
                "where name>'%s__' and name<'%s__~' and id>%d and workunitid<>%d and server_state=%d order by id limit 1 ",
                filename, filename, prev_result.id, prev_result.workunitid, RESULT_SERVER_STATE_UNSENT
            );
#endif
        } else {
#ifdef USE_REGEXP
            sprintf(query,
                "where name like '%s__%%' and id>%d and server_state=%d order by id limit 1 ",
                filename, prev_result.id, RESULT_SERVER_STATE_UNSENT
            );
#else
            sprintf(query,
                "where name>'%s__' and name<'%s__~' and id>%d and server_state=%d order by id limit 1 ",
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
            log_messages.printf(SCHED_MSG_LOG::DEBUG,
                "make_more_work_for_file(%s, %d)=%d\n", filename, i, make_work_retval
            );
          
            if (make_work_retval) {
                // can't make any more work for this file

                if (config.one_result_per_user_per_wu) {

                    // do an EXPENSIVE db query 
                    char query[256];
#ifdef USE_REGEXP
                    sprintf(query,
                        "where server_state=%d and name like '%s__%%' limit 1",
                        RESULT_SERVER_STATE_UNSENT, filename
                    );
#else
                    sprintf(query,
                        "where server_state=%d and name>'%s__' and name<'%s__~' limit 1",
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
                log_messages.printf(SCHED_MSG_LOG::DEBUG,
                    "No remaining work for file %s (%d), flagging for removal\n", filename, i
                );
                break;
            } // make_work_retval

            // if the user has not configured us to wait and try
            // again, we are finished.
            //
            if (!config.locality_scheduling_wait_period) {
                break;
            }

            // wait a bit and try again to find a suitable unsent result
            sleep(config.locality_scheduling_wait_period);
          
        } // query_retval
        else {
            int retval_send;

            // we found an unsent result, so try sending it.
            // This *should* always work.
            //
            retval_send = possibly_send_result(
                result, sreq, reply, platform, ss
            );
            boinc_db.commit_transaction();

            // if no app version, give up completely
            //
            if (retval_send == ERR_NO_APP_VERSION) return retval_send;

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
            } else if (!config.one_result_per_user_per_wu) {
                log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                    "Database inconsistency?  possibly_send_result(%d) failed for [RESULT#%d], returning %d\n",
                    i, result.id, retval_send
                );
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
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss, int& nsent, char *start_f, char *end_f
) {
    DB_RESULT result;
    char filename[256], min_resultname[256], query[1024];
    int retval;

    log_messages.printf(SCHED_MSG_LOG::DEBUG,
        "send_new_file_work_deterministic_seeded() start=%s end=%s\n", start_f, end_f?end_f:"+Inf");

    strcpy(min_resultname, start_f);
    while (1) {

        // are we done with the search yet?
        if (end_f && strcmp(min_resultname, end_f)>=0)
          break;

        sprintf(query,
            "where server_state=%d and name>'%s' order by name limit 1",
            RESULT_SERVER_STATE_UNSENT, min_resultname
        );
        retval = result.lookup(query);
        if (retval) break; // no more unsent results or at the end of the filenames, return -1
        retval = extract_filename(result.name, filename);
        if (retval) return retval; // not locality scheduled, now what???

        log_messages.printf(SCHED_MSG_LOG::DEBUG,
            "send_new_file_work_deterministic will try filename %s\n", filename
        );

        retval = send_results_for_file(
            filename, nsent, sreq, reply, platform, ss, false
        );
        if (nsent>0) break; 
        // construct a name which is lexically greater than the name of any result
        // which uses this file.
        sprintf(min_resultname, "%s__~", filename);
    }
    return 0;
}


// Returns 0 if this has sent additional new work.  Returns non-zero
// if it has not sent any new work.
//
static int send_new_file_work_deterministic(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    char start_filename[256];
    int getfile_retval, nsent=0;

    // get random filename as starting point for deterministic search
    if ((getfile_retval = get_working_set_filename(start_filename))) {
        strcpy(start_filename, "");
    }
  
    // start deterministic search with randomly chosen filename, go to
    // lexical maximum
    send_new_file_work_deterministic_seeded(sreq, reply, platform, ss, nsent, start_filename, NULL);
    if (nsent) {
        return 0;
    }

    // continue deterministic search at lexically first possible
    // filename, continue to randomly choosen one
    if (!getfile_retval && reply.work_needed(true)) {
        send_new_file_work_deterministic_seeded(sreq, reply, platform, ss, nsent, "", start_filename);
        if (nsent) {
            return 0;
        }
    }

    return 1;
}


static int send_new_file_work_working_set(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    char filename[256];
    int retval, nsent;

    retval = get_working_set_filename(filename);
    if (retval) return retval;

    log_messages.printf(SCHED_MSG_LOG::DEBUG,
        "send_new_file_working_set will try filename %s\n", filename
    );

    return send_results_for_file(
        filename, nsent, sreq, reply, platform, ss, true
    );
}

// prototype
static int send_old_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss, int timeout);

// The host doesn't have any files for which work is available.
// Pick new file to send.  Returns nonzero if no work is available.
//
static int send_new_file_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {

    while (reply.work_needed(true)) {
        int random_time=6*3600+rand()%(6*3600);

        // send work that's been hanging around the queue for more than 6
        // to 12 hours
        log_messages.printf(SCHED_MSG_LOG::DEBUG,
            "send_new_file_work() trying to send results created > %.1f hours ago\n", ((double)random_time)/3600.0);
        send_old_work(sreq, reply, platform, ss, random_time);
    
        if (reply.work_needed(true)) {
            log_messages.printf(SCHED_MSG_LOG::DEBUG,
                "send_new_file_work() trying to send from working set\n"
            );
            send_new_file_work_working_set(sreq, reply, platform, ss);
        }    

        if (reply.work_needed(true)) {
            log_messages.printf(SCHED_MSG_LOG::DEBUG,
                "send_new_file_work() trying deterministic method\n"
            );
            if (send_new_file_work_deterministic(sreq, reply, platform, ss)) {
                // if no work remains at all,
                // we learn it here and return nonzero.
                //
                return 1;
            }
        }
    }
    return 0;
}


// DAVID, this is missing a return value!  Am I right that this will
// also eventually move 'non locality' work through and out of the
// system?
static int send_old_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss, int timeout
) {
    char buf[1024], filename[256];
    int retval, extract_retval, nsent;
    DB_RESULT result;

    int cutoff = time(0) - timeout;
    boinc_db.start_transaction();
    sprintf(buf, "where server_state=%d and create_time<%d limit 1",
        RESULT_SERVER_STATE_UNSENT, cutoff
    );
    retval = result.lookup(buf);
    if (!retval) {
        retval = possibly_send_result(result, sreq, reply, platform, ss);
        boinc_db.commit_transaction();
        if (!retval) {
            log_messages.printf(SCHED_MSG_LOG::DEBUG,
                "send_old_work(%s) send laggard result [RESULT#%d]\n", result.name, result.id
            );
            extract_retval=extract_filename(result.name, filename);
            if (!extract_retval) {
                send_results_for_file(
                    filename, nsent, sreq, reply, platform, ss, false
                );
            } else {
                // David, is this right?  Is this the only place in
                // the locality scheduler that non-locality work //
                // gets done?
                log_messages.printf(SCHED_MSG_LOG::DEBUG,
                    "Note: sent NON-LOCALITY result %s\n", result.name
                );
            }
        }

    } else {
        boinc_db.commit_transaction();
    }
    // DAVID, YOU CHANGED THIS FROM VOID TO INT.  IS THIS THE RIGHT
    // RETURN VAL?  You should probably use the return value from
    // sent_results_for_file as well.
    return retval;
}

void send_work_locality(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    int i;
    int nsent, nfiles, j, k;

    // seed the random number generator
    unsigned int seed=time(0)+getpid();
    srand(seed); 

    nfiles = (int) sreq.file_infos.size();
    for (i=0; i<nfiles; i++)
        log_messages.printf(
                SCHED_MSG_LOG::DEBUG,
                "[HOST#%d]: has file %s\n", reply.host.id, sreq.file_infos[i].name
        );

    if (!nfiles)
        nfiles=1;
    j = rand()%nfiles;

    // send old work if there is any
    //
    if (config.locality_scheduling_send_timeout) {
        send_old_work(sreq, reply, platform, ss, config.locality_scheduling_send_timeout);
    }

    // send work for existing files
    //
    for (i=0; i<(int)sreq.file_infos.size(); i++) {
        k = (i+j)%nfiles;
        if (!reply.work_needed(true)) break;
        FILE_INFO& fi = sreq.file_infos[k];
        send_results_for_file(
            fi.name, nsent, sreq, reply, platform, ss, false
        );

        // if we couldn't send any work for this file, tell client to delete it
        //
        if (nsent == 0) {
            reply.file_deletes.push_back(fi);
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG,
                "[HOST#%d]: delete file %s (not needed)\n", reply.host.id, fi.name
            ); 
        } // nsent==0
    } // loop over files already on the host

    // send new files if needed
    //
    if (reply.work_needed(true)) {
        send_new_file_work(sreq, reply, platform, ss);
    }
}

// Explanation of the logic of this scheduler:

// (1) If there is an (one) unsent result which is older than
// (1) config.locality_scheduling_send_timeout (7 days) and is
// (1) feasible for the host, sent it.

// (2) If we did send a result in the previous step, then send any
// (2) additional results that are feasible for the same input file.

// (3) If additional results are needed, step through input files on
// (3) the host.  For each, if there are results that are feasible for
// (3) the host, send them.  If there are no results that are feasible
// (3) for the host, delete the input file from the host.

// (4) If additional results are needed, and there is (one) unsent
// (4) result which is older than 2 hours and is feasible for the
// (4) host, send it.

// (5) If we did send a result in the previous step, then send any
// (5) additional results that are feasible for the same input file.

// (6) If additional results are needed, select an input file name at
// (6) random from the current input file working set advertised by
// (6) the WU generator.  If there are results for this input file
// (6) that are feasible for this host, send them.

// (7) If additional results are needed, carry out an expensive,
// (7) deterministic search for ANY results that are feasible for the
// (7) host.  This search starts from a random filename advertised by
// (7) the WU generator, but continues cyclicly to cover ALL results
// (7) for ALL files. If a feasible result is found, send it.  Then
// (7) send any additional results that use the same input file.  If
// (7) there are no feasible results for the host, we are finished:
// (7) exit.

// (8) If addtional results are needed, return to step 4 above.







const char *BOINC_RCSID_238cc1aec4 = "$Id$";

