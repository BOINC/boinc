// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

using namespace std;

#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "filesys.h"
#include "md5_file.h"
#include "error_numbers.h"

#include "sched_msgs.h"
#include "sched_util.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

const char* STOP_DAEMONS_FILENAME = "../stop_daemons";
    // NOTE: this must be same as in the "start" script
const char* STOP_SCHED_FILENAME = "../stop_sched";
    // NOTE: this must be same as in the "start" script
const int STOP_SIGNAL = SIGHUP;
    // NOTE: this must be same as in the "start" script

void write_pid_file(const char* filename) {
    FILE* fpid = fopen(filename, "w");
    if (!fpid) {
        log_messages.printf(SCHED_MSG_LOG::NORMAL, "Couldn't write pid\n");
        return;
    }
    fprintf(fpid, "%d\n", (int)getpid());
    fclose(fpid);
}

// caught_sig_int will be set to true if SIGINT is caught.
bool caught_stop_signal = false;
static void stop_signal_handler(int) {
    fprintf(stderr, "GOT STOP SIGNAL\n");
    caught_stop_signal = true;
}

void install_stop_signal_handler() {
    signal(STOP_SIGNAL, stop_signal_handler);
    // handler is now default again so hitting ^C again will kill the program.
}

void check_stop_daemons() {
    if (caught_stop_signal) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Quitting due to SIGINT\n");
        exit(0);
    }
    if (boinc_file_exists(STOP_DAEMONS_FILENAME)) {
        log_messages.printf(SCHED_MSG_LOG::NORMAL, "Quitting due to stop trigger\n");
        exit(0);
    }
}

bool check_stop_sched() {
    return boinc_file_exists(STOP_SCHED_FILENAME);
}

// try to open a file.
// On failure:
//   return ERR_FOPEN if the dir is there but not file
//     (this is generally a nonrecoverable failure)
//   return ERR_OPENDIR if dir is not there.
//     (this is generally a recoverable error,
//     like NFS mount failure, that may go away later)
//
int try_fopen(char* path, FILE*& f, char* mode) {
    char* p;
    DIR* d;
    char dirpath[256];

    f = fopen(path, mode);
    if (!f) {
        memset(dirpath, '\0', sizeof(dirpath));
        p = strrchr(path, '/');
        if (p) {
            strncpy(dirpath, path, (int)(p-path));
        } else {
            strcpy(dirpath, ".");
        }
        if ((d = opendir(dirpath)) == NULL) {
            return ERR_OPENDIR;
        } else {
            closedir(d);
            return ERR_FOPEN;
        }
    }
    return 0;
}

void get_log_path(char* p, char* filename) {
    char buf[256];
    char path[256];
    gethostname(buf, 256);
    char* q = strchr(buf, '.');
    if (q) *q=0;
    sprintf(path, "log_%s", buf);
    sprintf(p, "../%s/%s", path, filename);
    mkdir(path, 0777);
}

static void filename_hash_old(const char* filename, int fanout, char* dir) {
    int sum=0;
    const char* p = filename;

    while (*p) sum += *p++;
    sum %= fanout;
	sprintf(dir, "%x", sum);
}

static void filename_hash(const char* filename, int fanout, char* dir) {
	std::string s = md5_string((const unsigned char*)filename, strlen(filename));
	int x = strtol(s.substr(1, 7).c_str(), 0, 16);
    sprintf(dir, "%x", x % fanout);
}

// given a filename, compute its path in a directory hierarchy
// If create is true, create the directory if needed
// NOTE: this first time around I used a bad hash function.
// During the period of transition to the good hash function,
// programs to look for files (validator, assimilator, file deleter)
// will have to try both the old and new variants.
// We can phase this out after everyone is caught up.
//
int dir_hier_path(
    const char* filename, const char* root, int fanout, bool new_hash,
	char* path, bool create
) {
    char dir[256], dirpath[256];
    int retval;

    if (fanout==0) {
        sprintf(path, "%s/%s", root, filename);
        return 0;
    }

	if (new_hash) {
	    filename_hash(filename, fanout, dir);
	} else {
	    filename_hash_old(filename, fanout, dir);
	}

    sprintf(dirpath, "%s/%s", root, dir);
    if (create) {
        retval = boinc_mkdir(dirpath);
        if (retval && (retval != EEXIST)) {
            return ERR_MKDIR;
        }
    }
    sprintf(path, "%s/%s", dirpath, filename);
    return 0;
}

int dir_hier_url(
    const char* filename, const char* root, int fanout, bool new_hash,
	char* result
) {
    char dir[256];

    if (fanout==0) {
        sprintf(result, "%s/%s", root, filename);
        return 0;
    }

	if (new_hash) {
		filename_hash(filename, fanout, dir);
	} else {
		filename_hash_old(filename, fanout, dir);
	}
    sprintf(result, "%s/%s/%s", root, dir, filename);
    return 0;
}

const char *BOINC_RCSID_affa6ef1e4 = "$Id$";
