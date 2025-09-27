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

// db_purge [options]
// options are listed in usage() below
//
// purge (delete) workunit and result DB records that are no longer needed.
// Purging a WU means optionally writing it and its results
// to XML archive files,
// then deleting it and the results from the DB.
//
// if --retired_wus:
//      purge WUs for which file_delete_state=FILE_DELETE_DONE
//      and the WU is in a retired batch
// otherwise
//      purge WUs for which file_delete_state=FILE_DELETE_DONE.
//
// The XML files have names of the form
// wu_archive_TIME and result_archive_TIME
// where TIME is the time it was created.
// In addition, generate index files associating each WU and result ID
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
#include <string.h>
#include "zlib.h"

using std::string;

#include "boinc_db.h"
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "util.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "svn_version.h"

#include "error_numbers.h"
#include "str_util.h"

void usage() {
    fprintf(stderr,
        "Purge workunit and result records that are no longer needed.\n\n"
        "Usage: db_purge [options]\n"
        "   -d N or --debug_level N     Set verbosity level (1-4; 3=normal, 4=debug)\n"
        "   --retired_wus               purge WUs only from retired batches\n"
        "   --min_age_days N            Purge Wus w/ mod time at least N days ago\n"
        "   --max N                     Purge at most N WUs\n"
        "   --zip                       Compress output files by piping through zip\n"
        "   --gzip                      Compress output files by piping through gzip\n"
        "   --zlib                      Compress output files using zlib\n"
        "   --no_archive                Don't write output files, just purge\n"
        "   --daily_dir                 Write archives in a new directory each day\n"
        "   --max_wu_per_file N         Write at most N WUs per output file\n"
        "   --sleep N                   Sleep N sec after DB scan\n"
        "   --one_pass                  Make one DB scan, then exit\n"
        "   --dont_delete               Don't actually delete anything from the DB (for testing)\n"
        "   --mod M R                   Handle only WUs with ID mod M == R\n"
        "   -h or --help                Show this help text\n"
        "   -v or --version             Show version information\n"
    );
}

#define WU_FILENAME_PREFIX              "wu_archive"
#define RESULT_FILENAME_PREFIX          "result_archive"
#define WU_INDEX_FILENAME_PREFIX        "wu_index"
#define RESULT_INDEX_FILENAME_PREFIX    "result_index"

#define DB_QUERY_LIMIT                  1000

#define COMPRESSION_NONE    0
#define COMPRESSION_GZIP    1
#define COMPRESSION_ZIP     2
#define COMPRESSION_ZLIB    3

#define WU_ARCHIVE_DATA \
        "<workunit_archive>\n" \
        "  <id>%lu</id>\n" \
        "  <create_time>%d</create_time>\n" \
        "  <appid>%lu</appid>\n" \
        "  <name>%s</name>\n" \
        "  <xml_doc>%s</xml_doc>\n" \
        "  <batch>%d</batch>\n" \
        "  <rsc_fpops_est>%.15e</rsc_fpops_est>\n" \
        "  <rsc_fpops_bound>%.15e</rsc_fpops_bound>\n" \
        "  <rsc_memory_bound>%.15e</rsc_memory_bound>\n" \
        "  <rsc_disk_bound>%.15e</rsc_disk_bound>\n" \
        "  <need_validate>%d</need_validate>\n" \
        "  <canonical_resultid>%lu</canonical_resultid>\n" \
        "  <canonical_credit>%.15e</canonical_credit>\n" \
        "  <transition_time>%d</transition_time>\n" \
        "  <delay_bound>%d</delay_bound>\n" \
        "  <error_mask>%d</error_mask>\n" \
        "  <file_delete_state>%d</file_delete_state>\n" \
        "  <assimilate_state>%d</assimilate_state>\n" \
        "  <hr_class>%d</hr_class>\n" \
        "  <opaque>%f</opaque>\n" \
        "  <min_quorum>%d</min_quorum>\n" \
        "  <target_nresults>%d</target_nresults>\n" \
        "  <max_error_results>%d</max_error_results>\n" \
        "  <max_total_results>%d</max_total_results>\n" \
        "  <max_success_results>%d</max_success_results>\n" \
        "  <result_template_file>%s</result_template_file>\n" \
        "  <priority>%d</priority>\n" \
        "  <mod_time>%s</mod_time>\n" \
        "</workunit_archive>\n", \
        wu.id, \
        wu.create_time, \
        wu.appid, \
        wu.name, \
        wu.xml_doc, \
        wu.batch, \
        wu.rsc_fpops_est, \
        wu.rsc_fpops_bound, \
        wu.rsc_memory_bound, \
        wu.rsc_disk_bound, \
        wu.need_validate, \
        wu.canonical_resultid, \
        wu.canonical_credit, \
        wu.transition_time, \
        wu.delay_bound, \
        wu.error_mask, \
        wu.file_delete_state, \
        wu.assimilate_state, \
        wu.hr_class, \
        wu.opaque, \
        wu.min_quorum, \
        wu.target_nresults, \
        wu.max_error_results, \
        wu.max_total_results, \
        wu.max_success_results, \
        wu.result_template_file, \
        wu.priority, \
        wu.mod_time

#define RESULT_ARCHIVE_DATA \
        "<result_archive>\n" \
        "  <id>%lu</id>\n" \
        "  <create_time>%d</create_time>\n" \
        "  <workunitid>%lu</workunitid>\n" \
        "  <server_state>%d</server_state>\n" \
        "  <outcome>%d</outcome>\n" \
        "  <client_state>%d</client_state>\n" \
        "  <hostid>%lu</hostid>\n" \
        "  <userid>%lu</userid>\n" \
        "  <report_deadline>%d</report_deadline>\n" \
        "  <sent_time>%d</sent_time>\n" \
        "  <received_time>%d</received_time>\n" \
        "  <name>%s</name>\n" \
        "  <cpu_time>%.15e</cpu_time>\n" \
        "  <xml_doc_in>%s</xml_doc_in>\n" \
        "  <xml_doc_out>%s</xml_doc_out>\n" \
        "  <stderr_out>%s</stderr_out>\n" \
        "  <batch>%d</batch>\n" \
        "  <file_delete_state>%d</file_delete_state>\n" \
        "  <validate_state>%d</validate_state>\n" \
        "  <claimed_credit>%.15e</claimed_credit>\n" \
        "  <granted_credit>%.15e</granted_credit>\n" \
        "  <opaque>%f</opaque>\n" \
        "  <random>%d</random>\n" \
        "  <app_version_num>%d</app_version_num>\n" \
        "  <app_version_id>%lu</app_version_id>\n" \
        "  <appid>%lu</appid>\n" \
        "  <exit_status>%d</exit_status>\n" \
        "  <teamid>%lu</teamid>\n" \
        "  <priority>%d</priority>\n" \
        "  <mod_time>%s</mod_time>\n" \
        "</result_archive>\n", \
        result.id, \
        result.create_time, \
        result.workunitid, \
        result.server_state, \
        result.outcome, \
        result.client_state, \
        result.hostid, \
        result.userid, \
        result.report_deadline, \
        result.sent_time, \
        result.received_time, \
        result.name, \
        result.cpu_time, \
        result.xml_doc_in, \
        result.xml_doc_out, \
        stderr_out_escaped, \
        result.batch, \
        result.file_delete_state, \
        result.validate_state, \
        result.claimed_credit, \
        result.granted_credit, \
        result.opaque, \
        result.random, \
        result.app_version_num, \
        result.app_version_id, \
        result.appid, \
        result.exit_status, \
        result.teamid, \
        result.priority, \
        result.mod_time

// will be FILE* or gzFile, depending on compression_type
//
void* wu_stream=NULL;
void* re_stream=NULL;
void* wu_index_stream=NULL;
void* re_index_stream=NULL;

int time_int=0;
double min_age_days = 0;
bool no_archive = false;
bool dont_delete = false;
bool daily_dir = false;
bool retired_wus = false;
int max_number_workunits_to_purge = 0;
    // If nonzero, maximum number of workunits to purge.
    // Since all results associated with a purged workunit are also purged,
    // this also limits the number of purged results.
int purged_workunits = 0;
    // # of WUs purged so far
const char *suffix[4] = {"", ".gz", ".zip", ".gz"};
    // subscripts MUST be in agreement with defines above
int compression_type = COMPRESSION_NONE;
int max_wu_per_file = 0;
    // set on command line if archive files should be closed and re-opened
    // after getting some max no of WU in the file
int wu_stored_in_file = 0;
    // keep track of how many WU archived in file so far
int id_modulus=0, id_remainder=0;
    // allow more than one to run - doesn't work if archiving is enabled
char app_name[256];
DB_APP app;
string retired_batch_ids;
    // if --retired_wus, a list of retired batch IDs as "id1,id2,..."

bool time_to_quit() {
    if (max_number_workunits_to_purge) {
        if (purged_workunits >= max_number_workunits_to_purge) return true;
    }
    return false;
}

void fail(const char* msg) {
    perror("perror: ");
    log_messages.printf(MSG_CRITICAL, "%s", msg);
    exit(1);
}

// Open an archive.
// If the user has asked for compression,
// then we popen(2) a pipe to gzip or zip.
// This does 'in place' compression.
//
void open_archive(const char* filename_prefix, void*& f){
    char path[MAXPATHLEN];
    char command[MAXPATHLEN+512];
    sprintf(command, "/bin/false");

    if (daily_dir) {
        time_t time_time = time_int;
        char dirname[32];
        strftime(dirname, sizeof(dirname), "%Y_%m_%d", gmtime(&time_time));
        safe_strcpy(path, config.project_path("archives/%s",dirname));
        if (mkdir(path,0775)) {
            if(errno!=EEXIST) {
                char errstr[256];
                sprintf(errstr, "could not create directory '%s': %s\n",
                path, strerror(errno));
                fail(errstr);
            }
        }
        safe_strcpy(path,
            config.project_path(
                "archives/%s/%s_%d.xml", dirname, filename_prefix, time_int
            )
        );
    } else {
        safe_strcpy(path,
            config.project_path("archives/%s_%d.xml", filename_prefix, time_int)
        );
    }
    // append appropriate suffix for file type
    safe_strcat(path, suffix[compression_type]);

    // and construct appropriate command if needed
    if (compression_type == COMPRESSION_GZIP) {
        snprintf(command, sizeof(command), "gzip - > %s", path);
    }

    if (compression_type == COMPRESSION_ZIP) {
        snprintf(command, sizeof(command), "zip - - > %s", path);
    }

    log_messages.printf(MSG_NORMAL, "Opening archive %s\n", path);

    if (compression_type == COMPRESSION_NONE) {
        if (!(f = fopen(path,"w"))) {
            char buf[256];
            sprintf(buf, "Can't open archive file %s %s\n",
                path, errno?strerror(errno):""
            );
            fail(buf);
        }
    } else if (compression_type == COMPRESSION_ZLIB) {
        f = gzopen(path,"w");
        if (!f) {
            log_messages.printf(MSG_CRITICAL,
                "Can't open file %s: %d:%s\n",
                path, errno, strerror(errno)
            );
            exit(4);
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

    if (compression_type != COMPRESSION_ZLIB) {
        //
        // set buffering to line buffered, since we are outputing XML on a
        // line-by-line basis.
        //
        setlinebuf((FILE*)f);
    }

    return;
}

void close_archive(const char *filename, void*& fp){
    char path[MAXPATHLEN];

    // Set file pointer to NULL after closing file to indicate that it's closed.
    //
    if (!fp) return;

    // In case of errors, carry on anyway.  This is deliberate, not lazy
    //
    if (compression_type == COMPRESSION_NONE) {
        fclose((FILE*)fp);
    } else if (compression_type == COMPRESSION_ZLIB) {
        gzclose((gzFile)fp);
    } else {
        pclose((FILE*)fp);
    }

    fp = NULL;

    // reconstruct the filename
    if (daily_dir) {
        time_t time_time = time_int;
        char dirname[32];
        strftime(dirname, sizeof(dirname), "%Y_%m_%d", gmtime(&time_time));
        safe_strcpy(path,
            config.project_path(
                  "archives/%s/%s_%d.xml", dirname, filename, time_int
            )
        );
    } else {
        safe_strcpy(path,
            config.project_path("archives/%s_%d.xml", filename, time_int)
        );
    }
    // append appropriate file type
    safe_strcat(path, suffix[compression_type]);

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
    if (compression_type == COMPRESSION_ZLIB) {
        gzprintf((gzFile)wu_stream, "<archive>\n");
        gzprintf((gzFile)re_stream, "<archive>\n");
    } else {
        fprintf((FILE*)wu_stream, "<archive>\n");
        fprintf((FILE*)re_stream, "<archive>\n");
    }
    return;
}

// closes (and optionally compresses) the archive files.  Clears file
// pointers to indicate that files are not open.
//
void close_all_archives() {
    if (compression_type == COMPRESSION_ZLIB) {
        if (wu_stream) gzprintf((gzFile)wu_stream, "</archive>\n");
        if (re_stream) gzprintf((gzFile)re_stream, "</archive>\n");
    } else {
        if (wu_stream) fprintf((FILE*)wu_stream, "</archive>\n");
        if (re_stream) fprintf((FILE*)re_stream, "</archive>\n");
    }
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

    // xml_escape can increase size by factor of 6, e.g. x -> &#NNN;
    //
    char stderr_out_escaped[BLOB_SIZE*6];
    xml_escape(result.stderr_out, stderr_out_escaped, sizeof(stderr_out_escaped));

    n = fprintf((FILE*)re_stream, RESULT_ARCHIVE_DATA);

    if (n >= 0) {
        n = fprintf((FILE*)re_index_stream,
            "%lu     %d    %s\n",
            result.id, time_int, result.name
        );
    }

    if (n < 0) fail("fprintf() failed\n");

    return 0;
}

int archive_wu(DB_WORKUNIT& wu) {
    int n = fprintf((FILE*)wu_stream, WU_ARCHIVE_DATA);
    if (n < 0) fail("archive_wu: data fprintf() failed\n");

    n = fprintf((FILE*)wu_index_stream,
        "%lu     %d    %s\n",
        wu.id, time_int, wu.name
    );

    if (n < 0) fail("archive_wu: index fprintf() failed\n");

    return 0;
}

int archive_result_gz (DB_RESULT& result) {
    int n;
    char buf[BLOB_SIZE*7];

    // xml_escape can increase size by factor of 6, e.g. x -> &#NNN;
    //
    char stderr_out_escaped[BLOB_SIZE*6];
    xml_escape(result.stderr_out, stderr_out_escaped, sizeof(stderr_out_escaped));

    n = snprintf(buf, sizeof(buf), RESULT_ARCHIVE_DATA);
    if ((n <= 0) || n > (int)sizeof(buf)) {
        fail("ERROR: printing result archive failed\n");
    }
    n = gzwrite((gzFile)re_stream, buf, (unsigned int)n);
    if (n <= 0) {
        fail("ERROR: writing result archive failed\n");
    }

    n = gzflush((gzFile)re_stream, Z_SYNC_FLUSH);
    if (n != Z_OK) {
        fail("ERROR: writing result archive failed (flush)\n");
    }

    n = gzprintf((gzFile)re_index_stream,
        "%lu     %d    %s\n",
        result.id, time_int, result.name
    );
    if (n <= 0) {
        fail("ERROR: writing result index failed\n");
    }

    n = gzflush((gzFile)re_index_stream, Z_SYNC_FLUSH);
    if (n != Z_OK) {
        fail("ERROR: writing result index failed (flush)\n");
    }

    return 0;
}

int archive_wu_gz (DB_WORKUNIT& wu) {
    int n;
    char buf[BLOB_SIZE*2];

    n = snprintf(buf, sizeof(buf), WU_ARCHIVE_DATA);
    if ((n <= 0) || n > (int)sizeof(buf)) {
        fail("ERROR: printing workunit archive failed\n");
    }
    n = gzwrite((gzFile)wu_stream, buf, (unsigned int)n);
    if (n <= 0) {
        fail("ERROR: writing workunit archive failed\n");
    }

    n = gzflush((gzFile)wu_stream,Z_SYNC_FLUSH);
    if (n != Z_OK) {
        fail("ERROR: writing workunit archive failed (flush)\n");
    }

    n = gzprintf((gzFile)wu_index_stream,
        "%lu     %d    %s\n",
        wu.id, time_int, wu.name
    );
    if (n <= 0) {
        fail("ERROR: writing workunit index failed\n");
    }

    n = gzflush((gzFile)wu_index_stream,Z_SYNC_FLUSH);
    if (n != Z_OK) {
        fail("ERROR: writing workunit index failed (flush)\n");
    }

    return 0;
}

int purge_and_archive_results(DB_WORKUNIT& wu, int& number_results) {
    int retval= 0;
    DB_RESULT result;
    char buf[256];

    number_results=0;

    sprintf(buf, "where workunitid=%lu", wu.id);
    while (!result.enumerate(buf)) {
        if (!no_archive) {
            if (compression_type == COMPRESSION_ZLIB) {
                retval = archive_result_gz(result);
            } else {
                retval = archive_result(result);
            }
            if (retval) return retval;
            log_messages.printf(MSG_DEBUG,
                "Archived result [%lu] to a file\n", result.id
            );
        }
        if (dont_delete) {
            log_messages.printf(MSG_DEBUG,
                "Didn't purge result [%lu] from database (-dont_delete)\n", result.id
            );
        } else {
            retval = result.delete_from_db();
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "Couldn't delete result [%lu] from database\n", result.id
                );
                return retval;
            }
            log_messages.printf(MSG_NORMAL,
                "Purged result [%lu] batch %d\n", result.id, result.batch
            );
        }
        number_results++;
    }
    return 0;
}

// get list of IDs of retired batches
//
int get_retired_batch_ids(string &out) {
    out.clear();
    char query[256];
    sprintf(query, "select id from batch where state=%d", BATCH_STATE_RETIRED);
    int retval = boinc_db.do_query(query);
    if (retval) return mysql_errno(boinc_db.mysql);

    MYSQL_RES *rp;
    rp = mysql_store_result(boinc_db.mysql);
    if (!rp) return mysql_errno(boinc_db.mysql);
    bool first = true;
    while (1) {
        MYSQL_ROW row = mysql_fetch_row(rp);
        if (!row) {
            mysql_free_result(rp);
            break;
        }
        if (first) {
            first = false;
        } else {
            out += ",";
        }
        out += row[0];
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
    int min_age_seconds = 0;

    // check to see if we got a stop signal.
    // Note that if we do catch a stop signal here,
    // we call an exit handler that closes [and optionally compresses] files
    // before returning to the OS.
    //
    check_stop_daemons();

    bool did_something = false;
    DB_WORKUNIT wu;
    char buf[256];
    string clause;

    sprintf(buf, "where file_delete_state=%d", FILE_DELETE_DONE);
    clause = buf;
    if (min_age_days) {
        min_age_seconds = (int) (min_age_days*86400);
        sprintf(buf,
            " and mod_time<current_timestamp() - interval %d second",
            min_age_seconds
        );
        clause += buf;
    }
    if (id_modulus) {
        sprintf(buf, " and id %% %d = %d", id_modulus, id_remainder);
        clause += buf;
    }
    if (strlen(app_name)) {
        sprintf(buf, " and appid=%lu", app.id);
        clause += buf;
    }
    if (retired_wus) {
        clause += " and batch in (";
        clause += retired_batch_ids;
        clause += ")";
    } else {
        clause += " and batch=0";
    }

    sprintf(buf, " limit %d", DB_QUERY_LIMIT);
    clause += buf;

    int n=0;
    while (1) {
        retval = wu.enumerate(clause.c_str());
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_CRITICAL,
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
        // if a result fails to be deleted, don't purge this workunit,
        // or this result will be left orphaned and never get deleted
        if (retval) continue;

        do_pass_purged_results += n;

        if (!no_archive) {
            if (compression_type == COMPRESSION_ZLIB) {
                retval= archive_wu_gz(wu);
            } else {
                retval= archive_wu(wu);
            }
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "Failed to write to XML file workunit:%lu\n", wu.id
                );
                exit(5);
            }
            log_messages.printf(MSG_DEBUG,
                "Archived workunit [%lu] to a file\n", wu.id
            );
        }

        // purge workunit from DB
        //
        if (dont_delete) {
            log_messages.printf(MSG_DEBUG,
                "Didn't purge workunit [%lu] from database (--dont_delete)\n", wu.id
            );
        } else {
            retval= wu.delete_from_db();
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "Can't delete workunit [%lu] from database: %d\n",
                    wu.id, retval
                );
                exit(6);
            }
            if (config.enable_assignment) {
                DB_ASSIGNMENT asg;
                sprintf(buf, "workunitid=%lu", wu.id);
                asg.delete_from_db_multi(buf);
            }
            log_messages.printf(MSG_NORMAL,
                "Purged workunit [%lu] batch %d\n", wu.id, wu.batch
            );
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

int main(int argc, char** argv) {
    int retval;
    bool one_pass = false;
    int i;
    int sleep_sec = 300;
    check_stop_daemons();
    char buf[256];

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "one_pass")) {
            one_pass = true;
        } else if (is_arg(argv[i], "dont_delete")) {
            dont_delete = true;
        } else if (is_arg(argv[i], "d") || is_arg(argv[i], "debug_level")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage();
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl >= MSG_DEBUG) {
                g_print_queries = true;
            }
        } else if (is_arg(argv[i], "min_age_days")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage();
                exit(1);
            }
            min_age_days = atof(argv[i]);
        } else if (is_arg(argv[i], "max")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage();
                exit(1);
            }
            max_number_workunits_to_purge= atoi(argv[i]);
        } else if (is_arg(argv[i], "daily_dir")) {
            daily_dir = true;
        } else if (is_arg(argv[i], "zip")) {
            compression_type=COMPRESSION_ZIP;
        } else if (is_arg(argv[i], "gzip")) {
            compression_type=COMPRESSION_GZIP;
        } else if (is_arg(argv[i], "zlib")) {
            compression_type=COMPRESSION_ZLIB;
        } else if (is_arg(argv[i], "max_wu_per_file")) {
            if(!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage();
                exit(1);
            }
            max_wu_per_file = atoi(argv[i]);
        } else if (is_arg(argv[i], "no_archive")) {
            no_archive = true;
        } else if (is_arg(argv[i], "retired_wus")) {
            retired_wus = true;
        } else if (is_arg(argv[i], "sleep")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage();
                exit(1);
            }
            sleep_sec = atoi(argv[i]);
            if (sleep_sec < 1 || sleep_sec > 86400) {
                log_messages.printf(MSG_CRITICAL,
                    "Unreasonable value of sleep interval: %d seconds\n",
                    sleep_sec
                );
                usage();
                exit(1);
            }
        } else if (is_arg(argv[i], "--help") || is_arg(argv[i], "-h")) {
            usage();
            return 0;
        } else if (is_arg(argv[i], "--version") || is_arg(argv[i], "-v")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else if (is_arg(argv[i], "mod")) {
            if (!argv[i+1] || !argv[i+2]) {
                log_messages.printf(MSG_CRITICAL,
                    "%s requires two arguments\n\n", argv[i]
                );
                usage();
                exit(1);
            }
            id_modulus   = atoi(argv[++i]);
            id_remainder = atoi(argv[++i]);
        } else if (is_arg(argv[i], "app")) {
            safe_strcpy(app_name, argv[++i]);
        } else {
            log_messages.printf(MSG_CRITICAL,
                "unknown command line argument: %s\n\n", argv[i]
            );
            usage();
            exit(1);
        }
    }

    if (id_modulus && !no_archive) {
        log_messages.printf(MSG_CRITICAL,
            "If you use modulus, you must set no_archive\n\n"
        );
        usage();
        exit(1);
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
        log_messages.printf(MSG_CRITICAL, "Can't open DB: %s\n",
            boinc_db.error_string()
        );
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

    if (strlen(app_name)) {
        sprintf(buf, "where name='%s'", app_name);
        retval = app.lookup(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "Can't find app %s\n", app_name);
            exit(1);
        }
    }

    while (1) {
        if (time_to_quit()) {
            break;
        }
        if (retired_wus) {
            retval = get_retired_batch_ids(retired_batch_ids);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "Can't get retired batch IDs"
                );
                exit(1);
            }
            if (retired_batch_ids.empty()) {
                log_messages.printf(MSG_NORMAL,
                    "no retired batches; sleeping %d\n",
                    sleep_sec
                );
                daemon_sleep(sleep_sec);
                continue;
            }
        }

        bool did_something = do_pass();
        if (one_pass) {
            break;
        }
        if (!did_something) {
            log_messages.printf(MSG_NORMAL,
                "No WUs to purge; sleeping %d\n",
                sleep_sec
            );
            daemon_sleep(sleep_sec);
        }
    }

    // files and database are closed by exit handler
    exit(0);
}
