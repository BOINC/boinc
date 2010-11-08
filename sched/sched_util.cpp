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


#include "config.h"
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "filesys.h"
#include "md5_file.h"
#include "error_numbers.h"

#include "sched_msgs.h"
#include "sched_util.h"
#include "sched_config.h"
#include "util.h"

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

const char* STOP_DAEMONS_FILENAME = "stop_daemons";
    // NOTE: this must be same as in the "start" script
const char* STOP_SCHED_FILENAME = "stop_sched";
    // NOTE: this must be same as in the "start" script
const int STOP_SIGNAL = SIGHUP;
    // NOTE: this must be same as in the "start" script

void write_pid_file(const char* filename) {
#ifndef _USING_FCGI_
    FILE* fpid = fopen(filename, "w");
#else
    FCGI_FILE* fpid = FCGI::fopen(filename,"w");
#endif

    if (!fpid) {
        log_messages.printf(MSG_CRITICAL, "Couldn't write pid file\n");
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
        log_messages.printf(MSG_NORMAL, "Quitting due to SIGHUP\n");
        exit(0);
    }
    const char *stop_file = config.project_path(STOP_DAEMONS_FILENAME);
    if (boinc_file_exists(stop_file)) {
        log_messages.printf(MSG_NORMAL,
            "Quitting because trigger file '%s' is present\n",
            stop_file
        );
        exit(0);
    }
}

bool check_stop_sched() {
    return boinc_file_exists(config.project_path(STOP_SCHED_FILENAME));
}

// try to open a file.
// On failure:
//   return ERR_FOPEN if the dir is there but not file
//     (this is generally a nonrecoverable failure)
//   return ERR_OPENDIR if dir is not there.
//     (this is generally a recoverable error,
//     like NFS mount failure, that may go away later)
//
#ifndef _USING_FCGI_
int try_fopen(const char* path, FILE*& f, const char* mode) {
#else
int try_fopen(const char* path, FCGI_FILE*& f, const char *mode) {
#endif
    const char* p;
    DIR* d;
    char dirpath[256];

#ifndef _USING_FCGI_
    f = fopen(path, mode);
#else
    f = FCGI::fopen(path, mode);
#endif

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
    char host[256];
    const char *dir;

    gethostname(host, 256);
    char* q = strchr(host, '.');
    if (q) *q=0;
    dir = config.project_path("log_%s", host);
    sprintf(p, "%s/%s", dir, filename);
    mode_t old_mask = umask(0);
    mkdir(dir, 01770);
        // make log_x directory sticky and group-rwx
        // so that whatever apache puts there will be owned by us
    umask(old_mask);
}

static void filename_hash(const char* filename, int fanout, char* dir) {
	std::string s = md5_string((const unsigned char*)filename, strlen(filename));
	int x = strtol(s.substr(1, 7).c_str(), 0, 16);
    sprintf(dir, "%x", x % fanout);
}

// given a filename, compute its path in a directory hierarchy
// If create is true, create the directory if needed
//
int dir_hier_path(
    const char* filename, const char* root, int fanout,
	char* path, bool create
) {
    char dir[256], dirpath[256];
    int retval;

    if (fanout==0) {
        sprintf(path, "%s/%s", root, filename);
        return 0;
    }

    filename_hash(filename, fanout, dir);

    sprintf(dirpath, "%s/%s", root, dir);
    if (create) {
        retval = boinc_mkdir(dirpath);
        if (retval && (errno != EEXIST)) {
            fprintf(stderr, "boinc_mkdir(%s): %s: errno %d\n",
                dirpath, boincerror(retval), errno
            );
            return ERR_MKDIR;
        }
    }
    sprintf(path, "%s/%s", dirpath, filename);
    return 0;
}

// same, but the output is a URL (used by tools/backend_lib.C)
//
int dir_hier_url(
    const char* filename, const char* root, int fanout,
	char* result
) {
    char dir[256];

    if (fanout==0) {
        sprintf(result, "%s/%s", root, filename);
        return 0;
    }

    filename_hash(filename, fanout, dir);
    sprintf(result, "%s/%s/%s", root, dir, filename);
    return 0;
}

void compute_avg_turnaround(HOST& host, double turnaround) {
    double new_avg;
    if (host.avg_turnaround == 0) {
        new_avg = turnaround;
    } else {
        new_avg = .7*host.avg_turnaround + .3*turnaround;
    }
    host.avg_turnaround = new_avg;
}

// Request lock on the given file with given fd.  Returns:
// 0 if we get lock
// PID (>0) if another process has lock
// -1 if error
//
int mylockf(int fd) {
    struct flock fl;
    fl.l_type=F_WRLCK;
    fl.l_whence=SEEK_SET;
    fl.l_start=0;
    fl.l_len=0;
    if (-1 != fcntl(fd, F_SETLK, &fl)) return 0;

    // if lock failed, find out why
    errno=0;
    fcntl(fd, F_GETLK, &fl);
    if (fl.l_pid>0) return fl.l_pid;
    return -1;
}

int count_results(char* query, int& n) {
    DB_RESULT result;
    int retval = result.count(n, query);
    if (retval) return retval;
    return 0;
}

int count_workunits(int& n, const char* query) {
    DB_WORKUNIT workunit;
    int retval = workunit.count(n, query);
    if (retval) return retval;
    return 0;
}

int count_unsent_results(int& n, int appid) {
    char buf[256];
    if (appid) {
        sprintf(buf, "where server_state=%d and appid=%d ",
            RESULT_SERVER_STATE_UNSENT, appid
        );
    } else {
        sprintf(buf, "where server_state=%d", RESULT_SERVER_STATE_UNSENT);
    }
    return count_results(buf, n);

}

bool is_arg(const char* x, const char* y) {
    char buf[256];
    strcpy(buf, "--");
    strcat(buf, y);
    if (!strcmp(buf, x)) return true;
    if (!strcmp(buf+1, x)) return true;
    return false;
}

#ifdef GCL_SIMULATOR

void simulator_signal_handler(int signum){    
    FILE *fsim;
    char currenttime[64];
    fsim = fopen(config.project_path("simulator/sim_time.txt"),"r");
    if(fsim){
        fscanf(fsim,"%s", currenttime);
        simtime = atof(currenttime); 
        fclose(fsim);
    }
    log_messages.printf(MSG_NORMAL,
        "Invoked by the simulator at time %.0f... \n", simtime
    );
}

int itime() {
    return (int) simtime;
}

void continue_simulation(const char *daemonname){
    char daemonfilelok[64];
    char daemonfile[64];
    sprintf(daemonfile, config.project_path("simulator/sim_%s.txt"),daemonname);
    sprintf(daemonfilelok, config.project_path("simulator/sim_%s.lok"),daemonname);
    FILE *fsimlok = fopen(daemonfilelok, "w");
    if (fsimlok){
        fclose(fsimlok);
        FILE *fsim = fopen(daemonfile, "w");
        if (fsim) {
            fclose(fsim);
        }
    }
    remove(daemonfilelok);
}

#endif

const char *BOINC_RCSID_affa6ef1e4 = "$Id$";
