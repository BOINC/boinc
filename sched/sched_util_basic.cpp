// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// Utility functions for server software (not just scheduler)

#include "config.h"
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "error_numbers.h"
#include "filesys.h"
#include "md5_file.h"
#include "util.h"
#include "str_replace.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"
#include "boinc_stdio.h"

const char* STOP_DAEMONS_FILENAME = "stop_daemons";
    // NOTE: this must be same as in the "start" script
const char* STOP_SCHED_FILENAME = "stop_sched";
    // NOTE: this must be same as in the "start" script
const int STOP_SIGNAL = SIGHUP;
    // NOTE: this must be same as in the "start" script

void write_pid_file(const char* filename) {
    FILE* fpid = boinc::fopen(filename, "w");

    if (!fpid) {
        log_messages.printf(MSG_CRITICAL, "Couldn't write pid file\n");
        return;
    }
    boinc::fprintf(fpid, "%d\n", (int)getpid());
    boinc::fclose(fpid);
}

// caught_sig_int will be set to true if STOP_SIGNAL (normally SIGHUP)
//  is caught.
bool caught_stop_signal = false;
static void stop_signal_handler(int) {
    boinc::fprintf(stderr, "GOT STOP SIGNAL\n");
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

// sleep for n seconds, but check every second for trigger file
//
void daemon_sleep(int nsecs) {
    for (int i=0; i<nsecs; i++) {
        check_stop_daemons();
        sleep(1);
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
int try_fopen(const char* path, FILE*& f, const char* mode) {
    const char* p;
    DIR* d;
    char dirpath[MAXPATHLEN];

    f = boinc::fopen(path, mode);

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

int get_log_path(char* p, const char* filename) {
    char host[256];
    const char *dir;

    gethostname(host, 256);
    char* q = strchr(host, '.');
    if (q) *q=0;
    dir = config.project_path("log_%s", host);
    sprintf(p, "%s/%s", dir, filename);
    mode_t old_mask = umask(0);
    // make log_x directory sticky and group-rwx
    // so that whatever apache puts there will be owned by us
    int retval = mkdir(dir, 01770);
    umask(old_mask);
    if (retval && errno != EEXIST) return ERR_MKDIR;

    return 0;
}

static void filename_hash(const char* filename, int fanout, char* dir) {
    std::string s = md5_string((const unsigned char*)filename, strlen(filename));
    int x = strtol(s.substr(1, 7).c_str(), 0, 16);
    sprintf(dir, "%x", x % fanout);
}

// returns:
// 0 if same file is already there and has correct .md5, we don't need to copy or create .md5
// 1 if same file is already there and .md5 file is missing, need to create corresponding .md5 file
// 2 if a file isn't there, need to copy and create .md5
// -1 if a different file is there
// -2 if a file operation failed
//
// file_path - source path to file, dl_hier_path - path to the same file in download hier
//
int check_download_file(const char* file_path, const char* dl_hier_path) {
    bool md5_file_exists = false;
    char md5_file_path[MAXPATHLEN];
    char md5_hash_src[MD5_LEN], md5_hash_dst[MD5_LEN];
    double nbytes;
    std::string file_content, file_hash;
    int file_size;

    int retval = md5_file(file_path, md5_hash_src, nbytes);
    if (retval) {
        return -2;
    }

    snprintf(md5_file_path, MAXPATHLEN, "%s.md5", dl_hier_path);
    if (boinc_file_exists(md5_file_path)) {
        retval = read_file_string(md5_file_path, file_content);
        if (retval) {
            return -2;
        }
        std::stringstream stream(file_content);
        stream >> file_hash >> file_size;
        md5_file_exists = true;
    }

    if (!boinc_file_exists(dl_hier_path)) {
        return 2;
    }
    // calculating md5 hash of existing file in dl hier
    retval = md5_file(dl_hier_path, md5_hash_dst, nbytes);
    if (retval) {
        return -2;
    }
    int hashes_equal = !strncmp(md5_hash_src, md5_hash_dst, MD5_LEN);
    if (md5_file_exists && hashes_equal) {
        // the right file with correct .md5 is there
        return 0;
    } else if (hashes_equal) {
        // files are the same, but need to create .md5
        return 1;
    }
    // if the content of the file in dl hier differs from the source file's content
    // then skip staging, consider to manually delete file in dl hier and retry
    return -1;
}

// given a filename, compute its path in a directory hierarchy
// If create is true, create the directory if needed
//
int dir_hier_path(
    const char* filename, const char* root, int fanout,
    char* path, bool create
) {
    char dir[256], dirpath[MAXPATHLEN];
    int retval;

    if (fanout==0) {
        snprintf(path, MAXPATHLEN, "%s/%s", root, filename);
        return 0;
    }

    filename_hash(filename, fanout, dir);

    snprintf(dirpath, MAXPATHLEN, "%s/%s", root, dir);
    if (create) {
        retval = boinc_mkdir(dirpath);
        if (retval && (errno != EEXIST)) {
            boinc::fprintf(stderr, "boinc_mkdir(%s): %s: errno %d\n",
                dirpath, boincerror(retval), errno
            );
            return ERR_MKDIR;
        }
    }
    snprintf(path, MAXPATHLEN, "%s/%s", dirpath, filename);
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
    // coverity[check_return]
    fcntl(fd, F_GETLK, &fl);
    if (fl.l_pid>0) return fl.l_pid;
    return -1;
}

// check if there is a write lock on the given file with given fd.  Returns:
// 0 if there is no write lock
// PID (>0) of the process that has the lock
// -1 if error
//
int checklockf(int fd) {
    struct flock fl;
    fl.l_type=F_RDLCK;
    fl.l_whence=SEEK_SET;
    fl.l_start=0;
    fl.l_len=0;
    if (-1 != fcntl(fd, F_GETLK, &fl)) {
        if (fl.l_type == F_UNLCK) return 0;
        if (fl.l_pid>0) return fl.l_pid;
    }
    return -1;
}

bool is_arg(const char* x, const char* y) {
    char buf[256];
    strcpy(buf, "--");
    safe_strcat(buf, y);
    if (!strcmp(buf, x)) return true;
    if (!strcmp(buf+1, x)) return true;
    return false;
}

// the following is used:
// - to enforce limits on in-progress jobs for GPUs and CPUs
//   (see handle_request.cpp)
// - to determine what resources the project has apps for (sched_shmem.cpp)
//
int plan_class_to_proc_type(const char* plan_class) {
    if (strstr(plan_class, "cuda")) {
        return PROC_TYPE_NVIDIA_GPU;
    }
    if (strstr(plan_class, "nvidia")) {
        return PROC_TYPE_NVIDIA_GPU;
    }
    if (strstr(plan_class, "ati")) {
        return PROC_TYPE_AMD_GPU;
    }
    if (strstr(plan_class, "amd")) {
        return PROC_TYPE_AMD_GPU;
    }
    if (strstr(plan_class, "intel_gpu")) {
        return PROC_TYPE_INTEL_GPU;
    }
    if (strstr(plan_class, "apple_gpu")) {
        return PROC_TYPE_APPLE_GPU;
    }
    return PROC_TYPE_CPU;
}

#ifdef GCL_SIMULATOR

void simulator_signal_handler(int signum) {
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
    sprintf(daemonfile, strcat((char*)config.project_path("simulator/"),"sim_%s.txt"),daemonname);
    sprintf(daemonfilelok, strcat((char*)config.project_path("simulator/"),"sim_%s.lok"),daemonname);
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
