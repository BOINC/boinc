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

#if defined(_WIN32)
#include "boinc_win.h"
#endif
#ifdef _WIN32
#include "win_util.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <sstream>
#include <string>
#include <cmath>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#endif

#include "boinc_stdio.h"
#include "error_numbers.h"
#include "common_defs.h"
#include "filesys.h"
#include "str_replace.h"
#include "str_util.h"

using std::string;
using std::stringstream;
using std::vector;

// Use this instead of strncpy().
// Result will always be null-terminated, and it's faster.
// see http://www.gratisoft.us/todd/papers/strlcpy.html
//
#if !HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t size) {
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size-1 : ret;
        memcpy(dst, src, len);
        dst[len] = '\0';
    }

    return ret;
}
#endif

#if !HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t size) {
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);

    if (size) {
        size_t len = (src_len >= size-dst_len) ? (size-dst_len-1) : src_len;
        memcpy(&dst[dst_len], src, len);
        dst[dst_len + len] = '\0';
    }

    return dst_len + src_len;
}
#endif // !HAVE_STRLCAT

#if !HAVE_STRCASESTR
// BOINC only uses strcasestr() for short strings,
// so the following will suffice
//
const char *strcasestr(const char *s1, const char *s2) {
    char needle[1024], haystack[1024], *p=NULL;
    strlcpy(haystack, s1, sizeof(haystack));
    strlcpy(needle, s2, sizeof(needle));
    // convert both strings to lower case
    p = haystack;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    p = needle;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    // find the substring
    p = strstr(haystack, needle);
    // correct the pointer to point to the substring within s1
    if (p) {
        p = const_cast<char *>(s1)+(p-haystack);
    }
    return p;
}
#endif

// version of strcpy that works even if strings overlap (p < q)
//
void strcpy_overlap(char* p, const char* q) {
    while (1) {
        *p++ = *q;
        if (!*q) break;
        q++;
    }
}

// Converts a double precision time (where the value of 1 represents
// a day) into a string.  smallest_timescale determines the smallest
// unit of time division used
// smallest_timescale: 0=seconds, 1=minutes, 2=hours, 3=days, 4=years
//
int ndays_to_string (double x, int smallest_timescale, char *buf) {
    double years, days, hours, minutes, seconds;
    char year_buf[64], day_buf[16], hour_buf[16], min_buf[16], sec_buf[16];

    if (x < 0 || buf == NULL) return ERR_NULL;

    years = x / 365.25;
    days = fmod(x, 365.25);
    hours = fmod(x*24, 24);
    minutes = fmod(x*24*60, 60);
    seconds = fmod(x*24*60*60, 60);

    if (smallest_timescale==4) {
        snprintf( year_buf, sizeof(year_buf), "%.3f yr ", years );
    } else if (years > 1 && smallest_timescale < 4) {
        snprintf( year_buf, sizeof(year_buf), "%d yr ", (int)years );
    } else {
        safe_strcpy( year_buf, "" );
    }

    if (smallest_timescale==3) {
        snprintf( day_buf, sizeof(day_buf), "%.2f day%s ", days, (days>1?"s":"") );
    } else if (days > 1 && smallest_timescale < 3) {
        snprintf( day_buf, sizeof(day_buf), "%d day%s ", (int)days, (days>1?"s":"") );
    } else {
        safe_strcpy( day_buf, "" );
    }

    if (smallest_timescale==2) {
        snprintf( hour_buf, sizeof(hour_buf), "%.2f hr ", hours );
    } else if (hours > 1 && smallest_timescale < 2) {
        snprintf( hour_buf, sizeof(hour_buf), "%d hr ", (int)hours );
    } else {
        safe_strcpy( hour_buf, "" );
    }

    if (smallest_timescale==1) {
        snprintf( min_buf, sizeof(min_buf), "%.2f min ", minutes );
    } else if (minutes > 1 && smallest_timescale < 1) {
        snprintf( min_buf, sizeof(min_buf), "%d min ", (int)minutes );
    } else {
        safe_strcpy( min_buf, "" );
    }

    if (smallest_timescale==0) {
        snprintf( sec_buf, sizeof(sec_buf), "%.2f sec ", seconds );
    } else if (seconds > 1 && smallest_timescale < 0) {
        snprintf( sec_buf, sizeof(sec_buf), "%d sec ", (int)seconds );
    } else {
        safe_strcpy( sec_buf, "" );
    }
    // the "-0.05" below is to prevent it from printing 60.0 sec
    // when the real value is e.g. 59.91
    //
    sprintf(buf, "%s%s%s%s%s", year_buf, day_buf, hour_buf, min_buf, sec_buf);

    return 0;
}

// convert seconds into a string "0h00m00s00"
//
void secs_to_hmsf(double secs, char* buf) {
    int s = secs;
    int f = (secs - s) * 100.0;
    int h = s / 3600;
    s -= h * 3600;
    int m = s / 60;
    s -= m * 60;
    sprintf(buf, "%uh%02um%02us%02u", h, m, s, f);
}

// Convert nbytes into a string.  If total_bytes is non-zero,
// convert the two into a fractional display (i.e. 4/16 KB)
//
void nbytes_to_string(double nbytes, double total_bytes, char* str, int len) {
    char buf[256];
    double xTera = (1024.0*1024.0*1024.0*1024.0);
    double xGiga = (1024.0*1024.0*1024.0);
    double xMega = (1024.0*1024.0);
    double xKilo = (1024.0);

    if (total_bytes != 0) {
        if (total_bytes >= xTera) {
            snprintf(buf, sizeof(buf), "%0.2f/%0.2f TB", nbytes/xTera, total_bytes/xTera);
        } else if (total_bytes >= xGiga) {
            snprintf(buf, sizeof(buf), "%0.2f/%0.2f GB", nbytes/xGiga, total_bytes/xGiga);
        } else if (total_bytes >= xMega) {
            snprintf(buf, sizeof(buf), "%0.2f/%0.2f MB", nbytes/xMega, total_bytes/xMega);
        } else if (total_bytes >= xKilo) {
            snprintf(buf, sizeof(buf), "%0.2f/%0.2f KB", nbytes/xKilo, total_bytes/xKilo);
        } else {
            snprintf(buf, sizeof(buf), "%0.0f/%0.0f bytes", nbytes, total_bytes);
        }
    } else {
        if (nbytes >= xTera) {
            snprintf(buf, sizeof(buf), "%0.2f TB", nbytes/xTera);
        } else if (nbytes >= xGiga) {
            snprintf(buf, sizeof(buf), "%0.2f GB", nbytes/xGiga);
        } else if (nbytes >= xMega) {
            snprintf(buf, sizeof(buf), "%0.2f MB", nbytes/xMega);
        } else if (nbytes >= xKilo) {
            snprintf(buf, sizeof(buf), "%0.2f KB", nbytes/xKilo);
        } else {
            snprintf(buf, sizeof(buf), "%0.0f bytes", nbytes);
        }
    }

    strlcpy(str, buf, len);
}

// take a string containing some space separated words.
// return an array of pointers to the null-terminated words.
// Modifies the string arg.
// Returns argc
//
// WARNING: the argv[] pointers are into the original string.
// If that goes away (stack) or is modified,
// the pointers are invalidated.

// TODO: use strtok here

#define NOT_IN_TOKEN                0
#define IN_SINGLE_QUOTED_TOKEN      1
#define IN_DOUBLE_QUOTED_TOKEN      2
#define IN_UNQUOTED_TOKEN           3

int parse_command_line(char* p, char** argv) {
    int state = NOT_IN_TOKEN;
    int argc=0;

    while (*p) {
        switch(state) {
        case NOT_IN_TOKEN:
            if (isspace(*p)) {
            } else if (*p == '\'') {
                p++;
                argv[argc++] = p;
                state = IN_SINGLE_QUOTED_TOKEN;
                break;
            } else if (*p == '\"') {
                p++;
                argv[argc++] = p;
                state = IN_DOUBLE_QUOTED_TOKEN;
                break;
            } else {
                argv[argc++] = p;
                state = IN_UNQUOTED_TOKEN;
            }
            break;
        case IN_SINGLE_QUOTED_TOKEN:
            if (*p == '\'') {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_DOUBLE_QUOTED_TOKEN:
            if (*p == '\"') {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_UNQUOTED_TOKEN:
            if (isspace(*p)) {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        }
        p++;
    }
    argv[argc] = 0;
    return argc;
}

// remove whitespace from start and end of a string
//
void strip_whitespace(string& str) {
    while (1) {
        if (str.length() == 0) break;
        if (!isascii(str[0])) break;
        if (!isspace(str[0])) break;
        str.erase(0, 1);
    }

    int n = (int) str.length();
    while (n>0) {
        if (!isascii(str[n-1])) break;
        if (!isspace(str[n-1])) break;
        n--;
    }
    str.erase(n, str.length()-n);
}

void strip_whitespace(char *str) {
    string s = str;
    strip_whitespace(s);
    strcpy(str, s.c_str());
}

// remove whitespace and quotes from start and end of a string
//
void strip_quotes(string& str) {
    while (1) {
        if (str.length() == 0) break;
        if (str[0] == '"' || str[0] == '\'') {
            str.erase(0, 1);
            continue;
        }
        if (!isascii(str[0])) break;
        if (!isspace(str[0])) break;
        str.erase(0, 1);
    }

    int n = (int) str.length();
    while (n>0) {
        if (str[n-1] == '"' || str[n-1] == '\'') {
            if (str[n-2] != '\\') {
                n--;
                continue;
            }
        }
        if (!isascii(str[n-1])) break;
        if (!isspace(str[n-1])) break;
        n--;
    }
    str.erase(n, str.length()-n);
}

void strip_quotes(char *str) {
    string s = str;
    strip_quotes(s);
    strcpy(str, s.c_str());
}

// This only unescapes some special shell characters used in /etc/os-release
// see https://www.freedesktop.org/software/systemd/man/os-release.html
void unescape_os_release(char* buf) {
    char* out = buf;
    char* in = buf;
    while (*in) {
        if (*in != '\\') {
            *out++ = *in++;
        } else if (*(in+1) == '$') {
            *out++ = '$';
            in += 2;
        } else if (*(in+1) == '\'') {
            *out++ = '\'';
            in += 2;
        } else if (*(in+1) == '"') {
            *out++ = '"';
            in += 2;
        } else if (*(in+1) == '\\') {
            *out++ = '\\';
            in += 2;
        } else if (*(in+1) == '`') {
            *out++ = '`';
            in += 2;
        } else {
            *out++ = *in++;
        }
    }
    *out = 0;
}

// collapse multiple whitespace into one (will not strip_whitespace)
//
void collapse_whitespace(string& str) {
    int n = (int) str.length();
    if (n<2) return;
    for (int i=1; i<n; i++) {
        if (isspace(str[i-1]) && isspace(str[i])) {
            str.erase(i, 1);
            n--; i--;
        }
    }
}

void collapse_whitespace(char *str) {
    string s = str;
    collapse_whitespace(s);
    strcpy(str, s.c_str());
}

char* time_to_string(double t) {
    static char buf[100];
    if (!t) {
        safe_strcpy(buf, "---");
    } else {
        time_t x = (time_t)t;
        struct tm* tm = localtime(&x);
        strftime(buf, sizeof(buf)-1, "%d-%b-%Y %H:%M:%S", tm);
    }
    return buf;
}

char* precision_time_to_string(double t) {
    static char buf[100];
    char finer[16];
    int hundreds_of_microseconds=(int)(10000*(t-(int)t));
    if (hundreds_of_microseconds == 10000) {
        // paranoia -- this should never happen!
        //
        hundreds_of_microseconds=0;
        t+=1.0;
    }
    time_t x = (time_t)t;
    struct tm* tm = localtime(&x);

    strftime(buf, sizeof(buf)-1, "%Y-%m-%d %H:%M:%S", tm);
    snprintf(finer, sizeof(finer), ".%04d", hundreds_of_microseconds);
    safe_strcat(buf, finer);
    return buf;
}

string timediff_format(double diff) {
    char buf[256];
    int tdiff = (int)diff;

    int sex = tdiff % 60;
    tdiff /= 60;
    if (!tdiff) {
        snprintf(buf, sizeof(buf), "00:00:%02d", sex);
        return buf;
    }

    int min = tdiff % 60;
    tdiff /= 60;
    if (!tdiff) {
        snprintf(buf, sizeof(buf), "00:%02d:%02d", min, sex);
        return buf;
    }

    int hours = tdiff % 24;
    tdiff /= 24;
    if (!tdiff) {
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hours, min, sex);
        return buf;
    }

    snprintf(buf, sizeof(buf), "%d days %02d:%02d:%02d", tdiff, hours, min, sex);
    return buf;

}

void mysql_timestamp(double dt, char* p) {
    struct tm* tmp;
    time_t t = (time_t)dt;
    tmp = localtime(&t);     // MySQL timestamps are in local time
    sprintf(p, "%4d%02d%02d%02d%02d%02d",
        tmp->tm_year+1900, tmp->tm_mon+1, tmp->tm_mday,
        tmp->tm_hour, tmp->tm_min, tmp->tm_sec
    );
}

// Return a text-string description of a given error.
// Must be kept consistent with error_numbers.h
//
const char* boincerror(int which_error) {
    switch (which_error) {
        case BOINC_SUCCESS: return "Success";
        case ERR_SELECT: return "select() failed";
        case ERR_MALLOC: return "malloc() failed";
        case ERR_READ: return "read() failed";
        case ERR_WRITE: return "write() failed";
        case ERR_FREAD: return "fread() failed";
        case ERR_FWRITE: return "fwrite() failed";
        case ERR_IO: return "system I/O error";
        case ERR_CONNECT: return "connect() failed";
        case ERR_FOPEN: return "fopen() failed";
        case ERR_RENAME: return "rename() failed";
        case ERR_UNLINK: return "unlink() failed";
        case ERR_OPENDIR: return "opendir() failed";
        case ERR_XML_PARSE: return "unexpected XML tag or syntax";
        case ERR_GETHOSTBYNAME: return "can't resolve hostname";
        case ERR_GIVEUP_DOWNLOAD: return "file download timed out";
        case ERR_GIVEUP_UPLOAD: return "file upload timed out";
        case ERR_NULL: return "unexpected null pointer";
        case ERR_NEG: return "unexpected negative value";
        case ERR_BUFFER_OVERFLOW: return "buffer overflow";
        case ERR_MD5_FAILED: return "md5 checksum failed for file";
        case ERR_RSA_FAILED: return "RSA key check failed for file";
        case ERR_OPEN: return "open() failed";
        case ERR_DUP2: return "dup() failed";
        case ERR_NO_SIGNATURE: return "no signature";
        case ERR_THREAD: return "thread failure";
        case ERR_SIGNAL_CATCH: return "caught signal";
        case ERR_BAD_FORMAT: return "bad file format";
        case ERR_UPLOAD_TRANSIENT: return "transient upload error";
        case ERR_UPLOAD_PERMANENT: return "permanent upload error";
        case ERR_IDLE_PERIOD: return "user preferences say can't start work";
        case ERR_ALREADY_ATTACHED: return "already attached to project";
        case ERR_FILE_TOO_BIG: return "file size too big";
        case ERR_GETRUSAGE: return "getrusage() failed";
        case ERR_BENCHMARK_FAILED: return "benchmark failed";
        case ERR_BAD_HEX_FORMAT: return "hex format key data bad";
        case ERR_GETADDRINFO: return "getaddrinfo() failed";
        case ERR_DB_NOT_FOUND: return "no database rows found in lookup/enumerate";
        case ERR_DB_NOT_UNIQUE: return "database lookup not unique";
        case ERR_DB_CANT_CONNECT: return "can't connect to database";
        case ERR_GETS: return "gets()/fgets() failed";
        case ERR_SCANF: return "scanf()/fscanf() failed";
        case ERR_READDIR: return "readdir() failed";
        case ERR_SHMGET: return "shmget() failed";
        case ERR_SHMCTL: return "shmctl() failed";
        case ERR_SHMAT: return "shmat() failed";
        case ERR_FORK: return "fork() failed";
        case ERR_EXEC: return "exec() failed";
        case ERR_NOT_EXITED: return "process didn't exit";
        case ERR_NOT_IMPLEMENTED: return "system call not implemented";
        case ERR_GETHOSTNAME: return "gethostname() failed";
        case ERR_NETOPEN: return "netopen() failed";
        case ERR_SOCKET: return "socket() failed";
        case ERR_FCNTL: return "fcntl() failed";
        case ERR_AUTHENTICATOR: return "authentication error";
        case ERR_SCHED_SHMEM: return "scheduler shared memory contents bad";
        case ERR_ASYNCSELECT: return "async select() failed";
        case ERR_BAD_RESULT_STATE: return "bad result state";
        case ERR_DB_CANT_INIT: return "can't init database";
        case ERR_NOT_UNIQUE: return "state files have redundant entries";
        case ERR_NOT_FOUND: return "not found";
        case ERR_NO_EXIT_STATUS: return "no exit status in scheduler request";
        case ERR_FILE_MISSING: return "file missing";
        case ERR_KILL: return "kill() or TerminateProcess() failed";
        case ERR_SEMGET: return "semget() failed";
        case ERR_SEMCTL: return "semctl() failed";
        case ERR_SEMOP: return "semop() failed";
        case ERR_FTOK: return "ftok() failed";
        case ERR_SOCKS_UNKNOWN_FAILURE: return "SOCKS: unknown error";
        case ERR_SOCKS_REQUEST_FAILED: return "SOCKS: request failed";
        case ERR_SOCKS_BAD_USER_PASS: return "SOCKS: bad user password";
        case ERR_SOCKS_UNKNOWN_SERVER_VERSION: return "SOCKS: unknown server version";
        case ERR_SOCKS_UNSUPPORTED: return "SOCKS: unsupported";
        case ERR_SOCKS_CANT_REACH_HOST: return "SOCKS: can't reach host";
        case ERR_SOCKS_CONN_REFUSED: return "SOCKS: connection refused";
        case ERR_TIMER_INIT: return "timer init";
        case ERR_INVALID_PARAM: return "invalid parameter";
        case ERR_SIGNAL_OP: return "signal op";
        case ERR_BIND: return "bind() failed";
        case ERR_LISTEN: return "listen() failed";
        case ERR_TIMEOUT: return "timeout";
        case ERR_PROJECT_DOWN: return "project down";
        case ERR_RESULT_START: return "result start failed";
        case ERR_RESULT_DOWNLOAD: return "result download failed";
        case ERR_RESULT_UPLOAD: return "result upload failed";
        case ERR_BAD_USER_NAME: return "bad username";
        case ERR_INVALID_URL: return "invalid URL";
        case ERR_MAJOR_VERSION: return "bad major version";
        case ERR_NO_OPTION: return "no option";
        case ERR_MKDIR: return "mkdir() failed";
        case ERR_INVALID_EVENT: return "invalid event";
        case ERR_ALREADY_RUNNING: return "already running";
        case ERR_NO_APP_VERSION: return "no app version";
        case ERR_WU_USER_RULE: return "user already did result for this workunit";
        case ERR_ABORTED_VIA_GUI: return "result aborted via GUI";
        case ERR_INSUFFICIENT_RESOURCE: return "insufficient resources";
        case ERR_RETRY: return "retry";
        case ERR_WRONG_SIZE: return "wrong buffer size";
        case ERR_USER_PERMISSION: return "user permission";
        case ERR_BAD_EMAIL_ADDR: return "bad email address";
        case ERR_BAD_PASSWD: return "bad password";
        case ERR_SHMEM_NAME: return "can't get shared mem segment name";
        case ERR_NO_NETWORK_CONNECTION: return "no available network connection";
        case ERR_IN_PROGRESS: return "operation in progress";
        case ERR_NONUNIQUE_EMAIL: return "email already registered";
        case ERR_ACCT_CREATION_DISABLED: return "account creation disabled";
        case ERR_ATTACH_FAIL_INIT: return "Couldn't start master page download";
        case ERR_ATTACH_FAIL_DOWNLOAD: return "Couldn't download master page";
        case ERR_ATTACH_FAIL_PARSE: return "Couldn't parse master page";
        case ERR_ATTACH_FAIL_BAD_KEY: return "Invalid account key";
        case ERR_ATTACH_FAIL_FILE_WRITE: return "Couldn't write account file";
        case ERR_ATTACH_FAIL_SERVER_ERROR: return "Couldn't attach because of server error";
        case ERR_SIGNING_KEY: return "signing key failure";
        case ERR_FFLUSH: return "fflush() failed";
        case ERR_FSYNC: return "fsync() failed";
        case ERR_TRUNCATE: return "truncate() failed";
        case ERR_WRONG_URL: return "wrong URL";
        case ERR_DUP_NAME: return "coprocs with duplicate names detected";
        case ERR_FILE_WRONG_SIZE: return "file has the wrong size";
        case ERR_GETGRNAM: return "getgrnam() failed";
        case ERR_CHOWN: return "chown() failed";
        case ERR_HTTP_PERMANENT: return "permanent HTTP error";
        case ERR_HTTP_TRANSIENT: return "transient HTTP error";
        case ERR_BAD_FILENAME: return "file name is empty or has '..'";
        case ERR_TOO_MANY_EXITS: return "application exited too many times";
        case ERR_RMDIR: return "rmdir() failed";
        case ERR_SYMLINK: return "symlink() failed";
        case ERR_DB_CONN_LOST: return "DB connection lost during enumeration";
        case ERR_CRYPTO: return "encryption error";
        case ERR_ABORTED_ON_EXIT: return "job was aborted on client exit";
        case ERR_PROC_PARSE: return "a /proc entry was not parsed correctly";
        case ERR_STATFS: return "statfs() failed";
        case ERR_PIPE: return "pipe() failed";
        case ERR_NEED_HTTPS: return "HTTPS needed";
        case ERR_CHMOD : return "chmod() failed";
        case ERR_STAT : return "stat() failed";
        case ERR_FCLOSE : return "fclose() failed";
        case ERR_INVALID_STATE: return "invalid state";
        case HTTP_STATUS_NOT_FOUND: return "HTTP file not found";
        case HTTP_STATUS_PROXY_AUTH_REQ: return "HTTP proxy authentication failure";
        case HTTP_STATUS_RANGE_REQUEST_ERROR: return "HTTP range request error";
        case HTTP_STATUS_EXPECTATION_FAILED: return "HTTP expectation failed";
        case HTTP_STATUS_INTERNAL_SERVER_ERROR: return "HTTP internal server error";
        case HTTP_STATUS_NOT_IMPLEMENTED: return "HTTP not implemented";
        case HTTP_STATUS_BAD_GATEWAY: return "HTTP bad gateway";
        case HTTP_STATUS_SERVICE_UNAVAILABLE: return "HTTP service unavailable";
        case HTTP_STATUS_GATEWAY_TIMEOUT: return "HTTP gateway timeout";
        case ERR_ACCT_REQUIRE_CONSENT: return "This project requires to consent to its terms of use";
    }
    static char buf[128];
    snprintf(buf, sizeof(buf), "Error %d", which_error);
    return buf;
}

const char* network_status_string(int n) {
    switch (n) {
    case NETWORK_STATUS_ONLINE: return "online";
    case NETWORK_STATUS_WANT_CONNECTION: return "need connection";
    case NETWORK_STATUS_WANT_DISCONNECT: return "don't need connection";
    case NETWORK_STATUS_LOOKUP_PENDING: return "reference site lookup pending";
    default: return "unknown";
    }
}

const char* rpc_reason_string(int reason) {
    switch (reason) {
    case RPC_REASON_USER_REQ: return "Requested by user";
    case RPC_REASON_NEED_WORK: return "To fetch work";
    case RPC_REASON_RESULTS_DUE: return "To report completed tasks";
    case RPC_REASON_TRICKLE_UP: return "To send trickle-up message";
    case RPC_REASON_ACCT_MGR_REQ: return "Requested by account manager";
    case RPC_REASON_INIT: return "Project initialization";
    case RPC_REASON_PROJECT_REQ: return "Requested by project";
    default: return "Unknown reason";
    }
}

const char* suspend_reason_string(int reason) {
    switch (reason) {
    case SUSPEND_REASON_BATTERIES: return "on batteries";
    case SUSPEND_REASON_USER_ACTIVE: return "computer is in use";
    case SUSPEND_REASON_USER_REQ: return "user request";
    case SUSPEND_REASON_TIME_OF_DAY: return "time of day";
    case SUSPEND_REASON_BENCHMARKS: return "CPU benchmarks in progress";
    case SUSPEND_REASON_DISK_SIZE: return "need disk space - check preferences";
    case SUSPEND_REASON_NO_RECENT_INPUT: return "no recent user activity";
    case SUSPEND_REASON_INITIAL_DELAY: return "initial delay";
    case SUSPEND_REASON_EXCLUSIVE_APP_RUNNING: return "an exclusive app is running";
    case SUSPEND_REASON_CPU_USAGE: return "CPU is busy";
    case SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED: return "network transfer limit exceeded";
    case SUSPEND_REASON_OS: return "requested by operating system";
    case SUSPEND_REASON_WIFI_STATE: return "not connected to WiFi network";
    case SUSPEND_REASON_BATTERY_CHARGING: return "battery low";
    case SUSPEND_REASON_BATTERY_OVERHEATED: return "battery thermal protection";
    case SUSPEND_REASON_NO_GUI_KEEPALIVE: return "GUI not active";
    }
    return "unknown reason";
}

const char* run_mode_string(int mode) {
    switch (mode) {
    case RUN_MODE_ALWAYS: return "always";
    case RUN_MODE_AUTO: return "according to prefs";
    case RUN_MODE_NEVER: return "never";
    }
    return "unknown";
}

const char* battery_state_string(int state) {
    switch (state) {
    case BATTERY_STATE_DISCHARGING: return "discharging";
    case BATTERY_STATE_CHARGING: return "charging";
    case BATTERY_STATE_FULL: return "full";
    case BATTERY_STATE_OVERHEATED: return "overheated";
    }
    return "unknown";
}

const char* result_client_state_string(int state) {
    switch (state) {
    case RESULT_NEW: return "new";
    case RESULT_FILES_DOWNLOADING: return "downloading";
    case RESULT_FILES_DOWNLOADED: return "downloaded";
    case RESULT_COMPUTE_ERROR: return "compute error";
    case RESULT_FILES_UPLOADING: return "uploading";
    case RESULT_FILES_UPLOADED: return "uploaded";
    case RESULT_ABORTED: return "aborted";
    case RESULT_UPLOAD_FAILED: return "upload failed";
    }
    return "unknown";
}

const char* result_scheduler_state_string(int state) {
    switch (state) {
    case CPU_SCHED_UNINITIALIZED: return "uninitialized";
    case CPU_SCHED_PREEMPTED: return "preempted";
    case CPU_SCHED_SCHEDULED: return "scheduled";
    }
    return "unknown";
}

const char* active_task_state_string(int state) {
    switch (state) {
    case PROCESS_UNINITIALIZED: return "UNINITIALIZED";
    case PROCESS_EXECUTING: return "EXECUTING";
    case PROCESS_SUSPENDED: return "SUSPENDED";
    case PROCESS_ABORT_PENDING: return "ABORT_PENDING";
    case PROCESS_EXITED: return "EXITED";
    case PROCESS_WAS_SIGNALED: return "WAS_SIGNALED";
    case PROCESS_EXIT_UNKNOWN: return "EXIT_UNKNOWN";
    case PROCESS_ABORTED: return "ABORTED";
    case PROCESS_COULDNT_START: return "COULDNT_START";
    case PROCESS_QUIT_PENDING: return "QUIT_PENDING";
    case PROCESS_COPY_PENDING: return "COPY_PENDING";
    }
    return "Unknown";
}

const char* batch_state_string(int state) {
    switch (state) {
    case BATCH_STATE_INIT: return "uninitialized";
    case BATCH_STATE_IN_PROGRESS: return "in progress";
    case BATCH_STATE_COMPLETE: return "completed";
    case BATCH_STATE_ABORTED: return "aborted";
    case BATCH_STATE_RETIRED: return "retired";
    }
    return "unknown";
}

// string substitution:
// haystack is the input string
// out is the output buffer
// out_len is the length of the output buffer
// needle is string to search for within the haystack
// target is string to replace with
//
int string_substitute(
    const char* haystack, char* out, int out_len,
    const char* needle, const char* target
) {
    int i=0, j=0;
    int needle_len = (int)strlen(needle);
    int target_len = (int)strlen(target);
    int retval = 0;

    while (haystack[i]) {
        if (j+target_len >= out_len-1) {
            retval = ERR_BUFFER_OVERFLOW;
            break;
        }
        if (!strncmp(&haystack[i], needle, needle_len)){
            strlcpy(out+j, target, out_len-((out+j)-out));
            i += needle_len;
            j += target_len;
        } else {
            out[j++] = haystack[i++];
        }
    }
    out[j] = 0;
    return retval;
}

inline void remove_str(char* p, const char* str) {
    size_t n = strlen(str);
    while (1) {
        p = strstr(p, str);
        if (!p) break;
        strcpy_overlap(p, p+n);
    }
}

// remove _(" and ") from string
//
void strip_translation(char* p) {
    remove_str(p, "_(\"");
    remove_str(p, "\")");
}

char* lf_terminate(char* p) {
    int n = (int)strlen(p);
    if (p[n-1] == '\n') {
        return p;
    }
    p = (char*)realloc(p, n+2);
    p[n] = '\n';
    p[n+1] = 0;
    return p;
}

void parse_serialnum(char* in, char* boinc, char* vbox, char* coprocs) {
    strcpy(boinc, "");
    strcpy(vbox, "");
    strcpy(coprocs, "");
    while (*in) {
        if (*in != '[') break;      // format error
        char* p = strchr(in, ']');
        if (!p) break;              // format error
        p++;
        char c = *p;
        *p = 0;
        if (strstr(in, "BOINC")) {
            strcpy(boinc, in);
        } else if (strstr(in, "vbox")) {
            strcpy(vbox, in);
        } else {
            strcat(coprocs, in);
        }
        *p = c;
        in = p;
    }
}

vector<string> split(string s, char delim) {
    vector<string> result;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

// check whether filename is legit
// - can't start with /
// - can't have control chars
// - can't have ..
//
bool is_valid_filename(const char* name) {
    size_t n = strlen(name);
    for (size_t i=0; i<n; i++) {
        if (iscntrl(name[i])) {
            return false;
        }
    }
    if (strstr(name, "..")) {
        return false;
    }
    if (name[0] == '/') {
        return false;
    }
    return true;
}

// get the name part of a filepath
// returns:
//   0 on success
//  -1 when fpath is empty
//  -2 when fpath is a directory
int path_to_filename(string fpath, string& fname) {
    std::string::size_type n;
    if (fpath.size() == 0) {
        return -1;
    }
    n = fpath.rfind("/");
    if (n == std::string::npos) {
        fname = fpath;
    } else if (n == fpath.size()-1) {
        return -2;
    } else {
        fname = fpath.substr(n+1);
    }
    return 0;
}

// get the name part of a filepath
//
// wrapper for path_to_filename(string, string&)
int path_to_filename(string fpath, char* &fname) {
    string name;
    int retval = path_to_filename(fpath, name);
    if (retval) {
        return retval;
    }
    fname = new char[name.size()+1];
    strcpy(fname, name.c_str());
    return 0;
}
