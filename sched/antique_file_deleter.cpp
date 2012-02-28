// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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


// antique_file deleter
// removes files from the upload/download hierarchies that are
// older than any WU
// See usage() below for usage.

// how long to wait until delete antiques, and how often to do it
//
#define ANTIQUE_USLEEP 50000 // worked good for E@H, can't be too bad

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
#ifdef HAVE_STRINGS_H
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

#define LOCKFILE "antique_file_deleter.out"
#define PIDFILE  "antique_file_deleter.pid"

int antique_usleep = ANTIQUE_USLEEP;
bool antiques_deletion_dry_run = false;

void usage(char *name) {
    fprintf(stderr, "Deletes files that have been uploaded after the result was purged from the DB.\n\n"
        "   scan for and delete all files in the upload directories\n"
        "   that are older than any WU in the database,\n"
        "   and were created at least one month ago.\n"
        "   This deletes files uploaded by hosts after the WU was deleted.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  -d N | --debug_level N          set debug output level (1 to 4)\n"
        "  --dry_run                       don't delete any files, just log what would be deleted\n"
        "  --usleep N                      sleep this number of usecs after each examined file.\n"
        "                                  Throttles I/O if there are many files. Defaults to %d.\n"
        "  [ -h | --help ]                 shows this help text\n"
        "  [ -v | --version ]              shows version information\n",
        name, ANTIQUE_USLEEP
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

// return ctime() w/o \n
//
static inline char* actime(time_t t) {
    char*p=ctime(&t);
    char*c=strchr(p,'\n');
    if(c) *c='\0';
    return p;
}

//  returns:
//  0 if all went ok;
//  1 on a transient error affecting only a single file;
// -1 on a serious error that should switch off antique file deletion
//
int delete_antiques_from_dir(char*dirpath, time_t mtime, uid_t uid) {
    DIR*dir;
    struct dirent*entry;
    struct stat fstat;
    int ret;
    char path[2*256];

    // open directory
    errno = 0;
    dir = opendir(dirpath);
    if (!dir) {
        log_messages.printf(MSG_CRITICAL,
            "delete_antiques_from_dir(): "
            "Couldn't open dir '%s': %s (%d)\n",
            dirpath, strerror(errno), errno
        );
        return -1;
    }

    // scan directory
    errno = 0;
    while ((entry = readdir(dir))) {

        // might be woken by a signal
        check_stop_daemons();

        // construct absolute path of this entry
        strcpy(path, dirpath);
        strcat(path, "/");
        strcat(path, entry->d_name);

        // examine file
        log_messages.printf(MSG_DEBUG,
            "delete_antiques_from_dir(): examining file: '%s'\n",
            path
        );

        // stat
        errno = 0;
        if (lstat(path, &fstat)) {
            log_messages.printf(MSG_NORMAL,
                "delete_antiques_from_dir(): couldn't stat '%s: %s (%d)'\n",
                path, strerror(errno), errno
            );

        // regular file
        } else if ((fstat.st_mode & S_IFMT) != S_IFREG) {
            log_messages.printf(MSG_DEBUG,"not a regular plain file\n");

        // skip hidden files such as ".nfs"
        } else if (entry->d_name[0] == '.') {
            log_messages.printf(MSG_DEBUG,"hidden file or directory\n");

        // modification time
        } else if (fstat.st_mtime > mtime) {
            log_messages.printf(MSG_DEBUG,"too young: %s\n", actime(fstat.st_mtime));

        // check owner (must be apache)
        } else if (fstat.st_uid != uid) {
            log_messages.printf(MSG_DEBUG,"wrong owner: id %d\n", fstat.st_uid);

        // skip if dry_run
        } else if (antiques_deletion_dry_run) {
            log_messages.printf(MSG_NORMAL,
                  "Would delete '%s/%s' (%s)\n",
                dirpath, entry->d_name, actime(fstat.st_mtime));

        // found no reason to skip, actually delete this file
        } else {
            log_messages.printf(MSG_NORMAL, "Deleting file '%s' (%s)\n",
                path, actime(fstat.st_mtime)
            );
            errno = 0;
            if (unlink(path)) {
                log_messages.printf(MSG_CRITICAL,
                    "delete_antiques_from_dir(): "
                    "Couldn't unlink '%s: %s (%d)'\n",
                    path, strerror(errno), errno
                );
                closedir(dir);
                return 1;
            }
        }

        // throttle I/O if told to
        if (antique_usleep) {
            usleep(antique_usleep);
        }

    } // while (readdir())

    // if the loop terminated because of an error
    if (errno) {
        log_messages.printf(MSG_CRITICAL,
            "delete_antiques_from_dir(): "
            "Couldn't read dir '%s': %s (%d)\n",
            dirpath, strerror(errno), errno
        );
        closedir(dir);
        return 1;
    }

    // close dir
    errno = 0;
    ret = closedir(dir);
    if (ret) {
        log_messages.printf(MSG_CRITICAL,
            "delete_antiques_from_dir(): "
            "Couldn't close dir '%s': %s (%d)\n",
            dirpath, strerror(errno), errno
        );
    }
    return ret;
}


// collect information and call delete_antiques_from_dir()
// for every relevant directory
//
static int delete_antiques() {
    DB_WORKUNIT wu;
    time_t t = 0;
    int ret = 0;

    // t = min (create_time_of_oldest_wu, 31days_ago)
    t = time(0) - 32*86400;
    if (!wu.enumerate("order by id limit 1") && (t > wu.create_time)) {
        t = wu.create_time - 86400;
    }

    // find numerical userid of apache
    struct passwd *apache_info = getpwnam(config.httpd_user);

    if (!apache_info) {
        log_messages.printf(MSG_CRITICAL,
            "Couldn't find http_user '%s' in passwd\n",
            config.httpd_user
        );
        return -1;
    }

    log_messages.printf(MSG_DEBUG,
         "delete_antiques(): "
         "Deleting files older than epoch %lu (%s) with userid %u\n",
         (unsigned long)t,
         actime(t),
         apache_info->pw_uid
    );

    // if fanout is configured, scan every fanout directory,
    // else just the plain upload directory
    if (config.uldl_dir_fanout) {
        for(int d = 0; d < config.uldl_dir_fanout; d++) {
            char buf[270];
            snprintf(buf, sizeof(buf), "%s/%x", config.upload_dir, d);
            log_messages.printf(MSG_DEBUG,
                "delete_antiques(): scanning upload fanout directory '%s'\n",
                buf
            );
            ret = delete_antiques_from_dir(buf, t, apache_info->pw_uid);
            if (ret < 0) return ret;
        }
    } else {
        log_messages.printf(MSG_DEBUG,
            "delete_antiques(): scanning upload directory '%s'\n",
            config.upload_dir
        );
        ret = delete_antiques_from_dir(config.upload_dir, t, apache_info->pw_uid);
    }
    return ret;
}


int main(int argc, char** argv) {
    int retval;
    int i;
    
    check_stop_daemons();

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "d") || is_arg(argv[i], "debug_level")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (is_arg(argv[i], "dry_run")) {
            antiques_deletion_dry_run = true;
        } else if (is_arg(argv[i], "usleep")) {
            antique_usleep = atoi(argv[++i]);
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

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    log_messages.printf(MSG_NORMAL, "Starting\n");

    retval = boinc_db.open(
        config.replica_db_name,
        config.replica_db_host,
        config.replica_db_user,
        config.replica_db_passwd
    );
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

    install_stop_signal_handler();

    retval = delete_antiques();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "delete_antiques() returned with error %d\n",
            retval
        );
    }

    log_messages.printf(MSG_NORMAL, "Done\n");

    return retval;
}

const char *BOINC_RCSID_bd0d4938a6 = "$Id$";
