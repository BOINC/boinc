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

// db_purge options
//
// purge workunit and result records that are no longer needed.
// Specifically, purges WUs for which file_delete_state=DONE;
// this occurs only when it has been assimilated
// and all results have server_state=OVER.
// Purging a WU means writing it and all its results
// to XML-format archive files, then deleting it and its results from the DB.
//
// The XML files have names of the form
// wu_archive_TIME and result_archive_TIME
// where TIME is the time it was created.
// In addition there are index files associating each WU and result ID
// with the timestamp of the file it's in.
//
// Options:
//
// -min_age_days n      purge WUs with mod_time at least N days in the past
// -max n               purge at most N WUs
// -one_pass            go until nothing left to purge, then exit
//                      default: keep scanning indefinitely
// -max_wu_per_file n   write at most N WUs to an archive file
                // The file is then closed and another file is opened.
                // This can be used to get a series of small files
                // instead of one huge file.
// -zip
            // compress output files using zip.  If used with
            // -max_wu_per_file then the files get compressed after
            // being closed.  In any case the files are compressed
            // when db_purge exits on a signal.
// -gzip
            // compress output files using gzip.  If used with
            // -max_wu_per_file then the files get compressed after
            // being closed.  In any case the files are compressed
            // when db_purge exits on a signal.


#include "config.h"
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <string>
#include <time.h>
#include <errno.h>

using namespace std;

#include "boinc_db.h"
#include "util.h"
#include "parse.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#include "error_numbers.h"

#define WU_FILENAME_PREFIX              "wu_archive"
#define RESULT_FILENAME_PREFIX          "result_archive"
#define WU_INDEX_FILENAME_PREFIX        "wu_index"
#define RESULT_INDEX_FILENAME_PREFIX    "result_index"

#define DB_QUERY_LIMIT                  1000

SCHED_CONFIG    config;
FILE            *wu_stream=NULL;
FILE            *re_stream=NULL;
FILE            *wu_index_stream=NULL;
FILE            *re_index_stream=NULL;
int             time_int=0;
int min_age_days=0;

// These is used if limiting the total number of workunits to eliminate
int purged_workunits= 0;

// If nonzero, maximum number of workunits to purge.
// Since all results associated with a purged workunit are also purged,
// this also limits the number of purged results.
//
int max_number_workunits_to_purge=0;

// set on command line if compression of archives is desired
#define COMPRESSION_NONE    0
#define COMPRESSION_GZIP    1
#define COMPRESSION_ZIP     2

// subscripts MUST be in agreement with defines above
const char *suffix[3]={"", ".gz", ".zip"};

// default is no compression
int compression_type=COMPRESSION_NONE;

// set on command line if archive files should be closed and re-opened
// after getting some max no of WU in the file
int max_wu_per_file=0;

// keep track of how many WU archived in file so far
int wu_stored_in_file=0;

bool time_to_quit() {
    if (max_number_workunits_to_purge) {
        if (purged_workunits >= max_number_workunits_to_purge) return true;
    }
    return false;
}

// Open an archive.  Only subtle thing is that if the user has
// asked for compression, then we popen(2) a pipe to gzip or zip.
// This does 'in place' compression.
//
void open_archive(const char* filename_prefix, FILE*& f){
    char path[256];
    char command[512];

    // append appropriate suffix for file type
    sprintf(path, "../archives/%s_%d.xml", filename_prefix, time_int);
    strcat(path, suffix[compression_type]);

    // and construct appropriate command if needed
    if (compression_type==COMPRESSION_GZIP) {
        sprintf(command, "gzip - > %s", path);
    }
   
    if (compression_type==COMPRESSION_ZIP) {
        sprintf(command, "zip %s -", path);
    }

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
        "Opening archive %s\n", path
    );

    // in the case with no compression, just open the file, else open
    // a pipe to the compression executable.
    //
    if (compression_type==COMPRESSION_NONE) {   
        if (!(f = fopen( path,"w"))) {
            log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,"Can't open archive file %s %s\n",
                path, errno?strerror(errno):""
            );
            exit(3);
        }
    } else if (!(f = popen(command,"w"))) {
        log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,"Can't open pipe %s %s\n", 
            command, errno?strerror(errno):""
        );
        exit(4);
    }

    // set buffering to line buffered, since we are outputing XML on a
    // line-by-line basis.
    //
    setlinebuf(f);

    return;
}

void close_archive(const char *filename, FILE*& fp){
    char path[256];

    // Set file pointer to NULL after closing file to indicate that it's closed.
    //
    if (!fp) return;

    // In case of errors, carry on anyway.  This is deliberate, not lazy
    //
    if (compression_type==COMPRESSION_NONE) {
        fclose(fp);
    } else {
        pclose(fp);
    }
    
    fp = NULL;

    // append appropriate file type
    sprintf(path, "../archives/%s_%d.xml", filename, time_int);
    strcat(path, suffix[compression_type]);
    
    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
        "Closed archive file %s containing records of %d workunits\n",
        path, wu_stored_in_file
    );

    return;
}

// opens the various archive files.  Guarantees that the timestamp
// does not equal the previous timestamp
//
void open_all_archives() {
    int old_time=time_int;
    
    // make sure we get a NEW value of the file timestamp!
    //
    while (old_time == (time_int = (int)time(0))) {
        sleep(1);
    }
    
    // open all the archives.
    open_archive(WU_FILENAME_PREFIX, wu_stream);
    open_archive(RESULT_FILENAME_PREFIX, re_stream);
    open_archive(RESULT_INDEX_FILENAME_PREFIX, re_index_stream);
    open_archive(WU_INDEX_FILENAME_PREFIX, wu_index_stream);
    fprintf(wu_stream, "<archive>\n");
    fprintf(re_stream, "<archive>\n");

    return;
}

// closes (and optionally compresses) the archive files.  Clears file
// pointers to indicate that files are not open.
//
void close_all_archives() {
    fprintf(wu_stream, "</archive>\n");
    fprintf(re_stream, "</archive>\n");
    close_archive(WU_FILENAME_PREFIX, wu_stream);
    close_archive(RESULT_FILENAME_PREFIX, re_stream);
    close_archive(RESULT_INDEX_FILENAME_PREFIX, re_index_stream);
    close_archive(WU_INDEX_FILENAME_PREFIX, wu_index_stream);
    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
        "Closed archive files with %d workunits\n",
        wu_stored_in_file
    );

    return;
}


// The exit handler always calls this at the end to be sure that the
// database is closed cleanly.
//
void close_db_exit_handler() {
    boinc_db.close();
    return; 
}

int archive_result(DB_RESULT& result) {
    fprintf(re_stream,
        "<result_archive>\n"
        "    <id>%d</id>\n",
        result.id
    );

    string r1, r2;
    r1= result.stderr_out;
    xml_escape(r1, r2);

    fprintf(
        re_stream,
        "  <create_time>%d</create_time>\n"
        "  <workunitid>%d</workunitid>\n"
        "  <server_state>%d</server_state>\n"
        "  <outcome>%d</outcome>\n"
        "  <client_state>%d</client_state>\n"
        "  <hostid>%d</hostid>\n"
        "  <userid>%d</userid>\n"
        "  <report_deadline>%d</report_deadline>\n"
        "  <sent_time>%d</sent_time>\n"
        "  <received_time>%d</received_time>\n"
        "  <name>%s</name>\n"
        "  <cpu_time>%.15e</cpu_time>\n"
        "  <xml_doc_in>%s</xml_doc_in>\n"
        "  <xml_doc_out>%s</xml_doc_out>\n"
        "  <stderr_out>%s</stderr_out>\n"
        "  <batch>%d</batch>\n"
        "  <file_delete_state>%d</file_delete_state>\n"
        "  <validate_state>%d</validate_state>\n"
        "  <claimed_credit>%.15e</claimed_credit>\n"
        "  <granted_credit>%.15e</granted_credit>\n"
        "  <opaque>%f</opaque>\n"
        "  <random>%d</random>\n"
        "  <app_version_num>%d</app_version_num>\n"
        "  <appid>%d</appid>\n"
        "  <exit_status>%d</exit_status>\n"
        "  <teamid>%d</teamid>\n"
        "  <priority>%d</priority>\n"
        "  <mod_time>%s</mod_time>\n",
        result.create_time,
        result.workunitid,
        result.server_state,
        result.outcome,
        result.client_state,
        result.hostid,
        result.userid,
        result.report_deadline,
        result.sent_time,
        result.received_time,
        result.name,
        result.cpu_time,
        result.xml_doc_in,
        result.xml_doc_out,
        r2.c_str(),
        result.batch,
        result.file_delete_state,
        result.validate_state,
        result.claimed_credit,
        result.granted_credit,
        result.opaque,
        result.random,
        result.app_version_num,
        result.appid,
        result.exit_status,
        result.teamid,
        result.priority,
        result.mod_time
    );

    fprintf(re_stream,
        "</result_archive>\n"
    );

    fprintf(re_index_stream,
        "%d     %d\n",
        result.id, time_int
    );

    return 0;
}

int archive_wu(DB_WORKUNIT& wu) {
    fprintf(wu_stream,
        "<workunit_archive>\n"
        "    <id>%d</id>\n",
        wu.id
    );
    fprintf(wu_stream,
        "  <create_time>%d</create_time>\n"
        "  <appid>%d</appid>\n"
        "  <name>%s</name>\n"
        "  <xml_doc>%s</xml_doc>\n"
        "  <batch>%d</batch>\n"
        "  <rsc_fpops_est>%.15e</rsc_fpops_est>\n"
        "  <rsc_fpops_bound>%.15e</rsc_fpops_bound>\n"
        "  <rsc_memory_bound>%.15e</rsc_memory_bound>\n"
        "  <rsc_disk_bound>%.15e</rsc_disk_bound>\n"
        "  <need_validate>%d</need_validate>\n"
        "  <canonical_resultid>%d</canonical_resultid>\n"
        "  <canonical_credit>%.15e</canonical_credit>\n"
        "  <transition_time>%d</transition_time>\n"
        "  <delay_bound>%d</delay_bound>\n"
        "  <error_mask>%d</error_mask>\n"
        "  <file_delete_state>%d</file_delete_state>\n"
        "  <assimilate_state>%d</assimilate_state>\n"
        "  <hr_class>%d</hr_class>\n"
        "  <opaque>%f</opaque>\n"
        "  <min_quorum>%d</min_quorum>\n"
        "  <target_nresults>%d</target_nresults>\n"
        "  <max_error_results>%d</max_error_results>\n"
        "  <max_total_results>%d</max_total_results>\n"
        "  <max_success_results>%d</max_success_results>\n"
        "  <result_template_file>%s</result_template_file>\n"
        "  <priority>%d</priority>\n"
        "  <mod_time>%s</mod_time>\n",
        wu.create_time,
        wu.appid,
        wu.name,
        wu.xml_doc,
        wu.batch,
        wu.rsc_fpops_est,
        wu.rsc_fpops_bound,
        wu.rsc_memory_bound,
        wu.rsc_disk_bound,
        wu.need_validate,
        wu.canonical_resultid,
        wu.canonical_credit,
        wu.transition_time,
        wu.delay_bound,
        wu.error_mask,
        wu.file_delete_state,
        wu.assimilate_state,
        wu.hr_class,
        wu.opaque,
        wu.min_quorum,
        wu.target_nresults,
        wu.max_error_results,
        wu.max_total_results,
        wu.max_success_results,
        wu.result_template_file,
        wu.priority,
        wu.mod_time
    );

    fprintf(wu_stream,
        "</workunit_archive>\n"
    );

    fprintf(wu_index_stream,
        "%d     %d\n",
        wu.id, time_int
    );

    return 0;
}

int purge_and_archive_results(DB_WORKUNIT& wu, int& number_results) {
    int retval= 0;
    DB_RESULT result;
    char buf[256];

    number_results=0;

    sprintf(buf, "where workunitid=%d", wu.id);
    while (!result.enumerate(buf)) {
        retval= archive_result(result);
        if (retval) return retval;
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "Archived result [%d] to a file\n", result.id
        );
        retval= result.delete_from_db();
        if (retval) return retval;
        log_messages.printf(
            SCHED_MSG_LOG::MSG_DEBUG,
            "Purged result [%d] from database\n", result.id
        );
        number_results++;
    }
    return 0;
}

// return true if did anything
//
bool do_pass() {
    int retval= 0;

    // The number of workunits/results purged in a single pass of do_pass().
    // Since do_pass() may be invoked multiple times,
    // corresponding global variables store global totals.
    //
    int do_pass_purged_workunits = 0;
    int do_pass_purged_results = 0;

    // check to see if we got a stop signal.
    // Note that if we do catch a stop signal here,
    // we call an exit handler that closes [and optionally compresses] files
    // before returning to the OS.
    //
    check_stop_daemons();

    bool did_something = false;
    DB_WORKUNIT wu;
    char buf[256];

    if (min_age_days) {
        char timestamp[15];
        mysql_timestamp(dtime()-min_age_days*86400, timestamp);
        sprintf(buf,
            "where file_delete_state=%d and mod_time<'%s' limit %d",
            FILE_DELETE_DONE, timestamp, DB_QUERY_LIMIT
        );
    } else {
        sprintf(buf,
            "where file_delete_state=%d limit %d",
            FILE_DELETE_DONE, DB_QUERY_LIMIT
        );
    }

    int n=0;
    while (!wu.enumerate(buf)) {
        if (strstr(wu.name, "nodelete")) continue;
        did_something = true;
        
        // if archives have not already been opened, then open them.
        if (!wu_stream) {
            open_all_archives();
        }

        retval = purge_and_archive_results(wu, n);
        do_pass_purged_results += n;

        retval= archive_wu(wu);
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "Failed to write to XML file workunit:%d\n", wu.id
            );
            exit(5);
        }
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "Archived workunit [%d] to a file\n", wu.id
        );

        //purge workunit from DB
        retval= wu.delete_from_db();
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "Can't delete workunit [%d] from database:%d\n", wu.id, retval
            );
            exit(6);
        }
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "Purged workunit [%d] from database\n", wu.id
        );

        purged_workunits++;
        do_pass_purged_workunits++;
        wu_stored_in_file++;

        // flush the various output files.
        fflush(NULL);

        // if file has got max # of workunits, close and compress it.
        // This sets file pointers to NULL
        if (max_wu_per_file && wu_stored_in_file>=max_wu_per_file) {
            close_all_archives();
            wu_stored_in_file=0;
        }

        if (time_to_quit()) {
            break;
        }

    }

    if (do_pass_purged_workunits) {
        log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
            "Archived %d workunits and %d results\n",
            do_pass_purged_workunits,do_pass_purged_results
        );
    } 

    if (did_something && wu_stored_in_file>0) {
        log_messages.printf(SCHED_MSG_LOG::MSG_DEBUG,
            "Currently open archive files contain %d workunits\n",
            wu_stored_in_file);
    }

    if (do_pass_purged_workunits > DB_QUERY_LIMIT/2) {
	return true;
    } else {
       	return false;
    }
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
        } else if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-min_age_days")) {
            min_age_days = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-max")) {
            max_number_workunits_to_purge= atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-zip")) {
            compression_type=COMPRESSION_ZIP;
        } else if (!strcmp(argv[i], "-gzip")) {
            compression_type=COMPRESSION_GZIP;
        } else if (!strcmp(argv[i], "-max_wu_per_file")) {
            max_wu_per_file = atoi(argv[++i]);
        } else {
            log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
                "Unrecognized arg: %s\n", argv[i]
            );
            exit(1);
        }
    }

    retval = config.parse_file("..");
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL,
            "Can't parse config file\n"
        );
        exit(1);
    }

    if (asynch) {
        if (fork()) {
            exit(0);
        }
    }

    log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL, "Starting\n");

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::MSG_CRITICAL, "Can't open DB\n");
        exit(2);
    }
    install_stop_signal_handler();
    mkdir("../archives", 0777);

    // on exit, either via the check_stop_daemons signal handler, or
    // through a regular call to exit, these functions will be called
    // in the opposite order of registration.
    //
    atexit(close_db_exit_handler);
    atexit(close_all_archives);

    while (1) {
        if (time_to_quit()) {
            break;
        }
        if (!do_pass()) {
            if (one_pass) break;
            log_messages.printf(SCHED_MSG_LOG::MSG_NORMAL,
            	"Sleeping....\n"
            );
	    sleep(600);
        }
    }

    // files and database are closed by exit handler
    exit(0);
}

const char *BOINC_RCSID_0c1c4336f1 = "$Id$";
