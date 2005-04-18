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

// The BOINC file upload handler.
// See doc/upload.html for protocol spec.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>
#include <csignal>

#include "crypt.h"
#include "parse.h"
#include "util.h"
#include "error_numbers.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"

SCHED_CONFIG config;

#define ERR_TRANSIENT   true
#define ERR_PERMANENT   false

#define DEBUG_LEVEL     SCHED_MSG_LOG::DEBUG

char this_filename[256];

struct FILE_INFO {
    char name[256];
    double max_nbytes;
    char* xml_signature;
    char* signed_xml;
    int parse(FILE*);
};

int FILE_INFO::parse(FILE* in) {
    char buf[256];
    int retval;

    memset(this, 0, sizeof(FILE_INFO));
    signed_xml = strdup("");
    while (fgets(buf, 256, in)) {
        //log_messages.printf(SCHED_MSG_LOG::DEBUG, buf, "FILE_INFO::parse: ");
        if (match_tag(buf, "</file_info>")) return 0;
        else if (match_tag(buf, "<xml_signature>")) {
            retval = dup_element_contents(in, "</xml_signature>", &xml_signature);
            if (retval) return retval;
            continue;
        }
        strcatdup(signed_xml, buf);
        if (parse_str(buf, "<name>", name, sizeof(name))) {
            strcpy(this_filename, name);
            continue;
        }
        if (parse_double(buf, "<max_nbytes>", max_nbytes)) continue;
        if (match_tag(buf, "<generated_locally/>")) continue;
        if (match_tag(buf, "<upload_when_present/>")) continue;
        if (match_tag(buf, "<url>")) continue;
        log_messages.printf(SCHED_MSG_LOG::NORMAL, "FILE_INFO::parse: unrecognized: %s \n", buf);
    }
    return ERR_XML_PARSE;
}

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

    printf(
        "Content-type: text/plain\n\n"
        "<data_server_reply>\n"
        "    <status>%d</status>\n"
        "    <message>%s</message>\n"
        "</data_server_reply>\n",
        transient?1:-1,
        buf
    );

    log_messages.printf(
        SCHED_MSG_LOG::NORMAL, "Returning error to client %s: %s (%s)\n",
        get_remote_addr(), buf,
        transient?"transient":"permanent"
    );
    return 1;
}

int return_success(const char* text) {
    printf(
        "Content-type: text/plain\n\n"
        "<data_server_reply>\n"
        "    <status>0</status>\n"
    );
    if (text) {
        printf("    %s\n", text);
    }
    printf("</data_server_reply>\n");
    return 0;
}

#define BLOCK_SIZE  16382
double bytes_left;

// read from socket, write to file
// ALWAYS returns an HTML reply
//
int copy_socket_to_file(FILE* in, char* path, double offset, double nbytes) {
    unsigned char buf[BLOCK_SIZE];
    char buf2[256];
    FILE* out;
    int retval, n, m, fd, lockret;
    struct stat sbuf;

    out = fopen(path, "ab");
    if (!out) {
        return return_error(ERR_TRANSIENT, "can't open file %s: %s", path, strerror(errno));
    }

    // get file descriptor for locking purposes
    fd=fileno(out);
    if (fd<0) {
        fclose(out);
        return return_error(ERR_TRANSIENT, "can't get file descriptor for file %s: %s", path, strerror(errno));
    }

    // Put an advisory lock on the file.  This will prevent OTHER instances of file_upload_handler
    // from being able to write to the file.
    lockret=lockf(fd, F_TLOCK, 0);
    if (lockret) {
        fclose(out);
        return return_error(ERR_TRANSIENT, "can't get exclusive lock on file %s: %s", path, strerror(errno));
    }

    // check that file length corresponds to offset
   // TODO: use a 64-bit variant
    retval=stat(path, &sbuf);
    if (retval) {
        fclose(out);
        return return_error(ERR_TRANSIENT, "can't stat file %s: %s", path, strerror(errno));
    }
    if (sbuf.st_size != offset) {
        fclose(out);
	return return_error(ERR_TRANSIENT, "length of file %s %d bytes != offset %d bytes", path, (int)sbuf.st_size, offset);
    }

    bytes_left = nbytes - offset;
    if (bytes_left == 0) {
        fclose(out);
        log_messages.printf(SCHED_MSG_LOG::DEBUG, "offset == nbytes: %f\n", nbytes);
        return return_success(0);
    }

    while (1) {
        m = BLOCK_SIZE;
        if (m > bytes_left) m = (int)bytes_left;
        n = fread(buf, 1, m, in);
        if (n <= 0) {
            fclose(out);
            sprintf(buf2, "can't read file: asked for %d, got %d: %s", m, n, strerror(errno));
            return return_error(ERR_TRANSIENT, buf2);
        }
        m = fwrite(buf, 1, n, out);
        if (m != n) {
            fclose(out);
            return return_error(ERR_TRANSIENT, "can't write file: %s", strerror(errno));
        }
        bytes_left -= n;
        if (bytes_left == 0) break;
    }
    fclose(out);
    return return_success(0);
}

// read from socket, discard data
//
void copy_socket_to_null(FILE* in) {
    unsigned char buf[BLOCK_SIZE];

    while (1) {
        int n = fread(buf, 1, BLOCK_SIZE, in);
        if (n <= 0) return;
    }
}

// ALWAYS generates an HTML reply
//
int handle_file_upload(FILE* in, R_RSA_PUBLIC_KEY& key) {
    char buf[256], path[256];
    FILE_INFO file_info;
    int retval;
    double nbytes=-1, offset=0;
    bool is_valid;

    while (fgets(buf, 256, in)) {
#if 0
        log_messages.printf(SCHED_MSG_LOG::NORMAL,
            "got:%s\n", buf
        );
#endif
        if (match_tag(buf, "<file_info>")) {
            retval = file_info.parse(in);
            if (retval) {
                return return_error(ERR_PERMANENT, "FILE_INFO::parse");
            }
#if 0
            log_messages.printf(SCHED_MSG_LOG::NORMAL,
                "file info:\n%s\n", file_info.signed_xml
            );
#endif
            if (!config.ignore_upload_certificates) {
                if (!file_info.signed_xml || !file_info.xml_signature) {
                    log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                        "file info is missing signature\n"
                    );
                    return return_error(ERR_PERMANENT, "invalid signature");
                } else {
                    retval = verify_string(
                        file_info.signed_xml, file_info.xml_signature, key, is_valid
                    );
                    if (retval || !is_valid) {
                        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                            "verify_string() [%s] [%s] retval %d, is_valid = %d\n",
                            file_info.signed_xml, file_info.xml_signature,
                            retval, is_valid
                        );
                        log_messages.printf(SCHED_MSG_LOG::NORMAL,
                            "signed xml: %s", file_info.signed_xml
                        );
                        log_messages.printf(SCHED_MSG_LOG::NORMAL,
                            "signature: %s", file_info.xml_signature
                        );
                        return return_error(ERR_PERMANENT, "invalid signature");
                    }
                }
            }
            continue;
        }
        else if (parse_double(buf, "<offset>", offset)) continue;
        else if (parse_double(buf, "<nbytes>", nbytes)) continue;
        else if (match_tag(buf, "<data>")) {
            if (nbytes <= 0) {
                return return_error(ERR_PERMANENT, "nbytes missing or negative");
            }

            // enforce limits in signed XML
            //
            if (!config.ignore_upload_certificates) {
                if (nbytes > file_info.max_nbytes) {
                    sprintf(buf,
                        "file size (%d KB) exceeds limit (%d KB)",
                        (int)(nbytes/1024), (int)(file_info.max_nbytes/1024)
                    );
                    copy_socket_to_null(in);
                    return return_error(ERR_PERMANENT, buf);
                }
            }

            // make sure filename is legit
            //
            if (strstr(file_info.name, "..")) {
                return return_error(ERR_PERMANENT,
                    "file_upload_handler: .. found in filename: %s",
                    file_info.name
                );
            }

            if (strlen(file_info.name) == 0) {
                return return_error(ERR_PERMANENT,
                    "file_upload_handler: no filename; nbytes %f", nbytes
                );
            }

            retval = dir_hier_path(
                file_info.name, config.upload_dir, config.uldl_dir_fanout, true,
                path, true
            );
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "PID=%d Starting upload of %s from %s [offset=%.0f, nbytes=%.0f]\n",
                getpid(),
                file_info.name,
                get_remote_addr(),
                offset, nbytes
            );
            fflush(stderr);
            if (offset >= nbytes) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "ERROR: offset >= nbytes!!\n"
                );
                return return_success(0);
            }
            retval = copy_socket_to_file(in, path, offset, nbytes);
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "PID=%d Ended upload of %s from %s; retval %d\n",
                getpid(),
                file_info.name,
                get_remote_addr(),
                retval
            );
            fflush(stderr);
            return retval;
        } else {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "unrecognized: %s", buf
            );
        }
    }
    return return_error(ERR_PERMANENT, "Missing <data> tag");
}

// always returns HTML reply
//
int handle_get_file_size(char* file_name) {
    struct stat sbuf;
    char path[256], buf[256];
    int retval,fd;
    FILE *fp;

    // TODO: check to ensure path doesn't point somewhere bad
    // Use 64-bit variant
    //
    dir_hier_path(file_name, config.upload_dir, config.uldl_dir_fanout, true, path);
    retval = stat( path, &sbuf );
    if (retval && errno != ENOENT) {
        // file DOES perhaps exit, but can't stat it:try again later
        //
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "handle_get_file_size(): [%s] returning error\n", file_name);
        return return_error(ERR_TRANSIENT, "cannot open file" );
    } else if (retval) {
        // file does not exist: return zero length
        //
        log_messages.printf(SCHED_MSG_LOG::DEBUG, "handle_get_file_size(): [%s] returning zero\n", file_name);
        return return_success("<file_size>0</file_size>");
    } else if (!(fp=fopen(path, "ab"))) { 
        // file exists, but can't be written to: try again later
        // 
        /* Opening a file with append mode (a as the first character in
           the  mode argument) causes all subsequent writes to the file
           to be forced to the then current end-of-file, regardless  of
           intervening  calls  to  fseek(3C). If two separate processes
           open the same file for append, each process may write freely
           to  the file without fear of destroying output being written
           by the other.  The output from the  two  processes  will  be
           intermixed in the file in the order in which it is written.
        */
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "handle_get_file_size(): [%s] %d bytes long returning error\n",
            file_name, (int)sbuf.st_size
        );
        return return_error(ERR_TRANSIENT, "can't open file for writing" );
    } else if ((fd=fileno(fp))<0) {
        // file exists and is writable, but can't get file descriptor: try again later
        //
        fclose(fp);
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "handle_get_file_size(): [%s] %d bytes long returning error\n",
            file_name, (int)sbuf.st_size
        );
        return return_error(ERR_TRANSIENT, "can't get file descriptor" );
    } else if (lockf(fd, F_TEST, 0)) {
        // file locked by another file_upload_handler: try again later
        //
        fclose(fp);
        log_messages.printf(SCHED_MSG_LOG::CRITICAL,
            "handle_get_file_size(): [%s] %d bytes long returning error\n",
            file_name, (int)sbuf.st_size
        );
        return return_error(ERR_TRANSIENT, "file locked by another file_upload_handler" );
    } else {
        // file exists, writable, not locked, so return length.
        //
        fclose(fp);
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG, "handle_get_file_size(): [%s] returning %d\n",
            file_name, (int)sbuf.st_size
        );
        sprintf(buf, "<file_size>%d</file_size>", (int)sbuf.st_size);
        return return_success(buf);
    }
    return 0;
}

// always generates an HTML reply
//
int handle_request(FILE* in, R_RSA_PUBLIC_KEY& key) {
    char buf[256];
    char file_name[256];
    int major, minor, retval=0;
    bool got_version = false;
    bool did_something = false;

    while (fgets(buf, 256, in)) {
        log_messages.printf(SCHED_MSG_LOG::DEBUG, buf, "handle_request: ");
        if (parse_int(buf, "<core_client_major_version>", major)) {
#if 0
    // for now, allow old versions
            if (major != BOINC_MAJOR_VERSION) {
                retval = return_error(ERR_PERMANENT,
                    "Core client has major version %d; "
                    "expected %d.",
                    major, BOINC_MAJOR_VERSION
                );
                break;
            }
#endif
            got_version = true;
        } else if (parse_int(buf, "<core_client_minor_version>", minor)) {
            continue;
        } else if (match_tag(buf, "<file_upload>")) {
            if (!got_version) {
                retval = return_error(ERR_PERMANENT, "Missing version");
            } else {
                retval = handle_file_upload(in, key);
            }
            did_something = true;
            break;
        } else if (parse_str(buf, "<get_file_size>", file_name, sizeof(file_name))) {
            if (!got_version) {
                retval = return_error(ERR_PERMANENT, "Missing version");
            } else {
                retval = handle_get_file_size(file_name);
            }
            did_something = true;
            break;
        } else if (match_tag(buf, "<data_server_request>")) {
            // DO NOTHING
        } else {
            log_messages.printf(SCHED_MSG_LOG::DEBUG, "handle_request: unrecognized %s\n", buf);
        }
    }
    if (!did_something) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "handle_request: no command\n");
        return return_error(ERR_PERMANENT, "no command");
    }

    return retval;
}

int get_key(R_RSA_PUBLIC_KEY& key) {
    FILE* f;
    int retval;
    char buf[256];
    sprintf(buf, "%s/upload_public", config.key_dir);
    f = fopen(buf, "r");
    if (!f) return -1;
    retval = scan_key_hex(f, (KEY*)&key, sizeof(key));
    fclose(f);
    if (retval) return retval;
    return 0;
}

int pid;

void boinc_catch_signal(int signal_num) {
    log_messages.printf(SCHED_MSG_LOG::CRITICAL,
        "PID=%d FILE=%s (%.0f bytes left) IP=%s caught signal %d [%s]\n",
        pid, this_filename, bytes_left, get_remote_addr(),
        signal_num, strsignal(signal_num)
    );
    exit(1);
}

void installer() {
    pid=getpid();

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
}

int main() {
    int retval;
    R_RSA_PUBLIC_KEY key;
    char log_path[256];

    installer();

    get_log_path(log_path, "file_upload_handler.log");
    if (!freopen(log_path, "a", stderr)) {
        fprintf(stderr, "Can't open log file\n");
        return_error(ERR_TRANSIENT, "can't open log file");
        exit(1);
    }

    log_messages.pid = getpid();
    log_messages.set_debug_level(DEBUG_LEVEL);

    retval = config.parse_file("..");
    if (retval) {
        exit(1);
    }

    retval = get_key(key);
    if (retval) {
        return_error(ERR_TRANSIENT, "can't read key file");
        exit(1);
    }

    handle_request(stdin, key);
    return 0;
}

const char *BOINC_RCSID_470a0d4d11 = "$Id$";
