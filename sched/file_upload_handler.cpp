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

// The BOINC file upload handler.
// See https://github.com/BOINC/boinc/wiki/FileUpload for protocol spec.
//

#include "config.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>
#include <csignal>
#include <fcntl.h>
#include <string>

#include "boinc_stdio.h"

#include "crypt.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "str_util.h"
#include "svn_version.h"
#include "util.h"

#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"

using std::string;

#define LOCK_FILES
    // comment this out to not lock files
    // this may avoid filesystem hangs

#define ERR_TRANSIENT   true
#define ERR_PERMANENT   false

#define FUH_MIN_FREE_SPACE 1e9

char this_filename[256];
string variety = "";
double start_time();

inline static const char* get_remote_addr() {
    char* p = getenv("REMOTE_ADDR");
    if (p) return p;
    return "Unknown remote address";
}

int return_error(bool transient, const char* message, ...) {
    va_list va;
    va_start(va, message);
    char buf[10240];

    vsprintf(buf, message, va);
    va_end(va);

    boinc::fprintf(stdout,
        "Content-type: text/plain\n\n"
        "<data_server_reply>\n"
        "    <status>%d</status>\n"
        "    <message>%s</message>\n"
        "</data_server_reply>\n",
        transient?1:-1,
        buf
    );

    log_messages.printf(MSG_NORMAL,
        "Returning error to client %s: %s (%s)\n",
        get_remote_addr(), buf,
        transient?"transient":"permanent"
    );
    return 1;
}

int return_success(const char* text) {
    boinc::fprintf(stdout,
        "Content-type: text/plain\n\n"
        "<data_server_reply>\n"
        "    <status>0</status>\n"
    );
    if (text) {
        boinc::fprintf(stdout, "    %s\n", text);
    }
    boinc::fprintf(stdout, "</data_server_reply>\n");
    return 0;
}

#define BLOCK_SIZE  (256*1024)
double bytes_left=-1;

int accept_empty_file(char* name, char* path) {
    int fd = open(path,
        O_WRONLY|O_CREAT,
        config.fuh_set_initial_permission
    );
    if (fd<0) {
        return return_error(ERR_TRANSIENT,
            "can't open file %s: %s\n", name, strerror(errno)
        );
    }
    close(fd);
    return return_success(0);
}

// read from socket, discard data
//
void copy_socket_to_null(FILE* in) {
    unsigned char buf[BLOCK_SIZE];

    while (1) {
        int n = boinc::fread(buf, 1, BLOCK_SIZE, in);
        if (n <= 0) return;
    }
}

// read from socket, write to file
// ALWAYS returns an HTML reply
//
int copy_socket_to_file(FILE* in, char* name, char* path, double offset, double nbytes) {
    unsigned char buf[BLOCK_SIZE];
    struct stat sbuf;
    int pid, fd=0;

    // caller guarantees that nbytes > offset
    //
    bytes_left = nbytes - offset;

    while (bytes_left > 0) {
        size_t m;

        m = bytes_left<(double)BLOCK_SIZE ? (size_t)bytes_left : BLOCK_SIZE;

        // try to get m bytes from socket (n>=0 is number actually returned)
        //
        size_t n = boinc::fread(buf, 1, m, in);

        // delay opening the file until we've done the first socket read
        // to avoid filesystem lockups (WCG, possible paranoia)
        //
        if (!fd) {
            // Use raw IO not buffered IO so that we can use reliable
            // posix file locking.
            // Advisory file locking is not guaranteed reliable when
            // used with stream buffered IO.
            //
            // coverity[toctou]
            fd = open(path,
                O_WRONLY|O_CREAT,
                config.fuh_set_initial_permission
            );
            if (fd<0) {
                if (errno == EACCES) {
                    // this is this case when the file was already uploaded
                    // and made read-only;
                    // return success to the client won't keep trying
                    //
                    log_messages.printf(MSG_WARNING,
                      "client tried to reupload the read-only file %s\n",
                      path
                    );
                    copy_socket_to_null(in);
                    return return_success(0);
                }
                return return_error(ERR_TRANSIENT,
                    "can't open file %s: %s\n", name, strerror(errno)
                );
            }

#ifdef LOCK_FILES
            // Put an advisory lock on the file.
            // This will prevent OTHER instances of file_upload_handler
            // from being able to write to the file.
            //
            pid = mylockf(fd);
            if (pid>0) {
                close(fd);
                return return_error(ERR_TRANSIENT,
                    "can't lock file %s: %s locked by PID=%d\n",
                    name, strerror(errno), pid
                );
            } else if (pid < 0) {
                close(fd);
                return return_error(ERR_TRANSIENT, "can't lock file %s\n", name);
            }
#endif

            // check that file length corresponds to offset
            // TODO: use a 64-bit variant
            //
            if (stat(path, &sbuf)) {
                close(fd);
                return return_error(ERR_TRANSIENT,
                    "can't stat file %s: %s\n", name, strerror(errno)
                );
            }
            if (sbuf.st_size < offset) {
                close(fd);
                return return_error(ERR_TRANSIENT,
                    "length of file %s %zu bytes < offset %.0f bytes",
                    name, sbuf.st_size, offset
                );
            }
            if (offset) {
                if (-1 == lseek(fd, offset, SEEK_SET)) {
                    int err = errno; // make a copy to report the lseek() error and not printf() or close() errors.
                    log_messages.printf(MSG_CRITICAL,
                        "lseek(%s, %.0f) failed: %s (%d).\n",
                        this_filename, offset, strerror(err), err
                    );
                    close(fd);
                    return return_error(ERR_TRANSIENT,
                        "can't resume partial file %s: %s\n", name, strerror(err)
                );
                }
            }
            if (sbuf.st_size > offset) {
                log_messages.printf(MSG_NORMAL,
                    "file %s length on disk %zu bytes; host upload starting at %.0f bytes.\n",
                     this_filename, sbuf.st_size, offset
                );
            }
        }

        // try to write n bytes to file
        //
        size_t to_write=n;
        while (to_write > 0) {
            ssize_t ret = write(fd, buf+n-to_write, to_write);
            if (ret < 0) {
                close(fd);
                const char* errmsg;
                if (errno == ENOSPC) {
                    errmsg = "No space left on server";
                } else {
                    errmsg = strerror(errno);
                }
                return return_error(ERR_TRANSIENT,
                    "can't write file %s: %s\n", name, errmsg
                );
            }
            to_write -= ret;
        }

        // check that we got all bytes from socket that were requested
        // Note: fread() reads less than requested only if there's
        // an error or EOF (see the man page)
        //
        if (n != m) {
            close(fd);
            if (boinc::feof(in)) {
                return return_error(ERR_TRANSIENT,
                    "EOF on socket read : asked for %d, got %d\n",
                    m, n
                );
            } else if (boinc::ferror(in)) {
                return return_error(ERR_TRANSIENT,
                    "error %d (%s) on socket read: asked for %d, got %d\n",
                    boinc::ferror(in), strerror(boinc::ferror(in)), m, n
                );
            } else {
                return return_error(ERR_TRANSIENT,
                    "incomplete socket read: asked for %d, got %d\n",
                    m, n
                );
            }
        }

        bytes_left -= n;
    }
    // upload complete; set new file permissions if configured
    //
    if (config.fuh_set_completed_permission >= 0) {
        if (fchmod(fd, config.fuh_set_completed_permission)) {
            log_messages.printf(MSG_CRITICAL, "can't set %03o permissions on %s: %s\n",
                config.fuh_set_completed_permission,
                path,
                strerror(errno));
        }
    }
    close(fd);
    return return_success(0);
}

// ALWAYS generates an HTML reply
//
int handle_file_upload(FILE* in, R_RSA_PUBLIC_KEY& key) {
    char buf[256], path[MAXPATHLEN], signed_xml[1024];
    char name[256], stemp[256];
    double max_nbytes=-1;
    char xml_signature[1024];
    int retval;
    double offset=0, nbytes = -1;
    bool is_valid, btemp;

    strcpy(name, "");
    strcpy(xml_signature, "");
    bool found_data = false;

    // TODO: use XML parser

    while (boinc::fgets(buf, 256, in)) {
        log_messages.printf(MSG_DEBUG, "got:%s\n", buf);
        if (match_tag(buf, "<file_info>")) continue;
        if (match_tag(buf, "</file_info>")) continue;
        if (match_tag(buf, "<signed_xml>")) continue;
        if (match_tag(buf, "</signed_xml>")) continue;
        if (parse_bool(buf, "generated_locally", btemp)) continue;
        if (parse_bool(buf, "upload_when_present", btemp)) continue;
        if (parse_str(buf, "<url>", stemp, sizeof(stemp))) continue;
        if (parse_str(buf, "<md5_cksum>", stemp, sizeof(stemp))) continue;
        if (match_tag(buf, "<xml_signature>")) {
            copy_element_contents(
                in, "</xml_signature>", xml_signature, sizeof(xml_signature)
            );
            continue;
        }
        if (parse_str(buf, "<name>", name, sizeof(name))) {
            safe_strcpy(this_filename, name);
            continue;
        }
        if (parse_double(buf, "<max_nbytes>", max_nbytes)) continue;
        if (parse_double(buf, "<offset>", offset)) continue;
        if (parse_double(buf, "<nbytes>", nbytes)) continue;
        if (match_tag(buf, "<data>")) {
            found_data = true;
            break;
        }
        strip_whitespace(buf);
        if (strlen(buf)) {
            log_messages.printf(MSG_WARNING, "unrecognized: %s", buf);
        }
    }
    if (strlen(name) == 0) {
        return return_error(ERR_PERMANENT, "Missing name");
    }
    if (!found_data) {
        return return_error(ERR_PERMANENT, "Missing <data> tag");
    }
    if (!config.ignore_upload_certificates) {
        if (strlen(xml_signature) == 0) {
            return return_error(ERR_PERMANENT, "missing signature");
        }
        if (max_nbytes < 0) {
            return return_error(ERR_PERMANENT, "missing max_nbytes");
        }
        sprintf(signed_xml,
            "<name>%s</name><max_nbytes>%.0f</max_nbytes>",
            name, max_nbytes
        );
        retval = check_string_signature(
            signed_xml, xml_signature, key, is_valid
        );
        if (retval || !is_valid) {
            log_messages.printf(MSG_CRITICAL,
                "check_string_signature() [%s] [%s] retval %d, is_valid = %d\n",
                signed_xml, xml_signature,
                retval, is_valid
            );
            log_messages.printf(MSG_NORMAL, "signed xml: %s\n", signed_xml);
            log_messages.printf(MSG_NORMAL, "signature: %s\n", xml_signature);
            return return_error(ERR_PERMANENT, "invalid signature");
        }
    }
    if (nbytes < 0) {
        return return_error(ERR_PERMANENT, "nbytes missing or negative");
    }

    // enforce limits in signed XML
    //
    if (!config.ignore_upload_certificates) {
        if (nbytes > max_nbytes) {
            sprintf(buf,
                "file size (%d KB) exceeds limit (%d KB)",
                (int)(nbytes/1024), (int)(max_nbytes/1024)
            );
            copy_socket_to_null(in);
            return return_error(ERR_PERMANENT, buf);
        }
    }

    // make sure filename is legit
    //
    if (!is_valid_filename(name)) {
        return return_error(ERR_PERMANENT,
            "file_upload_handler: invalid filename: %s",
            name
        );
    }

    if (strlen(name) == 0) {
        return return_error(ERR_PERMANENT,
            "file_upload_handler: no filename; nbytes %f", nbytes
        );
    }

    retval = dir_hier_path(
        name, config.upload_dir, config.uldl_dir_fanout,
        path, true
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Failed to find/create directory for file '%s' in '%s'\n",
            name, config.upload_dir
        );
        return return_error(ERR_TRANSIENT, "can't open file %s: %s",
            name, boincerror(retval)
        );
    }
    log_messages.printf(MSG_NORMAL,
        "Starting upload of %s from %s [offset=%.0f, nbytes=%.0f]\n",
        name,
        get_remote_addr(),
        offset, nbytes
    );
#ifndef _USING_FCGI_
    fflush(stderr);
#endif
    if (nbytes == 0) {
        retval = accept_empty_file(name, path);
        log_messages.printf(MSG_NORMAL,
            "accepted empty file %s from %s\n", name, get_remote_addr()
        );
    } else {
        if (offset >= nbytes) {
            log_messages.printf(MSG_CRITICAL,
                "ERROR: offset >= nbytes!!\n"
            );
            return return_success(0);
        }
        retval = copy_socket_to_file(in, name, path, offset, nbytes);
        log_messages.printf(MSG_NORMAL,
            "Ended upload of %s from %s; retval %d\n",
            name,
            get_remote_addr(),
            retval
        );
    }
#ifndef _USING_FCGI_
    fflush(stderr);
#endif
    return retval;
}

bool volume_full(char* path) {
    double total, avail;
    int retval = get_filesystem_info(total, avail, path);
    if (retval) return false;
    if (avail < FUH_MIN_FREE_SPACE) {
        return true;
    }
    return false;
}

// always returns HTML reply
//
int handle_get_file_size(char* file_name) {
    struct stat sbuf;
    char path[MAXPATHLEN], buf[256];
    int retval, pid, fd;

    // TODO: check to ensure path doesn't point somewhere bad
    // Use 64-bit variant
    //
    retval = dir_hier_path(
        file_name, config.upload_dir, config.uldl_dir_fanout, path
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Failed to find/create directory for file '%s' in '%s'.\n",
            file_name, config.upload_dir
        );
        return return_error(ERR_TRANSIENT, "can't open file");
    }

    // if the volume is full, report a transient error
    // to prevent the client from starting a transfer
    //
    if (volume_full(config.upload_dir)) {
        return return_error(ERR_TRANSIENT, "Server is out of disk space");
    }

    fd = open(path, O_RDONLY);

    if (fd<0 && ENOENT==errno) {
        // file does not exist: return zero length
        //
        log_messages.printf(MSG_NORMAL,
            "handle_get_file_size(): [%s] returning zero\n", file_name
        );
        return return_success("<file_size>0</file_size>");
    }

    if (fd<0) {
        // can't get file descriptor: try again later
        //
        log_messages.printf(MSG_CRITICAL,
            "handle_get_file_size(): cannot open [%s] %s\n",
            file_name, strerror(errno)
        );
        return return_error(ERR_TRANSIENT, "can't open file");
    }
#ifdef LOCK_FILES
    if ((pid = checklockf(fd))) {
        // file locked by another file_upload_handler: try again later
        //
        close(fd);
        log_messages.printf(MSG_CRITICAL,
            "handle_get_file_size(): [%s] returning error\n", file_name
        );
        return return_error(ERR_TRANSIENT,
            "[%s] locked by file_upload_handler PID=%d", file_name, pid
        );
    }
#endif
    // file exists, readable, not locked by anyone else, so return length.
    //
    retval = stat(path, &sbuf);
    close(fd);
    if (retval) {
        // file DOES perhaps exist, but can't stat it: try again later
        //
        log_messages.printf(MSG_CRITICAL,
            "handle_get_file_size(): [%s] returning error %s\n",
            file_name, strerror(errno)
        );
        return return_error(ERR_TRANSIENT, "cannot stat file" );
    }

    log_messages.printf(MSG_NORMAL,
        "handle_get_file_size(): [%s] returning %d\n",
        file_name, (int)sbuf.st_size
    );
    sprintf(buf, "<file_size>%d</file_size>", (int)sbuf.st_size);
    return return_success(buf);
}

// always generates an HTML reply
//
int handle_request(FILE* in, R_RSA_PUBLIC_KEY& key) {
    char buf[256];
    char file_name[256];
    int major, minor, release, retval=0;
    bool did_something = false;
    double start_time = dtime();

#ifdef _USING_FCGI_
    log_messages.set_indent_level(1);
#endif
    while (boinc::fgets(buf, 256, in)) {
        log_messages.printf(MSG_DEBUG, "handle_request: %s", buf);
        if (parse_int(buf, "<core_client_major_version>", major)) {
            continue;
        } else if (parse_int(buf, "<core_client_minor_version>", minor)) {
            continue;
        } else if (parse_int(buf, "<core_client_release>", release)) {
            continue;
        } else if (match_tag(buf, "<file_upload>")) {
            retval = handle_file_upload(in, key);
            did_something = true;
            break;
        } else if (parse_str(buf, "<get_file_size>", file_name, sizeof(file_name))) {
            if (strstr(file_name, "..")) {
                return return_error(ERR_PERMANENT, "Bad filename");
            }
            retval = handle_get_file_size(file_name);
            did_something = true;
            break;
        } else if (match_tag(buf, "<data_server_request>")) {
            // DO NOTHING
        } else {
            log_messages.printf(MSG_DEBUG, "handle_request: unrecognized %s\n", buf);
        }
    }
    if (!did_something) {
        log_messages.printf(MSG_WARNING, "handle_request: no command\n");
        return return_error(ERR_TRANSIENT, "no command");
    }

    log_messages.printf(MSG_DEBUG, "elapsed time %f seconds\n", dtime()-start_time);

    return retval;
}

int get_key(R_RSA_PUBLIC_KEY& key) {
    int retval;
    char buf[256];
    sprintf(buf, "%s/upload_public", config.key_dir);
    FILE *f = boinc::fopen(buf, "r");
    if (!f) return -1;
#ifdef _USING_FCGI_
    retval = scan_key_hex(FCGI_ToFILE(f), (KEY*)&key, sizeof(key));
#else
    retval = scan_key_hex(f, (KEY*)&key, sizeof(key));
#endif
    boinc::fclose(f);
    if (retval) return retval;
    return 0;
}

void boinc_catch_signal(int signal_num) {
    char buffer[512]="";
    if (this_filename[0]) {
        sprintf(buffer, "FILE=%s (%.0f bytes left) ", this_filename, bytes_left);
    }
    log_messages.printf(MSG_CRITICAL,
        "%sIP=%s caught signal %d [%s]\n",
        buffer, get_remote_addr(),
        signal_num, strsignal(signal_num)
    );
#ifdef _USING_FCGI_
    // flush log for FCGI, otherwise it just buffers a lot
    log_messages.flush();
#endif

    // there is no point in trying to return an error.
    // At this point Apache has broken the connection
    // so a write to stdout will just generate a SIGPIPE
    //
    // return_error(ERR_TRANSIENT, "while downloading %s server caught signal %d", this_filename, signal_num);
    _exit(1);
}

void boinc_reopen_logfile(int signal_num) {
    char log_name[MAXPATHLEN];
    char log_path[MAXPATHLEN];

    // only handle SIGUSR1 here
    if (signal_num != SIGUSR2) {
        boinc_catch_signal(signal_num);
    }
    sprintf(log_name, "file_upload_handler%s.log", variety.c_str());
    if (get_log_path(log_path, log_name) == ERR_MKDIR) {
        boinc::fprintf(stderr, "Can't create log directory '%s'  (errno: %d)\n", log_path, errno);
    }
#ifndef _USING_FCGI_
    if (!freopen(log_path, "a", stderr)) {
        fprintf(stderr, "Can't open log file '%s' (errno: %d)\n",
            log_path, errno
        );
        return_error(ERR_TRANSIENT, "can't open log file '%s' (errno: %d)",
            log_path, errno
        );
        exit(1);
    }
#else
    FILE *f = boinc::fopen(log_path, "a");
    if (f) {
       log_messages.redirect(f);
    } else {
        boinc::fprintf(stderr, "Can't redirect FCGI log messages\n");
        return_error(ERR_TRANSIENT, "can't open log file (FCGI)");
        exit(1);
    }
#endif

}

void installer() {
    signal(SIGHUP, boinc_catch_signal);  // terminal line hangup
    signal(SIGINT, boinc_catch_signal);  // interrupt program
    signal(SIGQUIT, boinc_catch_signal); // quit program
    signal(SIGILL, boinc_catch_signal);  // illegal instruction
    signal(SIGTRAP, boinc_catch_signal); // illegal instruction
    signal(SIGABRT, boinc_catch_signal); // abort(2) call
    signal(SIGFPE, boinc_catch_signal);  // bus error
    signal(SIGKILL, boinc_catch_signal); // bus error
    signal(SIGBUS, boinc_catch_signal);  // bus error
    signal(SIGSEGV, boinc_catch_signal); // segmentation violation
    signal(SIGSYS, boinc_catch_signal);  // system call given invalid argument
    signal(SIGPIPE, boinc_catch_signal); // write on a pipe with no reader
    signal(SIGTERM, boinc_catch_signal); // terminate process
#ifdef _USING_FCGI_
    signal(SIGUSR1, boinc_catch_signal); // user defined 1
    signal(SIGUSR2, boinc_reopen_logfile); // user defined 2
#endif
}

void usage(char *name) {
    boinc::fprintf(stderr,
        "This is the BOINC file upload handler.\n"
        "It receives the results from the clients\n"
        "and puts them on the file server.\n\n"
        "Normally this is run as a CGI program.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ -h | --help ]        Show this help text.\n"
        "  [ -v | --version ]     Show version information.\n"
        "  [ -u V | --variety V]  Use V to construct logfile name and upload_dir from config.xml (FCGI only)\n",
        name
    );
}

int main(int argc, char *argv[]) {
    int retval;
    R_RSA_PUBLIC_KEY key;
#ifdef _USING_FCGI_
    unsigned int counter=0;
#endif

    for(int c = 1; c < argc; c++) {
        string option(argv[c]);
        if(option == "-v" || option == "--version") {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else if(option == "-h" || option == "--help") {
            usage(argv[0]);
            exit(0);
#ifdef _USING_FCGI_
        } else if(option == "-u" || option == "--variety") {
            variety = "_" + string(argv[++c]);
#endif
        } else if (option.length()){
            boinc::fprintf(stderr, "unknown command line argument: %s\n\n", argv[c]);
            usage(argv[0]);
            exit(1);
        }
    }

    installer();

    boinc_reopen_logfile(SIGUSR2);

    retval = config.parse_file();
    if (retval) {
        boinc::fprintf(stderr, "Can't parse config.xml: %s\n", boincerror(retval));
        return_error(ERR_TRANSIENT,
            "can't parse config file"
        );
        exit(1);
    }

    // check if --variety was specified and add it to config.upload_dir
    if (!variety.empty()) {
        log_messages.printf(MSG_NORMAL, "Using variety: %s\n", variety.c_str());
        strcat(config.upload_dir, variety.c_str());
    }

    log_messages.pid = getpid();
    log_messages.set_debug_level(config.fuh_debug_level);

    if (boinc_file_exists(config.project_path("stop_upload"))) {
        return_error(ERR_TRANSIENT,
            "File uploads are temporarily disabled."
        );
#ifndef _USING_FCGI_
        exit(1);
#endif
    }

    if (!config.ignore_upload_certificates) {
        retval = get_key(key);
        if (retval) {
            return_error(ERR_TRANSIENT, "can't read key file");
            exit(1);
        }
    }

    if (access(config.upload_dir, W_OK)) {
        log_messages.printf(MSG_CRITICAL, "can't write to upload_dir\n");
        return_error(ERR_TRANSIENT, "can't write to upload_dir");
        exit(1);
    }

    // intentionally disallows a value of 0 as this would mean we can't write the file in the first place
    if (config.fuh_set_initial_permission > 0) {
        // sanitize user input, no execute flags allowed for uploaded files
        config.fuh_set_initial_permission &= (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    } else {
        config.fuh_set_initial_permission = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
    }
    // intentionally allows a value of 0
    if (config.fuh_set_completed_permission >= 0) {
        // sanitize user input, no execute flags allowed for uploaded files
        config.fuh_set_completed_permission &= (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    }

#ifdef _USING_FCGI_
    log_messages.flush();
    while(FCGI_Accept() >= 0) {
        counter++;
        //fprintf(stderr, "file_upload_handler (FCGI): counter: %d\n", counter);
        if (boinc_file_exists(config.project_path("stop_upload"))) {
            return_error(ERR_TRANSIENT,
                "File uploads are temporarily disabled."
            );
            continue;
        }
        log_messages.set_indent_level(0);
#endif
        handle_request(stdin, key);
#ifdef _USING_FCGI_
        // flush log for FCGI, otherwise it just buffers a lot
        log_messages.flush();
    }
    // when exiting, write headers back to apache so it won't complain
    // about "incomplete headers"
    boinc::fprintf(stdout,"Content-type: text/plain\n\n");
#endif
    return 0;
}
