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

#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <glob.h>
#include <sys/stat.h>

#include "boinc_db.h"
#include "error_numbers.h"
#include "util.h"
#include "filesys.h"

#include "main.h"
#include "server_types.h"
#include "sched_shmem.h"
#include "sched_send.h"
#include "sched_msgs.h"
#include "sched_locality.h"
#include "sched_util.h"

#define VERBOSE_DEBUG

// returns zero if there is a file we can delete.
//
int delete_file_from_host(SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& sreply) {
    int nfiles = (int)sreq.file_infos.size();
    char buf[256];

    if (!nfiles) {

        double maxdisk=max_allowable_disk(sreq, sreply);

        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[HOST#%d]: no disk space but no files we can delete!\n", sreply.host.id
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

        if (sreply.disk_limits.max_used != 0.0) {
            strcat(buf, "Review preferences for maximum disk space used.");
        } else if (sreply.disk_limits.max_frac != 0.0) {
            strcat(buf, "Review preferences for maximum disk percentage used.");
        } else if (sreply.disk_limits.min_free != 0.0) {
            strcat(buf, "Review preferences for minimum disk free space allowed.");
        }
        USER_MESSAGE um(buf, "high");
        sreply.insert_message(um);
        sreply.set_delay(24*3600);
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
    int j = sreply.host.id % nfiles;
    FILE_INFO& fi = sreq.file_infos[j];
    sreply.file_deletes.push_back(fi);
    log_messages.printf(
        SCHED_MSG_LOG::MSG_DEBUG,
        "[HOST#%d]: delete file %s (make space)\n", sreply.host.id, fi.name
    );

    // give host 4 hours to nuke the file and come back.
    // This might in general be too soon, since host needs to complete any work
    // that depends upon this file, before it will be removed by core client.
    //
    sprintf(buf, "BOINC will delete file %s when no longer needed", fi.name);
    USER_MESSAGE um(buf, "low");
    sreply.insert_message(um);
    sreply.set_delay(4*3600);
    return 0;
}   

// returns true if the host already has the file, or if the file is
// included with a previous result being sent to this host.
//
bool host_has_file(
    SCHEDULER_REQUEST& request,
    SCHEDULER_REPLY& reply,
    char *filename,
    bool skip_last_wu
) {
    int i, uplim;
    bool has_file=false;

    // loop over files already on host to see if host already has the
    // file
    //
    for (i=0; i<(int)request.file_infos.size(); i++) {
        FILE_INFO& fi = request.file_infos[i];
        if (!strcmp(filename, fi.name)) {
            has_file=true;
            break;
        }
    }

    if (has_file) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "[HOST#%d] Already has file %s\n", reply.host.id, filename
        );
        return true;
    }

    // loop over files being sent to host to see if this file has
    // already been counted.
    //
    uplim=(int)reply.wus.size();
    if (skip_last_wu) {
        uplim--;
    }

    for (i=0; i<uplim; i++) {
        char wu_filename[256];

        if (extract_filename(reply.wus[i].name, wu_filename)) {
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
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "[HOST#%d] file %s already in scheduler reply(%d)\n", reply.host.id, filename, i
        );
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
int decrement_disk_space_locality(
    WORKUNIT& wu, SCHEDULER_REQUEST& request,
    SCHEDULER_REPLY& reply
) {
    char filename[256], path[512];
    int filesize;
    struct stat buf;

    // get filename from WU name
    //
    if (extract_filename(wu.name, filename)) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "No filename found in WU#%d (%s)\n",
            wu.id, wu.name
        );
        return -1;
    }

    // when checking to see if the host has the file, we need to
    // ignore the last WU included at the end of the reply, since it
    // corresponds to the one that we are (possibly) going to send!
    // So make a copy and pop the current WU off the end.

    if (!host_has_file(request, reply, filename, true))
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
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "Unable to find file %s at path %s\n", filename, path
        );
        return -1;
    }

    filesize=buf.st_size;
    
    if (filesize<wu.rsc_disk_bound) {
        reply.wreq.disk_available -= (wu.rsc_disk_bound-filesize);
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "[HOST#%d] reducing disk needed for WU by %d bytes (length of %s)\n",
            reply.host.id, filesize, filename
        );
        return 0;
    }
                  
    log_messages.printf(
        SCHED_MSG_LOG::MSG_CRITICAL,
        "File %s size %d bytes > wu.rsc_disk_bound for WU#%d (%s)\n",
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
    if (retval) return ERR_DB_NOT_FOUND;

    // wu_is_infeasible() returns a bitmask of potential reasons
    // why the WU is not feasible.  These are defined in sched_send.h.
    // INFEASIBLE_MEM, INFEASIBLE_DISK, INFEASIBLE_CPU.
    // 
    if (wu_is_infeasible(wu, sreq, reply)) {
        return ERR_INSUFFICIENT_RESOURCE;
    }

    if (config.one_result_per_user_per_wu) {
        sprintf(buf, "where userid=%d and workunitid=%d", reply.user.id, wu.id);
        retval = result2.count(count, buf);
        if (retval) return ERR_DB_NOT_FOUND;
        if (count > 0) return ERR_WU_USER_RULE;
    }

    retval = get_app_version(
        wu, app, avp, sreq, reply, platform, ss
    );
    if (retval) return ERR_NO_APP_VERSION;

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
            SCHED_MSG_LOG::MSG_DEBUG,
            "work generator says no work remaining for file %s\n", filename
        );
        return -1;
    }
        
    // open and touch a file in the need_work/
    // directory as a way of indicating that we need work for this file.
    // If this operation fails, don't worry or tarry!
    //
    sprintf(fullpath, "../locality_scheduling/need_work/%s", filename);
    if (boinc_touch_file(fullpath)) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "unable to touch %s\n", fullpath
        );
        return -1;
    }

    log_messages.printf(
        SCHED_MSG_LOG::MSG_DEBUG,
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
    
    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
        "get_working_set_filename(): returning %s\n", filename
    );

    return 0;

error_exit:
    log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
        "get_working_set_filename(): pattern %s not found\n", pattern
    );

    globfree(&globbuf);        
    return 1;
}

static void flag_for_possible_removal(char* filename) {
    char path[256];
    sprintf(path, "../locality_scheduling/working_set_removal/%s", filename);
    boinc_touch_file(path);
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
    bool /*in_working_set*/
) {
    DB_RESULT result, prev_result;
    char buf[256], query[1024];
    int i, maxid, retval_max, retval_lookup, sleep_made_no_work=0;

    nsent = 0;

    if (!reply.work_needed(true)) {
        return 0;
    }

    // find largest ID of results already sent to this user for this
    // file, if any.  Any result that is sent will have userid field
    // set, so unsent results can not be returned by this query.
    //
#ifdef USE_REGEXP
    char pattern[256], escaped_pattern[256];
    sprintf(pattern, "%s__", filename);
    escape_mysql_like_pattern(pattern, escaped_pattern);
    sprintf(buf, "where userid=%d and name like binary '%s%%'",
        reply.user.id, escaped_pattern
    );
#else
    sprintf(buf, "where userid=%d and name>binary '%s__' and name<binary '%s__~'",
        reply.user.id, filename, filename
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

        if (!reply.work_needed(true)) break;

        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "in_send_results_for_file(%s, %d) prev_result.id=%d\n", filename, i, prev_result.id
        );

        // find unsent result with next larger ID than previous largest ID
        //
        if (config.one_result_per_user_per_wu && prev_result.id) {

            // if one result per user per WU, insist on different WUID too
            //
#ifdef USE_REGEXP
            sprintf(query,
                "where name like binary '%s%%' and id>%d and workunitid<>%d and server_state=%d order by id limit 1 ",
                escaped_pattern, prev_result.id, prev_result.workunitid, RESULT_SERVER_STATE_UNSENT
            );
#else
            sprintf(query,
                "where name>binary '%s__' and name<binary '%s__~' and id>%d and workunitid<>%d and server_state=%d order by id limit 1 ",
                filename, filename, prev_result.id, prev_result.workunitid, RESULT_SERVER_STATE_UNSENT
            );
#endif
        } else {
#ifdef USE_REGEXP
            sprintf(query,
                "where name like binary '%s%%' and id>%d and server_state=%d order by id limit 1 ",
                escaped_pattern, prev_result.id, RESULT_SERVER_STATE_UNSENT
            );
#else
            sprintf(query,
                "where name>binary '%s__' and name<binary '%s__~' and id>%d and server_state=%d order by id limit 1 ",
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
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "make_more_work_for_file(%s, %d)=%d\n", filename, i, make_work_retval
            );
          
            if (make_work_retval) {
                // can't make any more work for this file

                if (config.one_result_per_user_per_wu) {

                    // do an EXPENSIVE db query 
#ifdef USE_REGEXP
                    sprintf(query,
                        "where server_state=%d and name like binary '%s%%' limit 1",
                        RESULT_SERVER_STATE_UNSENT, escaped_pattern
                    );
#else
                    sprintf(query,
                        "where server_state=%d and name>binary '%s__' and name<binary '%s__~' limit 1",
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
                log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                    "No remaining work for file %s (%d), flagging for removal\n", filename, i
                );
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
            retval_send = possibly_send_result(
                result, sreq, reply, platform, ss
            );
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
                log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                    "Database inconsistency?  possibly_send_result(%d) failed for [RESULT#%d], returning %d\n",
                    i, result.id, retval_send
                );
            } else {
                log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                    "possibly_send_result [RESULT#%d]: %s\n",
                    result.id, boincerror(retval_send)
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
    SCHED_SHMEM& ss, int& nsent, const char *start_f, const char *end_f
) {
    DB_RESULT result;
    char filename[256], min_resultname[256], query[1024];
    int retval;

    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
        "send_new_file_work_deterministic_seeded() start=%s end=%s\n", start_f, end_f?end_f:"+Inf");

    strcpy(min_resultname, start_f);
    while (1) {

        // are we done with the search yet?
        if (end_f && strcmp(min_resultname, end_f)>=0)
          break;

#if 0
        // an alternative here is to add ANOTHER index on name, server_state
        // to the result table.
        sprintf(query,
            "where server_state=%d and name>'%s' order by name limit 1",
            RESULT_SERVER_STATE_UNSENT, min_resultname
        );
#endif

        sprintf(query,
            "where name>'%s' order by name limit 1",
             min_resultname
        );

        retval = result.lookup(query);
        if (retval) break; // no more unsent results or at the end of the filenames, return -1
        retval = extract_filename(result.name, filename);
        if (retval) return retval; // not locality scheduled, now what???

        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "send_new_file_work_deterministic will try filename %s\n", filename
        );

        retval = send_results_for_file(
            filename, nsent, sreq, reply, platform, ss, false
        );

        if (retval==ERR_NO_APP_VERSION || retval==ERR_INSUFFICIENT_RESOURCE) return retval;

        if (nsent>0 || !reply.work_needed(true)) break; 
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
        send_new_file_work_deterministic_seeded(
            sreq, reply, platform, ss, nsent, "", start_filename
        );
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

    log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
        "send_new_file_working_set will try filename %s\n", filename
    );

    return send_results_for_file(
        filename, nsent, sreq, reply, platform, ss, true
    );
}

// prototype
static int send_old_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss, int t_min, int t_max);

// The host doesn't have any files for which work is available.
// Pick new file to send.  Returns nonzero if no work is available.
//
static int send_new_file_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {

    while (reply.work_needed(true)) {
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
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "send_new_file_work(): try to send old work\n"
        );

        retval_sow=send_old_work(sreq, reply, platform, ss, start, end);

        if (retval_sow==ERR_NO_APP_VERSION || retval_sow==ERR_INSUFFICIENT_RESOURCE) return retval_sow;

    
        while (reply.work_needed(true) && retry<10) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "send_new_file_work(%d): try to send from working set\n", retry
            );
            retry++;
            retval_snfwws=send_new_file_work_working_set(sreq, reply, platform, ss);
            if (retval_snfwws==ERR_NO_APP_VERSION || retval_snfwws==ERR_INSUFFICIENT_RESOURCE) return retval_snfwws;

        }    

        if (reply.work_needed(true)) {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "send_new_file_work(): try deterministic method\n"
            );
            if (send_new_file_work_deterministic(sreq, reply, platform, ss)) {
                // if no work remains at all,
                // we learn it here and return nonzero.
                //
                return 1;
            }
        }
    } // while reply.work_needed(true)
    return 0;
}


// DAVID, this is missing a return value!  Am I right that this will
// also eventually move 'non locality' work through and out of the
// system?
//
// This looks for work created in the range t_min < t < t_max.  Use
// t_min=INT_MIN if you wish to leave off the left constraint.
//
static int send_old_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss, int t_min, int t_max
) {
    char buf[1024], filename[256];
    int retval, extract_retval, nsent;
    DB_RESULT result;
    int now=time(0);

    if (!reply.work_needed(true)) {
        return 0;
    }


    boinc_db.start_transaction();

    if (t_min != INT_MIN) {
        sprintf(buf, "where server_state=%d and %d<create_time and create_time<%d limit 1",
            RESULT_SERVER_STATE_UNSENT, t_min, t_max
        );
    }
    else {
        sprintf(buf, "where server_state=%d and create_time<%d limit 1",
            RESULT_SERVER_STATE_UNSENT, t_max
        );
    }

    retval = result.lookup(buf);
    if (!retval) {
        retval = possibly_send_result(result, sreq, reply, platform, ss);
        boinc_db.commit_transaction();
        if (!retval) {
            double age=(now-result.create_time)/3600.0;
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "send_old_work(%s) sent result created %.1f hours ago [RESULT#%d]\n", result.name, age, result.id
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
                log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                    "Note: sent NON-LOCALITY result %s\n", result.name
                );
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
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "send_old_work() no feasible result younger than %.1f hours and older than %.1f hours\n",
                young, older
            );
        }
        else {
            log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
                "send_old_work() no feasible result older than %.1f hours\n",
                older
            );
        }
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
    int i, nsent, nfiles, j;

    // seed the random number generator
    unsigned int seed=time(0)+getpid();
    srand(seed); 

    nfiles = (int) sreq.file_infos.size();
    for (i=0; i<nfiles; i++)
        log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG,
                "[HOST#%d]: has file %s\n", reply.host.id, sreq.file_infos[i].name
        );

    if (!nfiles)
        nfiles=1;
    j = rand()%nfiles;

    // send old work if there is any. send this only to hosts which have
    // high-bandwidth connections, since asking dial-up users to upload
    // (presumably large) data files is onerous.
    //
    if (config.locality_scheduling_send_timeout && sreq.host.n_bwdown>100000) {
        int until=time(0)-config.locality_scheduling_send_timeout;
        int retval_sow=send_old_work(sreq, reply, platform, ss, INT_MIN, until);
        if (retval_sow==ERR_NO_APP_VERSION || retval_sow==ERR_INSUFFICIENT_RESOURCE) return;
    }

    // send work for existing files
    //
    for (i=0; i<(int)sreq.file_infos.size(); i++) {
        int k = (i+j)%nfiles;
        int retval_srff;

        if (!reply.work_needed(true)) break;
        FILE_INFO& fi = sreq.file_infos[k];
        retval_srff=send_results_for_file(
            fi.name, nsent, sreq, reply, platform, ss, false
        );

        if (retval_srff==ERR_NO_APP_VERSION || retval_srff==ERR_INSUFFICIENT_RESOURCE) return;

        // if we couldn't send any work for this file, and we STILL need work,
        // then it must be that there was no additional work remaining for this
        // file which is feasible for this host.  In this case, delete the file.
        // If the work was not sent for other (dynamic) reason such as insufficient
        // cpu, then DON'T delete the file.
        //
        if (nsent == 0 && reply.work_needed(true)) {
            reply.file_deletes.push_back(fi);
            log_messages.printf(
                SCHED_MSG_LOG::MSG_DEBUG,
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

// (8) If addtional results are needed, return to step 4 above.







const char *BOINC_RCSID_238cc1aec4 = "$Id$";

