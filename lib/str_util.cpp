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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#endif
#ifdef _WIN32
#include "win_util.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <string>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#if HAVE_ALLOCA_H
#include "alloca.h"
#endif
#endif

#include "error_numbers.h"
#include "common_defs.h"
#include "filesys.h"
#include "str_replace.h"
#include "str_util.h"

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

using std::string;

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
const char *strcasestr(const char *s1, const char *s2) {
    char *needle=NULL, *haystack=NULL, *p=NULL;
    bool need_free = false;
    // Is alloca() really less likely to fail with out of memory error
    // than strdup?
#if HAVE_STRDUPA
    haystack=strdupa(s1);
    needle=strdupa(s2);
#elif HAVE_ALLOCA_H || HAVE_ALLOCA
    haystack=(char *)alloca(strlen(s1)+1);
    needle=(char *)alloca(strlen(s2)+1);
    if (needle && haystack) {
        strlcpy(haystack,s1,strlen(s1)+1);
        strlcpy(needle,s2,strlen(s2)+1);
    }
#elif HAVE_STRDUP
    haystack=strdup(s1);
    needle=strdup(s1)
    need_free = true;
#else
    haystack=(char *)malloc(strlen(s1)+1);
    needle=(char *)malloc(strlen(s2)+1);
    if (needle && haystack) {
        strlcpy(haystack,s1,strlen(s1)+1);
        strlcpy(needle,s2,strlen(s2)+1);
    }
    need_free = true;
#endif
    if (needle && haystack) {
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
    }
    if (need_free) {
        if (needle) free(needle);
        if (haystack) free(haystack);
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
        sprintf( year_buf, "%.3f yr ", years );
    } else if (years > 1 && smallest_timescale < 4) {
        sprintf( year_buf, "%d yr ", (int)years );
    } else {
        strcpy( year_buf, "" );
    }

    if (smallest_timescale==3) {
        sprintf( day_buf, "%.2f day%s ", days, (days>1?"s":"") );
    } else if (days > 1 && smallest_timescale < 3) {
        sprintf( day_buf, "%d day%s ", (int)days, (days>1?"s":"") );
    } else {
        strcpy( day_buf, "" );
    }

    if (smallest_timescale==2) {
        sprintf( hour_buf, "%.2f hr ", hours );
    } else if (hours > 1 && smallest_timescale < 2) {
        sprintf( hour_buf, "%d hr ", (int)hours );
    } else {
        strcpy( hour_buf, "" );
    }

    if (smallest_timescale==1) {
        sprintf( min_buf, "%.2f min ", minutes );
    } else if (minutes > 1 && smallest_timescale < 1) {
        sprintf( min_buf, "%d min ", (int)minutes );
    } else {
        strcpy( min_buf, "" );
    }

    if (smallest_timescale==0) {
        sprintf( sec_buf, "%.2f sec ", seconds );
    } else if (seconds > 1 && smallest_timescale < 0) {
        sprintf( sec_buf, "%d sec ", (int)seconds );
    } else {
        strcpy( sec_buf, "" );
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
            sprintf(buf, "%0.2f/%0.2f TB", nbytes/xTera, total_bytes/xTera);
        } else if (total_bytes >= xGiga) {
            sprintf(buf, "%0.2f/%0.2f GB", nbytes/xGiga, total_bytes/xGiga);
        } else if (total_bytes >= xMega) {
            sprintf(buf, "%0.2f/%0.2f MB", nbytes/xMega, total_bytes/xMega);
        } else if (total_bytes >= xKilo) {
            sprintf(buf, "%0.2f/%0.2f KB", nbytes/xKilo, total_bytes/xKilo);
        } else {
            sprintf(buf, "%0.0f/%0.0f bytes", nbytes, total_bytes);
        }
    } else {
        if (nbytes >= xTera) {
            sprintf(buf, "%0.2f TB", nbytes/xTera);
        } else if (nbytes >= xGiga) {
            sprintf(buf, "%0.2f GB", nbytes/xGiga);
        } else if (nbytes >= xMega) {
            sprintf(buf, "%0.2f MB", nbytes/xMega);
        } else if (nbytes >= xKilo) {
            sprintf(buf, "%0.2f KB", nbytes/xKilo);
        } else {
            sprintf(buf, "%0.0f bytes", nbytes);
        }
    }

    strlcpy(str, buf, len);
}

// take a string containing some space separated words.
// return an array of pointers to the null-terminated words.
// Modifies the string arg.
// Returns argc
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
void strip_whitespace(char *str) {
    char *s = str;
    while (*s) {
        if (!isascii(*s)) break;
        if (!isspace(*s)) break;
        s++;
    }
    if (s != str) strcpy_overlap(str, s);

    size_t n = strlen(str);
    while (n>0) {
        n--;
        if (!isascii(str[n])) break;
        if (!isspace(str[n])) break;
        str[n] = 0;
    }
}

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

char* time_to_string(double t) {
    static char buf[100];
    time_t x = (time_t)t;
    struct tm* tm = localtime(&x);
    strftime(buf, sizeof(buf)-1, "%d-%b-%Y %H:%M:%S", tm);
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
    sprintf(finer, ".%04d", hundreds_of_microseconds);
    strcat(buf, finer);
    return buf;
}

string timediff_format(double diff) {
    char buf[256];
    int tdiff = (int)diff;

    int sex = tdiff % 60;
    tdiff /= 60;
    if (!tdiff) {
        sprintf(buf, "%d sec", sex);
        return buf;
    }

    int min = tdiff % 60;
    tdiff /= 60;
    if (!tdiff) {
        sprintf(buf, "%d min %d sec", min, sex);
        return buf;
    }

    int hours = tdiff % 24;
    tdiff /= 24;
    if (!tdiff) {
        sprintf(buf, "%d hr %d min %d sec", hours, min, sex);
        return buf;
    }

    int days = tdiff % 7;
    tdiff /= 7;
    if (!tdiff) {
        sprintf(buf, "%d days %d hr %d min %d sec", days, hours, min, sex);
        return buf;
    }

    sprintf(buf, "%d weeks %d days %d hrs %d min %d sec", (int)tdiff, days, hours, min, sex);
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
        case ERR_UPLOAD_TRANSIENT: return "transient upload error";
        case ERR_UPLOAD_PERMANENT: return "permanent upload error";
        case ERR_IDLE_PERIOD: return "user preferences say can't start work";
        case ERR_ALREADY_ATTACHED: return "already attached to project";
        case ERR_FILE_TOO_BIG: return "file size too big";
        case ERR_GETRUSAGE: return "getrusage() failed";
        case ERR_BENCHMARK_FAILED: return "benchmark failed";
        case ERR_BAD_HEX_FORMAT: return "hex format key data bad";
        case ERR_DB_NOT_FOUND: return "no database rows found in lookup/enumerate";
        case ERR_DB_NOT_UNIQUE: return "database lookup not unique";
        case ERR_DB_CANT_CONNECT: return "can't connect to database";
        case ERR_GETS: return "gets()/fgets() failedj";
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
        case ERR_WRONG_SIZE: return "wrong size";
        case ERR_USER_PERMISSION: return "user permission";
        case ERR_BAD_EMAIL_ADDR: return "bad email address";
        case ERR_BAD_PASSWD: return "bad password";
        case ERR_SHMEM_NAME: return "can't get shared mem segment name";
        case ERR_NO_NETWORK_CONNECTION: return "no available network connection";
        case ERR_IN_PROGRESS: return "operation in progress";
        case ERR_ACCT_CREATION_DISABLED: return "account creation disabled";
        case ERR_ATTACH_FAIL_INIT: return "Couldn't start master page download";
        case ERR_ATTACH_FAIL_DOWNLOAD: return "Couldn't download master page";
        case ERR_ATTACH_FAIL_PARSE: return "Couldn't parse master page";
        case ERR_ATTACH_FAIL_BAD_KEY: return "Invalid account key";
        case ERR_ATTACH_FAIL_FILE_WRITE: return "Couldn't write account file";
        case ERR_FFLUSH: return "fflush() failed";
        case ERR_FSYNC: return "fsync() failed";
        case ERR_TRUNCATE: return "truncate() failed";
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
        case 404: return "HTTP file not found";
        case 407: return "HTTP proxy authentication failure";
        case 416: return "HTTP range request error";
        case 500: return "HTTP internal server error";
        case 501: return "HTTP not implemented";
        case 502: return "HTTP bad gateway";
        case 503: return "HTTP service unavailable";
        case 504: return "HTTP gateway timeout";
    }
    static char buf[128];
    sprintf(buf, "Error %d", which_error);
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
    case SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED: return "network bandwidth limit exceeded";
    case SUSPEND_REASON_OS: return "requested by operating system";
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

#ifdef WIN32

// get message for last error
//
char* windows_error_string(char* pszBuf, int iSize) {
    DWORD dwRet;
    LPSTR lpszTemp = NULL;

    dwRet = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        GetLastError(),
        LANG_NEUTRAL,
        (LPSTR)&lpszTemp,
        0,
        NULL
    );

    // supplied buffer is not long enough
    if ( !dwRet || ( (long)iSize < (long)dwRet+14 ) ) {
        pszBuf[0] = '\0';
    } else {
        lpszTemp[lstrlenA(lpszTemp)-2] = '\0';  //remove cr and newline character
        sprintf ( pszBuf, "%s (0x%x)", lpszTemp, GetLastError() );
    }

    if ( lpszTemp ) {
        LocalFree((HLOCAL) lpszTemp );
    }

    return pszBuf;
}

// get message for given error
//
char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize
) {
    DWORD dwRet;
    LPSTR lpszTemp = NULL;

    dwRet = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        dwError,
        LANG_NEUTRAL,
        (LPSTR)&lpszTemp,
        0,
        NULL
    );

    // supplied buffer is not long enough
    if ( !dwRet || ( (long)iSize < (long)dwRet+14 ) ) {
        pszBuf[0] = '\0';
    } else {
        lpszTemp[lstrlenA(lpszTemp)-2] = '\0';  //remove cr and newline character
        sprintf( pszBuf, "%s (0x%x)", lpszTemp, dwError );
    }

    if ( lpszTemp ) {
        LocalFree((HLOCAL) lpszTemp );
    }

    return pszBuf;
}
#endif

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
            strcpy(out+j, target);
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

// remove _( and ") from string
//
void strip_translation(char* p) {
    remove_str(p, "_(\"");
    remove_str(p, "\")");
}
