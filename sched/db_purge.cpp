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

// db_purge options
//
// purge workunit and result records that are no longer needed.
// Specifically, purges WUs for which file_delete_state=DONE.
// Purging a WU means writing it and all its results
// to XML-format archive files, then deleting it and its results from the DB.
//
// The XML files have names of the form
// wu_archive_TIME and result_archive_TIME
// where TIME is the time it was created.
// In addition there are index files associating each WU and result ID
// with the timestamp of the file it's in.

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

#include "boinc_db.h"
#include "util.h"
#include "filesys.h"
#include "parse.h"
#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "svn_version.h"

#include "error_numbers.h"
#include "str_util.h"

#define WU_FILENAME_PREFIX              "wu_archive"
#define RESULT_FILENAME_PREFIX          "result_archive"
#define WU_INDEX_FILENAME_PREFIX        "wu_index"
#define RESULT_INDEX_FILENAME_PREFIX    "result_index"

#define DB_QUERY_LIMIT                  1000

#define COMPRESSION_NONE    0
#define COMPRESSION_GZIP    1
#define COMPRESSION_ZIP     2

FILE *wu_stream=NULL;
FILE *re_stream=NULL;
FILE *wu_index_stream=NULL;
FILE *re_index_stream=NULL;
int time_int=0;
double min_age_days = 0;
bool no_archive = false;
bool dont_delete = false;
bool daily_dir = false;
int purged_workunits = 0;
    // used if limiting the total number of workunits to eliminate
int max_number_workunits_to_purge = 0;
    // If nonzero, maximum number of workunits to purge.
    // Since all results associated with a purged workunit are also purged,
    // this also limits the number of purged results.
const char *suffix[3] = {"", ".gz", ".zip"};
    // subscripts MUST be in agreement with defines above
int compression_type = COMPRESSION_NONE;
int max_wu_per_file = 0;
    // set on command line if archive files should be closed and re-opened
    // after getting some max no of WU in the file
int wu_stored_in_file = 0;
    // keep track of how many WU archived in file so far

bool time_to_quit() {
    if (max_number_workunits_to_purge) {
        if (purged_workunits >= max_number_workunits_to_purge) return true;
    }
    return false;
}

void fail(const char* msg) {
    log_messages.printf(MSG_CRITICAL, msg);
    exit(1);
}

// Open an archive.
// If the user has asked for compression,
// then we popen(2) a pipe to gzip or zip.
// This does 'in place' compression.
//
void open_archive(const char* filename_prefix, FILE*& f){
    char path[256];
    char command[512];

    if (daily_dir) {
        time_t time_time = time_int;
        char dirname[32];
        strftime(dirname, sizeof(dirname), "%Y_%m_%d", gmtime(&time_time));
        strcpy(path, config.project_path("archives/%s",dirname));
        if (mkdir(path,0775)) {
            if(errno!=EEXIST) {
                char errstr[256];
                sprintf(errstr, "could not create directory '%s': %s\n",
                path, strerror(errno));
                fail(errstr);
            }
        }
        strcpy(path,
            config.project_path(
                "archives/%s/%s_%d.xml", dirname, filename_prefix, time_int
            )
        );
    } else {
        strcpy(path,
            config.project_path("archives/%s_%d.xml", filename_prefix, time_int)
        );
    }
    // append appropriate suffix for file type
    strcat(path, suffix[compression_type]);

    // and construct appropriate command if needed
    if (compression_type == COMPRESSION_GZIP) {
        sprintf(command, "gzip - > %s", path);
    }
   
    if (compression_type == COMPRESSION_ZIP) {
        sprintf(command, "zip - - > %s", path);
    }

    log_messages.printf(MSG_NORMAL,
        "Opening archive %s\n", path
    );

    if (compression_type == COMPRESSION_NONE) {   
        if (!(f = fopen(path,"w"))) {
            char buf[256];
            sprintf(buf, "Can't open archive file %s %s\n",
                path, errno?strerror(errno):""
            );
            fail(buf);
        }
    } else {
        f = popen(command,"w");
        if (!f) {
            log_messages.printf(MSG_CRITICAL,
                "Can't open pipe %s %s\n", 
                command, errno?strerror(errno):""
            );
            exit(4);
        }
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
    if (compression_type == COMPRESSION_NONE) {
        fclose(fp);
    } else {
        pclose(fp);
    }
    
    fp = NULL;

    // reconstruct the filename
    if (daily_dir) {
        time_t time_time = time_int;
        char dirname[32];
        strftime(dirname, sizeof(dirname), "%Y_%m_%d", gmtime(&time_time));
        strcpy(path,
            config.project_path(
                  "archives/%s/%s_%d.xml", dirname, filename, time_int
            )
        );
    } else {
        strcpy(path,
            config.project_path("archives/%s_%d.xml", filename, time_int)
        );
    }
    // append appropriate file type
    strcat(path, suffix[compression_type]);
    
    log_messages.printf(MSG_NORMAL,
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
    if (wu_stream) fprintf(wu_stream, "</archive>\n");
    if (re_stream) fprintf(re_stream, "</archive>\n");
    close_archive(WU_FILENAME_PREFIX, wu_stream);
    close_archive(RESULT_FILENAME_PREFIX, re_stream);
    close_archive(RESULT_INDEX_FILENAME_PREFIX, re_index_stream);
    close_archive(WU_INDEX_FILENAME_PREFIX, wu_index_stream);
    log_messages.printf(MSG_NORMAL,
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
    int n;
    n = fprintf(re_stream,
        "<result_archive>\n"
        "    <id>%d</id>\n",
        result.id
    );

    // xml_escape can increase size by factor of 6, e.g. x -> &#NNN;
    //
    char buf[BLOB_SIZE*6];
    xml_escape(result.stderr_out, buf, sizeof(buf));

    if (n >= 0) n = fprintf(
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
        buf,
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

    if (n >= 0) n = fprintf(re_stream,
        "</result_archive>\n"
    );

    if (n >= 0) {
        n = fprintf(re_index_stream,
            "%d     %d    %s\n",
            result.id, time_int, result.name
        );
    }

    if (n < 0) fail("fprintf() failed\n");

    return 0;
}

int archive_wu(DB_WORKUNIT& wu) {
    int n;
    n = fprintf(wu_stream,
        "<workunit_archive>\n"
        "    <id>%d</id>\n",
        wu.id
    );
    if (n >= 0) n = fprintf(wu_stream,
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
        "  <canonical_resultid>%u</canonical_resultid>\n"
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

    if (n >= 0) n = fprintf(wu_stream,
        "</workunit_archive>\n"
    );

    if (n >= 0) {
        n = fprintf(wu_index_stream,
            "%d     %d    %s\n",
            wu.id, time_int, wu.name
        );
    }

    if (n < 0) fail("fprintf() failed\n");

    return 0;
}

int purge_and_archive_results(DB_WORKUNIT& wu, int& number_results) {
    int retval= 0;
    DB_RESULT result;
    char buf[256];

    number_results=0;

    sprintf(buf, "where workunitid=%d", wu.id);
    while (!result.enumerate(buf)) {
        if (!no_archive) {
            retval = archive_result(result);
            if (retval) return retval;
            log_messages.printf(MSG_DEBUG,
                "Archived result [%d] to a file\n", result.id
            );
        }
        if (!dont_delete) {
            retval = result.delete_from_db();
            if (retval) return retval;
        }
        log_messages.printf(MSG_DEBUG,
            "Purged result [%d] from database\n", result.id
        );
        number_results++;
    }
    return 0;
}

// return true if did anything
//
bool do_pass() {
    int retval = 0;

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
        mysql_timestamp(dtime()-min_age_days*86400., timestamp);
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
    while (1) {
        retval = wu.enumerate(buf);
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_DEBUG,
                    "DB connection lost, exiting\n"
                );
                exit(0);
            }
            break;
        }
        if (strstr(wu.name, "nodelete")) continue;
        did_something = true;
        
        // if archives have not already been opened, then open them.
        //
        if (!no_archive && !wu_stream) {
            open_all_archives();
        }

        retval = purge_and_archive_results(wu, n);
        do_pass_purged_results += n;

        if (!no_archive) {
            retval= archive_wu(wu);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "Failed to write to XML file workunit:%d\n", wu.id
                );
                exit(5);
            }
            log_messages.printf(MSG_DEBUG,
                "Archived workunit [%d] to a file\n", wu.id
            );
        }

        // purge workunit from DB
        //
        if (!dont_delete) {
            retval= wu.delete_from_db();
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "Can't delete workunit [%d] from database:%d\n",
                    wu.id, retval
                );
                exit(6);
            }
        }
        log_messages.printf(MSG_DEBUG,
            "Purged workunit [%d] from database\n", wu.id
        );

        if (config.enable_assignment) {
            DB_ASSIGNMENT asg;
            char buf2[256];
            sprintf(buf, "workunitid=%d", wu.id);
            asg.delete_from_db_multi(buf2);
        }

        purged_workunits++;
        do_pass_purged_workunits++;
        wu_stored_in_file++;

        if (!no_archive) {
            fflush(NULL);

            // if file has got max # of workunits, close and compress it.
            // This sets file pointers to NULL
            //
            if (max_wu_per_file && wu_stored_in_file>=max_wu_per_file) {
                close_all_archives();
                wu_stored_in_file = 0;
            }
        }

        if (time_to_quit()) {
            break;
        }

    }

    if (do_pass_purged_workunits) {
        log_messages.printf(MSG_NORMAL,
            "Archived %d workunits and %d results\n",
            do_pass_purged_workunits, do_pass_purged_results
        );
    } 

    if (did_something && wu_stored_in_file>0) {
        log_messages.printf(MSG_DEBUG,
            "Currently open archive files contain %d workunits\n",
            wu_stored_in_file
        );
    }

    if (do_pass_purged_workunits > DB_QUERY_LIMIT/2) {
        return true;
    } else {
        return false;
    }
}

void usage(char* name) {
    fprintf(stderr,
        "Purge workunit and result records that are no longer needed.\n\n"
        "Usage: %s [options]\n"
        "    [-d N | --debug_level N]      Set verbosity level (1 to 4)\n"
        "    [--min_age_days N]            Purge Wus w/ mod time at least N days ago\n"
        "    [--max N]                     Purge at most N WUs\n"
        "    [--zip]                       Compress output files using zip\n"
        "    [--gzip]                      Compress output files using gzip\n"
        "    [--no_archive]                Don't write output files, just purge\n"
        "    [--daily_dir]                 Write archives in a new directory each day\n"
        "    [--max_wu_per_file N]         Write at most N WUs per output file\n"
        "    [--sleep N]                   Sleep N sec after DB scan\n"
        "    [--one_pass]                  Make one DB scan, then exit\n"
        "    [--dont_delete]               Don't actually delete anything from the DB (for testing only)\n"
        "    [--h | --help]                Show this help text\n"
        "    [--v | --version]             Show version information\n",
        name
    );
}

int main(int argc, char** argv) {
    int retval;
    bool one_pass = false;
    int i;
    int sleep_sec = 600;
    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "one_pass")) {
            one_pass = true;
        } else if (is_arg(argv[i], "dont_delete")) {
            dont_delete = true;
        } else if (is_arg(argv[i], "d") || is_arg(argv[i], "debug_level")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (is_arg(argv[i], "min_age_days")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            min_age_days = atof(argv[i]);
        } else if (is_arg(argv[i], "max")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            max_number_workunits_to_purge= atoi(argv[i]);
        } else if (is_arg(argv[i], "daily_dir")) {
            daily_dir=true;
        } else if (is_arg(argv[i], "zip")) {
            compression_type=COMPRESSION_ZIP;
        } else if (is_arg(argv[i], "gzip")) {
            compression_type=COMPRESSION_GZIP;
        } else if (is_arg(argv[i], "max_wu_per_file")) {
            if(!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            max_wu_per_file = atoi(argv[i]);
        } else if (is_arg(argv[i], "no_archive")) {
            no_archive = true;
        } else if (is_arg(argv[i], "-sleep")) {
            if(!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            sleep_sec = atoi(argv[i]);
            if (sleep_sec < 1 || sleep_sec > 86400) {
                log_messages.printf(MSG_CRITICAL,
                    "Unreasonable value of sleep interval: %d seconds\n",
                    sleep_sec
                );
                usage(argv[0]);
                exit(1);
            }
        } else if (is_arg(argv[i], "--help") || is_arg(argv[i], "-help") || is_arg(argv[i], "-h")) {
            usage(argv[0]);
            return 0;
        } else if (is_arg(argv[i], "--version") || is_arg(argv[i], "-version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL,
                "unknown command line argument: %s\n\n", argv[i]
            );
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

    log_messages.printf(MSG_NORMAL, "Starting\n");

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't open DB\n");
        exit(2);
    }
    install_stop_signal_handler();
    boinc_mkdir(config.project_path("archives"));

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
        if (!do_pass() && !one_pass) {
            log_messages.printf(MSG_NORMAL, "Sleeping....\n");
            sleep(sleep_sec);
        }
        if (one_pass) {
            break;
        }
    }

    // files and database are closed by exit handler
    exit(0);
}

const char *BOINC_RCSID_0c1c4336f1 = "$Id$";
