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

// caught_sig_int will be set to true if STOP_SIGNAL (normally SIGHUP)
//  is caught.
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
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "Quitting due to SIGHUP\n");
        exit(0);
    }
    if (boinc_file_exists(STOP_DAEMONS_FILENAME)) {
        log_messages.printf(SCHED_MSG_LOG::NORMAL, "Quitting because trigger file '%s' is present\n", STOP_DAEMONS_FILENAME);
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
int try_fopen(const char* path, FILE*& f, const char* mode) {
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

void get_log_path(char* p, const char* filename) {
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

// Locality scheduling: get filename from result name
//

int extract_filename(char* in, char* out) {
    strcpy(out, in);
    char* p = strstr(out, "__");
    if (!p) return -1;
    *p = 0;
    return 0;
}

void compute_avg_turnaround(HOST& host, double turnaround) {
    double new_avg;
    if (host.avg_turnaround == 0) {
        new_avg = turnaround;
    } else {
        new_avg = .7*host.avg_turnaround + .3*turnaround;
    }
    log_messages.printf(SCHED_MSG_LOG::NORMAL,
        "turnaround %f; old %f; new %f\n",
        turnaround, host.avg_turnaround, new_avg
    );
    host.avg_turnaround = new_avg;
}

const char *BOINC_RCSID_affa6ef1e4 = "$Id$";
