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


// file deleter.  See usage() below for usage.

// enum sizes.  RESULT_PER_ENUM is three times larger on the
// assumption of 3-fold average redundancy.
// This balances the rate at which input and output files are deleted
//
#define WUS_PER_ENUM        500
#define RESULTS_PER_ENUM    1500

// how often to retry errors
//
#define ERROR_INTERVAL      3600

#include "config.h"
#include <list>
#include <cstring>
#include <string>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_STRINGS_H
#include <strings.h>
#endif

#include "boinc_db.h"
#include "parse.h"
#include "error_numbers.h"
#include "util.h"
#include "str_util.h"
#include "str_replace.h"
#include "filesys.h"
#include "strings.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

#define LOCKFILE "file_deleter.out"
#define PIDFILE  "file_deleter.pid"

#define DEFAULT_SLEEP_INTERVAL 5
#define RESULTS_PER_WU 4        // an estimate of redundancy 

int id_modulus=0, id_remainder=0, appid=0;
bool dont_retry_errors = false;
bool dont_delete_batches = false;
bool do_input_files = true;
bool do_output_files = true;
int sleep_interval = DEFAULT_SLEEP_INTERVAL;
static pthread_t thread_handle;

void usage(char *name) {
    fprintf(stderr, "Deletes files that are no longer needed.\n\n"
        "Default operation:\n"
        "1) enumerate N WUs and M results (N,M compile params)\n"
        "   that are ready to file-delete, and try to delete their files\n"
        "2) if the enums didn't yield anything, sleep for K seconds\n"
        "3) repeat from 1)\n"
        "4) every 1 hour, enumerate everything in state FILE_DELETE_ERROR\n"
        "   and try to delete it.\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  -d N | --debug_level N          set debug output level (1 to 4)\n"
        "  --mod M R                       handle only WUs with ID mod M == R\n"
        "  --appid ID                      handle only WUs of app with id ID\n"
        "  --app NAME                      handle only WUs of app with name NAME\n"
        "  --one_pass                      instead of sleeping in 2), exit\n"
        "  --dont_retry_error              don't do 4)\n"
        "  --preserve_result_files         update the DB, but don't delete output files.\n"
        "                                  For debugging.\n"
        "  --preserve_wu_files             update the DB, but don't delete input files.\n"
        "                                  For debugging.\n"
        "  --dont_delete_batches           don't delete anything with positive batch number\n"
        "  --input_files_only              delete only input (download) files\n"
        "  --output_files_only             delete only output (upload) files\n"
        "  [ -h | --help ]                 shows this help text\n"
        "  [ -v | --version ]              shows version information\n",
        name
    );
}

// Given a filename, find its full path in the upload directory hierarchy
// Return ERR_OPENDIR if dir isn't there (possibly recoverable error),
// ERR_NOT_FOUND if dir is there but not file
//
int get_file_path(
    const char *filename, char* upload_dir, int fanout, char* path
) {
    dir_hier_path(filename, upload_dir, fanout, path, true);
    if (boinc_file_exists(path)) {
        return 0;
    }
    char* p = strrchr(path, '/');
    *p = 0;
    if (boinc_file_exists(path)) {
        return ERR_NOT_FOUND;
    }
    return ERR_OPENDIR;
}


int wu_delete_files(WORKUNIT& wu) {
    char* p;
    char filename[256], pathname[256], buf[BLOB_SIZE];
    bool no_delete=false;
    int count_deleted = 0, retval, mthd_retval = 0;

    if (strstr(wu.name, "nodelete")) return 0;

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
                retval = get_file_path(
                    filename, config.download_dir, config.uldl_dir_fanout,
                    pathname
                );
                if (retval == ERR_OPENDIR) {
                    log_messages.printf(MSG_CRITICAL,
                        "[WU#%d] missing dir for %s\n",
                        wu.id, filename
                    );
                    mthd_retval = ERR_UNLINK;
                } else if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "[WU#%d] get_file_path: %s: %s\n",
                        wu.id, filename, boincerror(retval)
                    );
                } else {
                    log_messages.printf(MSG_NORMAL,
                        "[WU#%d] deleting %s\n", wu.id, filename
                    );
                    retval = unlink(pathname);
                    if (retval) {
                        log_messages.printf(MSG_CRITICAL,
                            "[WU#%d] unlink %s failed: %s\n",
                            wu.id, filename, boincerror(retval)
                        );
                        mthd_retval = ERR_UNLINK;
                    } else {
                        count_deleted++;
                    }
                    // delete the cached MD5 file if needed
                    //
                    if (config.cache_md5_info) {
                        strcat(pathname,".md5");
                        log_messages.printf(MSG_NORMAL,
                            "[WU#%d] deleting %s\n", wu.id, filename
                        );
                        retval = unlink(pathname);
                        if (retval) {
                            log_messages.printf(MSG_CRITICAL,
                                "[WU#%d] unlink %s failed: %s\n",
                                wu.id, filename, boincerror(retval)
                            );
                        }
                    }
                }
            }
        }
        p = strtok(0, "\n");
    }
    log_messages.printf(MSG_DEBUG,
        "[WU#%d] deleted %d file(s)\n", wu.id, count_deleted
    );
    return mthd_retval;
}

int result_delete_files(RESULT& result) {
    char* p;
    char filename[256], pathname[256], buf[BLOB_SIZE];
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
                if (retval == ERR_OPENDIR) {
                    mthd_retval = ERR_OPENDIR;
                    log_messages.printf(MSG_CRITICAL,
                        "[RESULT#%d] missing dir for %s\n",
                        result.id, pathname
                    );
                } else if (retval) {
                    // the fact that no result files were found is a critical
                    // error if this was a successful result, but is to be
                    // expected if the result outcome was failure, since in
                    // that case there may well be no output file produced.
                    //
                    int debug_or_crit;
                    if (RESULT_OUTCOME_SUCCESS == result.outcome) {
                        debug_or_crit=MSG_CRITICAL;
                    } else {
                        debug_or_crit=MSG_DEBUG;
                    }
                    log_messages.printf(debug_or_crit,
                        "[RESULT#%d] outcome=%d client_state=%d No file %s to delete\n",
                        result.id, result.outcome, result.client_state, filename
                    );
                } else {
                    retval = unlink(pathname);
                    if (retval) {
                        mthd_retval = ERR_UNLINK;
                        log_messages.printf(MSG_CRITICAL,
                            "[RESULT#%d] unlink %s error: %s %s\n",
                            result.id, pathname, boincerror(retval),
                            (retval && errno)?strerror(errno):""
                        );
                    } else {
                        count_deleted++;
                        log_messages.printf(MSG_NORMAL,
                            "[RESULT#%d] unlinked %s\n", result.id, pathname
                        );
                    }
                }
            }
        }
        p = strtok(0, "\n");
    }

    log_messages.printf(MSG_DEBUG,
        "[RESULT#%d] deleted %d file(s)\n", result.id, count_deleted
    );
    return mthd_retval;
}

// set by corresponding command line arguments.
//
static bool preserve_wu_files=false;
static bool preserve_result_files=false;

// return true if we changed the file_delete_state of a WU or a result
//
bool do_pass(bool retry_error) {
    DB_WORKUNIT wu;
    DB_RESULT result;
    bool did_something = false;
    char buf[256];
    char clause[256];
    int retval, new_state;

    check_stop_daemons();

    strcpy(clause, "");
    if (id_modulus) {
        sprintf(clause, " and id %% %d = %d ", id_modulus, id_remainder);
    }
    if (dont_delete_batches) {
        strcat(clause, " and batch <= 0 ");
    }
    if (appid) {
        sprintf(buf, " and appid = %d ", appid);
        strcat(clause, buf);
    }
    sprintf(buf,
        "where file_delete_state=%d %s limit %d",
        retry_error?FILE_DELETE_ERROR:FILE_DELETE_READY,
        clause, WUS_PER_ENUM
    );

    while (do_input_files) {
        retval = wu.enumerate(buf);
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_DEBUG, "DB connection lost, exiting\n");
                exit(0);
            }
            break;
        }

        if (preserve_wu_files) {
            retval = 0;
        } else {
            retval = wu_delete_files(wu);
        }
        if (retval) {
            new_state = FILE_DELETE_ERROR;
            log_messages.printf(MSG_CRITICAL,
                "[WU#%d] file deletion failed: %s\n", wu.id, boincerror(retval)
            );
        } else {
            new_state = FILE_DELETE_DONE;
        }
        if (new_state != wu.file_delete_state) {
            sprintf(buf, "file_delete_state=%d", new_state);
            retval = wu.update_field(buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[WU#%d] update failed: %s\n", wu.id, boincerror(retval)
                );
            } else {
                log_messages.printf(MSG_DEBUG,
                    "[WU#%d] file_delete_state updated\n", wu.id
                );
                did_something = true;
            }
        } 
    }

    sprintf(buf,
        "where file_delete_state=%d %s limit %d",
        retry_error?FILE_DELETE_ERROR:FILE_DELETE_READY,
        clause, RESULTS_PER_ENUM
    );

    while (do_output_files) {
        retval = result.enumerate(buf);
        if (retval) {
            if (retval != ERR_DB_NOT_FOUND) {
                log_messages.printf(MSG_DEBUG, "DB connection lost, exiting\n");
                exit(0);
            }
            break;
        }

        if (preserve_result_files) {
            retval = 0;
        } else {
            retval = result_delete_files(result);
        }
        if (retval) {
            new_state = FILE_DELETE_ERROR;
            log_messages.printf(MSG_CRITICAL,
                "[RESULT#%d] file deletion failed: %s\n", result.id, boincerror(retval)
            );
        } else {
            new_state = FILE_DELETE_DONE;
        }
        if (new_state != result.file_delete_state) {
            sprintf(buf, "file_delete_state=%d", new_state); 
            retval = result.update_field(buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "[RESULT#%d] update failed: %s\n", result.id, boincerror(retval)
                );
            } else {
                log_messages.printf(MSG_DEBUG,
                    "[RESULT#%d] file_delete_state updated\n", result.id
                );
                did_something = true;
            }
        } 
    } 

    return did_something;
}

struct FILE_RECORD {
     std::string name;
     int date_modified;
};

bool operator == (const FILE_RECORD& fr1, const FILE_RECORD& fr2) {
    return (fr1.date_modified == fr2.date_modified && fr1.name == fr2.name);
}

bool operator < (const FILE_RECORD& fr1, const FILE_RECORD& fr2) {
    if (fr1.date_modified < fr2.date_modified) return true;
    if (fr1.date_modified > fr2.date_modified) return false;
    if (fr1.name < fr2.name) return true;
    return false;
}


int main(int argc, char** argv) {
    int retval;
    bool one_pass = false;
    int i;
    DB_APP app;
    
    check_stop_daemons();

    *app.name='\0';
    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "one_pass")) {
            one_pass = true;
        } else if (is_arg(argv[i], "dont_retry_errors")) {
            dont_retry_errors = true;
        } else if (is_arg(argv[i], "preserve_wu_files")) {
            preserve_wu_files = true;
        } else if (is_arg(argv[i], "preserve_result_files")) {
            preserve_result_files = true;
        } else if (is_arg(argv[i], "app")) {
            strcpy(app.name, argv[++i]);
        } else if (is_arg(argv[i], "appid")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            appid = atoi(argv[i]);
        } else if (is_arg(argv[i], "d") || is_arg(argv[i], "debug_level")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (is_arg(argv[i], "mod")) {
            if (!argv[i+1] || !argv[i+2]) {
                log_messages.printf(MSG_CRITICAL, "%s requires two arguments\n\n", argv[i]);
                usage(argv[0]);
                exit(1);
            }
            id_modulus   = atoi(argv[++i]);
            id_remainder = atoi(argv[++i]);
        } else if (is_arg(argv[i], "dont_delete_antiques")) {
            log_messages.printf(MSG_CRITICAL, "'%s' has no effect, this file deleter does no antique files deletion\n", argv[i]);
        } else if (is_arg(argv[i], "antiques_deletion_dry_run")) {
            log_messages.printf(MSG_CRITICAL, "'%s' has no effect, this file deleter does no antique files deletion\n", argv[i]);
        } else if (is_arg(argv[i], "delete_antiques_interval")) {
            log_messages.printf(MSG_CRITICAL, "'%s' has no effect, this file deleter does no antique files deletion\n", argv[i]);
        } else if (is_arg(argv[i], "delete_antiques_usleep")) {
            log_messages.printf(MSG_CRITICAL, "'%s' has no effect, this file deleter does no antique files deletion\n", argv[i]);
        } else if (is_arg(argv[i], "delete_antiques_limit")) {
            log_messages.printf(MSG_CRITICAL, "'%s' has no effect, this file deleter does no antique files deletion\n", argv[i]);
        } else if (is_arg(argv[i], "dont_delete_batches")) {
            dont_delete_batches = true;
        } else if (is_arg(argv[i], "delete_antiques_now")) {
            log_messages.printf(MSG_CRITICAL, "'%s' has no effect, this file deleter does no antique files deletion\n", argv[i]);
        } else if (is_arg(argv[i], "input_files_only")) {
            do_output_files = false;
        } else if (is_arg(argv[i], "output_files_only")) {
            do_input_files = false;
        } else if (is_arg(argv[i], "sleep_interval")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            sleep_interval = atoi(argv[i]);
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    if (id_modulus) {
        log_messages.printf(MSG_DEBUG,
            "Using mod'ed WU/result enumeration.  mod = %d  rem = %d\n",
            id_modulus, id_remainder
        );
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    log_messages.printf(MSG_NORMAL, "Starting\n");

    retval = boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open DB\n");
        exit(1);
    }
    retval = boinc_db.set_isolation_level(READ_UNCOMMITTED);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.set_isolation_level: %s; %s\n",
            boincerror(retval), boinc_db.error_string()
        );
    }

    if (*app.name && !appid) {
      char buf[256];      
      sprintf(buf, "where name='%s'", app.name);
      retval = app.lookup(buf);
      if (retval) {
        log_messages.printf(MSG_CRITICAL, "Can't find app\n");
        exit(1);
      }
      appid=app.id;
      log_messages.printf(MSG_DEBUG, "Deleting files of appid %d\n",appid);
    }

    install_stop_signal_handler();

    bool retry_errors_now = !dont_retry_errors;
    double next_error_time=0;
    while (1) {
        bool got_any = do_pass(false);
        if (retry_errors_now) {
            bool got_any_errors = do_pass(true);
            if (got_any_errors) {
                got_any = true;
            } else {
                retry_errors_now = false;
                next_error_time = dtime() + ERROR_INTERVAL;
                log_messages.printf(MSG_DEBUG,
                    "ending retry of previous errors\n"
                );
            }
        }
        if (one_pass) break;
        if (!got_any) {
            daemon_sleep(sleep_interval);
        }
        if (!dont_retry_errors && !retry_errors_now && (dtime() > next_error_time)) {
            retry_errors_now = true;
            log_messages.printf(MSG_DEBUG,
                "starting retry of previous errors\n"
            );
        }
    }
}

const char *BOINC_RCSID_bd0d4938a6 = "$Id$";
