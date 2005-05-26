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


// file_deleter: deletes files that are no longer needed

#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <errno.h>

#include "boinc_db.h"
#include "parse.h"
#include "error_numbers.h"
#include "util.h"
#include "filesys.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define LOCKFILE "file_deleter.out"
#define PIDFILE  "file_deleter.pid"

#define SLEEP_INTERVAL 5

SCHED_CONFIG config;

// Given a filename, find its full path in the upload directory hierarchy
// Return an error if file isn't there.
//
int get_file_path(char *filename, char* upload_dir, int fanout, char* path) {

    dir_hier_path(filename, upload_dir, fanout, true, path);
    if (boinc_file_exists(path)) {
        return 0;
    }
            	
    // TODO: get rid of the old hash in about 3/2005
    //
    dir_hier_path(filename, upload_dir, fanout, false, path);
    if (boinc_file_exists(path)) {
        return 0;
    }
    return ERR_NOT_FOUND;
}


int wu_delete_files(WORKUNIT& wu) {
    char* p;
    char filename[256], pathname[256], buf[LARGE_BLOB_SIZE];
    bool no_delete=false;
    int count_deleted = 0, retval, mthd_retval = 0;

    if (strstr(wu.name, "nodelete")) return mthd_retval;

    safe_strcpy(buf, wu.xml_doc);
    
    p = strtok(buf, "\n");
    strcpy(filename, "");
    while (p) {
        if (parse_str(p, "<name>", filename, sizeof(filename))) {
        } else if (match_tag(p, "<file_info>")) {
            no_delete = false;
            strcpy(filename, "");
        } else if (match_tag(p, "<no_delete/>")) {
            no_delete = true;
        } else if (match_tag(p, "</file_info>")) {
            if (!no_delete) {
                retval = get_file_path(filename, config.download_dir, config.uldl_dir_fanout,
                    pathname
                );
                if (retval) {
                    log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                        "[WU#%d] get_file_path: %s: %d\n",
                        wu.id, filename, retval
                    );
                } else {
                    log_messages.printf(SCHED_MSG_LOG::NORMAL,
                        "[WU#%d] deleting %s\n", wu.id, filename
                    );
                    retval = unlink(pathname);
                    if (retval && strlen(config.download_dir_alt)) {
                    	sprintf(pathname, "%s/%s", config.download_dir_alt, filename);
                    	retval = unlink(pathname);
                    }
                    if (retval) {
                		log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                            "[WU#%d] unlink %s failed: %d\n",
                            wu.id, filename, retval
                        );
                    	mthd_retval = ERR_UNLINK;
                    } else {
                		count_deleted++;
                    }
                }
            }
        }
        p = strtok(0, "\n");
    }
    log_messages.printf(SCHED_MSG_LOG::DEBUG, "[WU#%d] deleted %d file(s)\n", wu.id, count_deleted);
    return mthd_retval;
}

int result_delete_files(RESULT& result) {
    char* p;
    char filename[256], pathname[256], buf[LARGE_BLOB_SIZE];
    bool no_delete=false;
    int count_deleted = 0, retval, mthd_retval = 0;

    safe_strcpy(buf, result.xml_doc_in);
    p = strtok(buf,"\n");
    while (p) {
        if (parse_str(p, "<name>", filename, sizeof(filename))) {
        } else if (match_tag(p, "<file_info>")) {
            no_delete = false;
            strcpy(filename, "");
        } else if (match_tag(p, "<no_delete/>")) {
            no_delete = true;
        } else if (match_tag(p, "</file_info>")) {
            if (!no_delete) {
                retval = get_file_path(
                    filename, config.upload_dir, config.uldl_dir_fanout,
                    pathname
                );
                if (retval) {
                    log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                        "[RESULT#%d] get_file_path: %s: %d\n",
                        result.id, filename, retval
                    );
                } else {
                    retval = unlink(pathname);
                    if (retval) {
                    	mthd_retval = ERR_UNLINK;
                    	log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                        	"[RESULT#%d] unlink %s returned %d %s\n",
                         	result.id, pathname, retval,
                         	(retval && errno)?strerror(errno):""
                        );
                    } else {
                    	count_deleted++;
                    	log_messages.printf(SCHED_MSG_LOG::NORMAL,
                        	"[RESULT#%d] unlinked %s\n", result.id, pathname
                    	);
                    }
                }
            }
        }
        p = strtok(0, "\n");
    }

    log_messages.printf(SCHED_MSG_LOG::DEBUG,
        "[RESULT#%d] deleted %d file(s)\n", result.id, count_deleted
    );
    return mthd_retval;
}

// set by corresponding command line arguments.
static bool preserve_wu_files=false;
static bool preserve_result_files=false;

// return nonzero if did anything
//
bool do_pass() {
    DB_WORKUNIT wu;
    DB_RESULT result;
    bool did_something = false, got_error = false;
    char buf[256];
    int retval;

    check_stop_daemons();

    sprintf(buf, "where file_delete_state=%d limit 1000", FILE_DELETE_READY);
    while (!wu.enumerate(buf)) {
        did_something = true;
        
        retval = 0;
        if (!preserve_wu_files) {
            retval = wu_delete_files(wu);
        }
        if (retval) {
            got_error = true;
        } else {
        	wu.file_delete_state = FILE_DELETE_DONE;
	        sprintf(buf, "file_delete_state=%d", wu.file_delete_state);
	        retval= wu.update_field(buf);
	        if (retval) {
	             log_messages.printf(SCHED_MSG_LOG::CRITICAL,
	                "[WU#%d] update failed: %d\n", wu.id, retval 
	             );
	        }
        }
    }

    sprintf(buf, "where file_delete_state=%d limit 1000", FILE_DELETE_READY);
    while (!result.enumerate(buf)) {
        did_something = true;
        retval = 0;
        if (!preserve_result_files) {
            retval = result_delete_files(result);
        }
        if (retval) {
            got_error = true;
        } else {
        	result.file_delete_state = FILE_DELETE_DONE;
	        sprintf(buf, "file_delete_state=%d", result.file_delete_state); 
	        retval= result.update_field(buf);
	        if (retval) {
	            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
	                "[RESULT#%d] update failed: %d\n", result.id, retval
	            );
	        } 
        }
    }
    if (got_error) {
        sleep(SLEEP_INTERVAL);
    }
    return did_something;
}

int main(int argc, char** argv) {
    int retval;
    bool asynch = false, one_pass = false;
    int i;

    check_stop_daemons();
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-preserve_wu_files")) {
            // This option is primarily for testing.
            // If enabled, the file_deleter will function 'normally'
            // and will update the database,
            // but will not actually delete the workunit input files.
            // It's equivalent to setting <no_delete/>
            // for all workunit input files.
            //
            preserve_wu_files = true;
        } else if (!strcmp(argv[i], "-preserve_result_files")) {
            // This option is primarily for testing.
            // If enabled, the file_deleter will function 'normally'
            // and will update the database,
            // but will not actually delete the result output files.
            // It's equivalent to setting <no_delete/>
            // for all result output files.
            //
            preserve_result_files = true;
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Unrecognized arg: %s\n", argv[i]);
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Can't parse config file\n");
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    // // Call lock_file after fork(), because file locks are not always inherited
    // if (lock_file(LOCKFILE)) {
    //     log_messages.printf(SCHED_MSG_LOG::NORMAL, "Another copy of file deleter is running\n");
    //     exit(1);
    // }
    // write_pid_file(PIDFILE);
    log_messages.printf(SCHED_MSG_LOG::NORMAL, "Starting\n");

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "can't open DB\n");
        exit(1);
    }
    install_stop_signal_handler();
    if (one_pass) {
        do_pass();
    } else {
        while (1) {
            if (!do_pass()) sleep(SLEEP_INTERVAL);
        }
    }
}

const char *BOINC_RCSID_bd0d4938a6 = "$Id$";
