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

#include <list>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "boinc_db.h"
#include "parse.h"
#include "error_numbers.h"
#include "util.h"
#include "filesys.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

using namespace std;

#define LOCKFILE "file_deleter.out"
#define PIDFILE  "file_deleter.pid"

#define SLEEP_INTERVAL 5
#define RESULTS_PER_WU 4		// an estimation of redundancy 

SCHED_CONFIG config;

int id_modulus=0, id_remainder=0;

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
                    // the fact that no result files were found is a critical
                    // error if this was a successful result, but is to be
                    // expected if the result outcome was failure, since in
                    // that case there may well be no output file produced.
                    //
                    int debug_or_crit;
                    if (RESULT_OUTCOME_SUCCESS == result.outcome) {
                        debug_or_crit=SCHED_MSG_LOG::CRITICAL;
                    } else {
                        debug_or_crit=SCHED_MSG_LOG::DEBUG;
                    }
                    log_messages.printf(debug_or_crit,
                        "[RESULT#%d] outcome=%d client_state=%d No file %s to delete\n",
                        result.id, result.outcome, result.client_state, filename
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
bool do_pass(bool retry_error) {
    DB_WORKUNIT wu;
    DB_RESULT result;
    bool did_something = false;
    char buf[256];
    char mod_clause[256];
    int retval;
    int result_loop_count;

    check_stop_daemons();

    if (id_modulus) {
        sprintf(mod_clause, " and id %% %d = %d ",
                id_modulus, id_remainder
        );
    } else {
        strcpy(mod_clause, "");
    }

    if (retry_error) {
    	sprintf(buf, "where file_delete_state=%d or file_delete_state=%d %s limit 1000", FILE_DELETE_READY, FILE_DELETE_ERROR, mod_clause);
    } else {
    	sprintf(buf, "where file_delete_state=%d %s limit 1000", FILE_DELETE_READY, mod_clause);
    }
    while (!wu.enumerate(buf)) {
        did_something = true;

        retval = 0;
        if (!preserve_wu_files) {
            retval = wu_delete_files(wu);
        }
        if (retval) {
        	wu.file_delete_state = FILE_DELETE_ERROR;
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[WU#%d] update failed: %d\n", wu.id, retval);
        } else {
        	wu.file_delete_state = FILE_DELETE_DONE;
        }
        sprintf(buf, "file_delete_state=%d", wu.file_delete_state);
        retval= wu.update_field(buf);
    }

    for (result_loop_count=0; result_loop_count < RESULTS_PER_WU; result_loop_count++) {

        if ( retry_error ) {
        	sprintf(buf, "where file_delete_state=%d or file_delete_state=%d %s limit 1000", 
		        FILE_DELETE_READY, FILE_DELETE_ERROR, mod_clause);
        } else {
        	sprintf(buf, "where file_delete_state=%d %s limit 1000", FILE_DELETE_READY, mod_clause);
        }

        while (!result.enumerate(buf)) {
            did_something = true;
            retval = 0;
            if (!preserve_result_files) {
                retval = result_delete_files(result);
            }
            if (retval) {
        	result.file_delete_state = FILE_DELETE_ERROR;
	        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[RESULT#%d] update failed: %d\n", result.id, retval);
            } else {
        	result.file_delete_state = FILE_DELETE_DONE;
            }
	    sprintf(buf, "file_delete_state=%d", result.file_delete_state); 
	    retval= result.update_field(buf);
        }
    } 

    return did_something;
}


struct FILE_RECORD {
     string name;
     int date_modified;
};

bool operator == (const FILE_RECORD& fr1, const FILE_RECORD& fr2) {
    return (fr1.date_modified == fr2.date_modified && fr1.name == fr2.name);
}

bool operator < (const FILE_RECORD& fr1, const FILE_RECORD& fr2) {
    if (fr1.date_modified < fr2.date_modified)
        return true;
    else if (fr1.date_modified > fr2.date_modified)
        return false;
    else if (fr1.name < fr2.name)
        return true;
    else
        return false;
}

// list of antique files to delete.  If order is
// NOT sorted by mod time (primary key) and name
// (secondary key) then unsorted==true.
//
std::list<FILE_RECORD> files_to_delete;
bool unsorted = true;

// delete files from list of old files.  Returns number
// of files deleted, or negative for error.
int delete_antique_files(int max_to_delete) {
    int nfiles=0;

    if (files_to_delete.empty()) return 0;

    // sort list and eliminate duplicates
    //
    if (unsorted) {
        files_to_delete.sort();
        files_to_delete.unique();
        unsorted = false;
    }

    while (nfiles<max_to_delete && !files_to_delete.empty()) {
        char timestamp[128];
        char pathname[1024];
        int retval;

      FILE_RECORD fr=files_to_delete.front();
      check_stop_daemons();

        retval=get_file_path((char *)fr.name.c_str(), config.upload_dir, config.uldl_dir_fanout, pathname);
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "get_file_path(%s) failed: %d\n",
                fr.name.c_str(), retval
            );
            return -1;
        }

      strcpy(timestamp, time_to_string(fr.date_modified));
        log_messages.printf(SCHED_MSG_LOG::DEBUG,
            "deleting [antique %s] %s\n",
          timestamp, pathname 
        );
        if (unlink(pathname)) {
          int save_error=errno;
          log_messages.printf(SCHED_MSG_LOG::CRITICAL,
              "unlink(%s) failed: %s\n",
              pathname, strerror(save_error)
            );
          return -1;
      }
      else {
            nfiles++;
          files_to_delete.pop_front();
      }
    } // while
    return nfiles;
}


// construct a list of old files.  return number of files
// added to list, or negative number for error.
//
int add_antiques_to_list(int days) {
    char command[256];
    char single_line[1024];
    FILE *fp;
    int dirlen=strlen(config.upload_dir);
    struct passwd *apache_info=getpwnam("apache");
    int del_time=time(0)-86400*days;
    int nfiles=0;

    if (!apache_info) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "no user named 'apache' found!\n");
      return -1;
    }

    sprintf(command,  "find %s -type f -mtime +%d", config.upload_dir, days);
    
    // Now execute the command, read output on a stream.  We could use
    // find to also exec a 'delete' command.  But we want to log all
    // file names into the log, and do lots of sanity checking, so
    // this way is better.
    //
    if (!(fp=popen(command, "r"))) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "command %s failed\n", command);
        return -2;
    }

    while (fgets(single_line, 1024, fp)) {
        char pathname[1024];
      char *fname_at_end=NULL;
      int nchars=strlen(single_line);
      struct stat statbuf;
      char *err=NULL;
        FILE_RECORD fr;        

      // We can interrupt this at any point.
      // pclose() is called when process exits.
        check_stop_daemons();

      // Do serious sanity checking on the path before
      // adding the file!!
      //
      if (!err && nchars > 1022) err="line too long";
      if (!err && nchars < dirlen + 1) err="path shorter than upload directory name";
      if (!err && single_line[nchars-1] != '\n') err="no newline terminator in line";
      if (!err && strncmp(config.upload_dir, single_line, dirlen)) err="upload directory not in path";
      if (!err && single_line[dirlen] != '/') err="no slash separator in path";
      if (!err) single_line[nchars-1]='\0';
      if (!err && stat(single_line, &statbuf)) err="stat failed";
      if (!err && statbuf.st_mtime > del_time) err="file too recent";
      if (!err && apache_info->pw_uid != statbuf.st_uid) err="file not owned by httpd user";
      if (!err && !(fname_at_end=rindex(single_line+dirlen, '/'))) err="no trailing filename";
      if (!err) fname_at_end++;
      if (!err && !strlen(fname_at_end)) err="trailing filename too short";
      if (!err && get_file_path(fname_at_end, config.upload_dir, config.uldl_dir_fanout, pathname)) err="get_file_path() failed";
      if (!err && strcmp(pathname, single_line)) err="file in wrong hierarchical upload subdirectory";

      if (err) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
              "Can't list %s for deletion: %s\n",
              single_line, err
            );
          // This file deleting business is SERIOUS.  Give up at the
          // first sign of ANYTHING amiss.
          //
          pclose(fp);
          return -3;
      }

        // insert this file onto the list
      fr.date_modified = statbuf.st_mtime;
      fr.name = fname_at_end;
      files_to_delete.push_back(fr);
        unsorted = true;
        nfiles++;
       
    } // while (fgets(single_line, 1024, fp)) {  
    pclose(fp);
    log_messages.printf(SCHED_MSG_LOG::DEBUG,
        "Found %d antique files to delete\n",
        nfiles 
    );
    return nfiles;
}

// returns number of files found & added, or negative for error.
//
int find_antique_files() {
    char buf[256];
    DB_WORKUNIT wu;
    static int nextrun=0;

    check_stop_daemons();

    // list files once per day
    //
    if (time(0) < nextrun) return 0;
    nextrun = time(0)+86400;

    // Find the oldest workunit.  We could add "where
    // file_delete_state!=FILE_DELETE_DONE" to the query, but this
    // might create some race condition with the 'regular' file delete
    // mechanism, so better to do it like this.
    //
    sprintf(buf, "order by create_time limit 1");
    if (!wu.enumerate(buf)) {
        // Don't ever delete files younger than a month.
        //
        int days = 1 + (time(0) - wu.create_time)/86400;
        if (days<31) days=31;

      return add_antiques_to_list(days);
    }
    return 0;
}

bool do_antique_pass(bool& delete_antiques) {
    bool did_something=false;

    if (!delete_antiques) return did_something;

    // If any problems appear in deleting antique files
    // immediately DISABLE this feature.
    //
    int howmany = find_antique_files();
    if (howmany < 0) {
        delete_antiques=false;
        return did_something;
    }

    howmany = delete_antique_files(10);
    if (howmany < 0) delete_antiques=false;
    if (howmany > 0) did_something=true;   

    return did_something;
}


int main(int argc, char** argv) {
    int retval;
    bool asynch = false, one_pass = false, retry_error = false, delete_antiques = false;
    int i;

    check_stop_daemons();
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-asynch")) {
            asynch = true;
        } else if (!strcmp(argv[i], "-one_pass")) {
            one_pass = true;
        } else if (!strcmp(argv[i], "-retry_error")) {
            retry_error=true;
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
        } else if (!strcmp(argv[i], "-mod")) {
            id_modulus   = atoi(argv[++i]);
            id_remainder = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-delete_antiques")) {
          // If turned on, look for and then delete files in the
          // upload/ directory which are OLDER than any workunit in
          // the database.  Such files are ones that were uploaded
          // by hosts which turned in results far after the
          // deadline. These files are not deleted from the upload
          // directory unless you have this option enabled.
          delete_antiques = true;
        } else {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Unrecognized arg: %s\n", argv[i]);
        }
    }

    if (id_modulus) {
        log_messages.printf(SCHED_MSG_LOG::DEBUG, "Using mod'ed WU/result enumeration.  modulus = %d  remainder = %d\n",
                            id_modulus, id_remainder);
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
    while (1) {
        if (!do_pass(retry_error)) {
            if (one_pass) break;
            if (!do_antique_pass(delete_antiques)) {
                sleep(SLEEP_INTERVAL);
            }
        }
    }
}

const char *BOINC_RCSID_bd0d4938a6 = "$Id$";
