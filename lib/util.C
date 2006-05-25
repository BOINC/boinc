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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef M_LN2
#define M_LN2      0.693147180559945309417
#endif

#ifndef _WIN32
#include "config.h"
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>
#include <unistd.h>
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#endif


#include "error_numbers.h"
#include "filesys.h"
#include "util.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

using std::min;
using std::string;

// Use this instead of strncpy().
// Result will always be null-terminated, and it's faster.
// see http://www.gratisoft.us/todd/papers/strlcpy.html
//
#if !defined(HAVE_STRLCPY)
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

#if !defined(HAVE_STRLCAT)
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

#define EPOCHFILETIME_SEC (11644473600.)
#define TEN_MILLION 10000000.

// return time of day (seconds since 1970) as a double
//
double dtime() {
#ifdef _WIN32
    LARGE_INTEGER time;
    FILETIME sysTime;
    double t;
    GetSystemTimeAsFileTime(&sysTime);
    time.LowPart = sysTime.dwLowDateTime;
    time.HighPart = sysTime.dwHighDateTime;  // Time is in 100 ns units
    t = (double)time.QuadPart;    // Convert to 1 s units
    t /= TEN_MILLION;                /* In seconds */
    t -= EPOCHFILETIME_SEC;     /* Offset to the Epoch time */
    return t;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + (tv.tv_usec/1.e6);
#endif
}

// return time today 0:00 in seconds since 1970 as a double
//
double dday() {
    const double seconds_per_day=24*60*60;
    const double now=dtime();

    return (now-fmod(now,seconds_per_day));
}

// do sanity check on a time, replace with now if bad
// We switched to using all UNIX times on 12/1/04.
// During the transition, times in client_state.xml may be
// in Windows (1601-based) format.  Fix them here.
//
void validate_time(double& t) {
    if (t==0) return;
    double now = dtime();
    if (t > now+86400*1000) {
        t -=  EPOCHFILETIME_SEC;
    }
}

// sleep for a specified number of seconds
//
void boinc_sleep(double seconds) {
#ifdef _WIN32
    ::Sleep((int)(1000*seconds));
#else
    unsigned int rem = (int) seconds;
    while (1) {
        rem = sleep(rem);
        if (rem == 0) break;
        if (rem > seconds) break;   // paranoia
    }
    int x = (int)fmod(seconds*1000000,1000000);
    if (x) usleep(x);
#endif
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

static char x2c(char *what) {
    register char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

void c2x(char *what) {
    char buf[3];
    char num = atoi(what);
    char d1 = num / 16;
    char d2 = num % 16;
    int abase1, abase2;
    if (d1 < 10) abase1 = 48;
    else abase1 = 55;
    if (d2 < 10) abase2 = 48;
    else abase2 = 55;
    buf[0] = d1+abase1;
    buf[1] = d2+abase2;
    buf[2] = 0;

    strcpy(what, buf);
}

// remove whitespace from start and end of a string
//
void strip_whitespace(char *str) {
    int n;
    while (1) {
        if (!str[0]) break;
        if (!isascii(str[0])) break;
        if (!isspace(str[0])) break;
        strcpy(str, str+1);
    }
    while (1) {
        n = (int)strlen(str);
        if (n == 0) break;
        if (!isascii(str[n-1])) break;
        if (!isspace(str[n-1])) break;
        str[n-1] = 0;
    }
}

void strip_whitespace(string& str) {
    int n;
    while (1) {
        if (str.length() == 0) break;
        if (!isascii(str[0])) break;
        if (!isspace(str[0])) break;
        str.erase(0, 1);
    }
    while (1) {
        n = (int)str.length();
        if (n == 0) break;
        if (!isascii(str[n-1])) break;
        if (!isspace(str[n-1])) break;
        str.erase(n-1, 1);
    }
}

#if 0
bool starts_with(const char* str, const char* prefix) {
    if (strstr(str, prefix) == str) return true;
    return false;
}

bool ends_with(const char* str, const char* suffix) {
    size_t nsuff = strlen(suffix);
    size_t nstr = strlen(str);

    if (nsuff > nstr) return false;
    if (strcmp(str+nstr-nsuff, suffix)) return false;
    return true;
}
#endif

void unescape_url(char *url) {
    int x,y;

    for (x=0,y=0;url[y];++x,++y) {
        if ((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
    }
    url[x] = '\0';
}

void unescape_url_safe(char *url, int url_size) {
    int x,y;

    for (x=0,y=0; url[y] && (x<url_size);++x,++y) {
        if ((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
    }
    url[x] = '\0';
}

// unescape_url needs to be able to handle potentially hostile
// urls.
void unescape_url(string& url) {
    char buf[1024];
    strncpy(buf, url.c_str(), sizeof(buf));
    unescape_url_safe(buf, sizeof(buf));
    url = buf;
}

void escape_url(char *in, char*out) {
    int x, y;
    for (x=0, y=0; in[x]; ++x) {
        if (isalnum(in[x])) {
            out[y] = in[x];
            ++y;
        } else {
            out[y] = '%';
            ++y;
            out[y] = 0;
            char buf[256];
            sprintf(buf, "%d", (char)in[x]);
            c2x(buf);
            strcat(out, buf);
            y += 2;
        }
    }
    out[y] = 0;
}

void escape_url_safe(const char *in, char*out, int out_size) {
    int x, y;
    for (x=0, y=0; in[x] && (y<out_size); ++x) {
        if (isalnum(in[x])) {
            out[y] = in[x];
            ++y;
        } else {
            out[y] = '%';
            ++y;
            out[y] = 0;
            char buf[256];
            sprintf(buf, "%d", (char)in[x]);
            c2x(buf);
            strcat(out, buf);
            y += 2;
        }
    }
    out[y] = 0;
}

// escape_url needs to be able to handle potentially hostile
// urls
void escape_url(string& url) {
    char buf[1024];
    escape_url_safe(url.c_str(), buf, sizeof(buf));
    url = buf;
}

// Escape a URL for the project directory, cutting off the "http://",
// converting '\' '/' and ' ' to '_',
// and converting the non alphanumeric characters to %XY
// where XY is their hexadecimal equivalent
//
void escape_url_readable(char *in, char* out) {
    int x, y;
    char *temp;

    temp = strstr(in,"://");
    if (temp) {
        in = temp + strlen("://");
    }
    for (x=0, y=0; in[x]; ++x) {
        if (isalnum(in[x]) || in[x]=='.' || in[x]=='-' || in[x]=='_') {
            out[y] = in[x];
            ++y;
        } else {
            out[y] = '_';
            ++y;
        }
    }
    out[y] = 0;
}


// Canonicalize a master url.
//   - Convert the first part of a URL (before the "://") to http://,
// or prepend it
//   - Remove double slashes in the rest
//   - Add a trailing slash if necessary
//
void canonicalize_master_url(char* url) {
    char buf[1024];
    size_t n;
	bool bSSL = false; // keep track if they sent in https://

    char *p = strstr(url, "://");
    if (p) {
		bSSL = (bool) (p == url + 5);
		strcpy(buf, p+3);
    } else {
        strcpy(buf, url);
    }
    while (1) {
        p = strstr(buf, "//");
        if (!p) break;
        strcpy(p, p+1);
    }
    n = strlen(buf);
    if (buf[n-1] != '/') {
        strcat(buf, "/");
    }
	sprintf(url, "http%s://%s", (bSSL ? "s" : ""), buf); // CMC Here -- add SSL if needed
}

void canonicalize_master_url(string& url) {
    char buf[1024];
    strcpy(buf, url.c_str());
    canonicalize_master_url(buf);
    url = buf;
}

// is the string a valid master URL, in canonical form?
//
bool valid_master_url(char* buf) {
    char* p, *q;
    size_t n;
	bool bSSL = false;

    p = strstr(buf, "http://");
	if (p != buf) {
		// allow https
	    p = strstr(buf, "https://");
		if (p == buf) {
			bSSL = true;
		} else {
			return false; // no http or https, it's bad!
	    }
	}
	q = p+strlen(bSSL ? "https://" : "http://");
    p = strstr(q, ".");
    if (!p) return false;
    if (p == q) return false;
    q = p+1;
    p = strstr(q, "/");
    if (!p) return false;
    if (p == q) return false;
    n = strlen(buf);
    if (buf[n-1] != '/') return false;
    return true;
}

char* time_to_string(double t) {
    static char buf[100];
    time_t x = (time_t)t;
    struct tm* tm = localtime(&x);
    strftime(buf, sizeof(buf)-1, "%Y-%m-%d %H:%M:%S", tm);
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
        sprintf(buf, "%d seconds", sex);
        return buf;
    }

    int min = tdiff % 60;
    tdiff /= 60;
    if (!tdiff) {
        sprintf(buf, "%d minutes and %d seconds", min, sex);
        return buf;
    }

    int hours = tdiff % 24;
    tdiff /= 24;
    if (!tdiff) {
        sprintf(buf, "%d hours, %d minutes and %d seconds", hours, min, sex);
        return buf;
    }

    int days = tdiff % 7;
    tdiff /= 7;
    if (!tdiff) {
        sprintf(buf, "%d days, %d hours, %d minutes and %d seconds", days, hours, min, sex);
        return buf;
    }

    sprintf(buf, "%d weeks, %d days, %d hours, %d minutes and %d seconds", (int)tdiff, days, hours, min, sex);
    return buf;
}

// read entire file into string
int read_file_string(const char* pathname, string& result) {
    result.erase();
    FILE* f;
    char buf[256];

    f = fopen(pathname, "r");
    if (!f) return ERR_FOPEN;

    while (fgets(buf, 256, f)) result += buf;
    fclose(f);
    return 0;
}

#ifdef WIN32

//
//  FUNCTION: windows_error_string
//
//  PURPOSE: copies error message text to string
//
//  PARAMETERS:
//    pszBuf - destination buffer
//    iSize - size of buffer
//
//  RETURN VALUE:
//    destination buffer
//
//  COMMENTS:
//
char* windows_error_string( char* pszBuf, int iSize ) {
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        GetLastError(),
        LANG_NEUTRAL,
        (LPTSTR)&lpszTemp,
        0,
        NULL
    );

    // supplied buffer is not long enough
    if ( !dwRet || ( (long)iSize < (long)dwRet+14 ) ) {
        pszBuf[0] = TEXT('\0');
    } else {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
        sprintf( pszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError() );
    }

    if ( lpszTemp ) {
        LocalFree((HLOCAL) lpszTemp );
    }

    return pszBuf;
}


//
//  FUNCTION: windows_format_error_string
//
//  PURPOSE: copies error message text to string
//
//  PARAMETERS:
//    dwError - the error value to look up
//    pszBuf - destination buffer
//    iSize - size of buffer
//
//  RETURN VALUE:
//    destination buffer
//
//  COMMENTS:
//
char* windows_format_error_string( unsigned long dwError, char* pszBuf, int iSize )
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        dwError,
        LANG_NEUTRAL,
        (LPTSTR)&lpszTemp,
        0,
        NULL
    );

    // supplied buffer is not long enough
    if ( !dwRet || ( (long)iSize < (long)dwRet+14 ) ) {
        pszBuf[0] = TEXT('\0');
    } else {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
        sprintf( pszBuf, TEXT("%s (0x%x)"), lpszTemp, dwError );
    }

    if ( lpszTemp ) {
        LocalFree((HLOCAL) lpszTemp );
    }

    return pszBuf;
}

int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetThreadTimes(
        thread_handle, &creationTime, &exitTime, &kernelTime, &userTime)
    ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        tKernel.LowPart  = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart    = userTime.dwLowDateTime;
        tUser.HighPart   = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu = totTime / 1.e7;
    } else {
        return -1;
    }
    return 0;
}

int boinc_process_cpu_time(double& cpu) {
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (GetProcessTimes(
        GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)
    ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        tKernel.LowPart  = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart    = userTime.dwLowDateTime;
        tUser.HighPart   = userTime.dwHighDateTime;
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Runtimes in 100-nanosecond units
        cpu = totTime / 1.e7;
    } else {
        return -1;
    }
    return 0;
}

static void get_elapsed_time(double& cpu) {
    static bool first = true;
    static DWORD first_count = 0;

    if (first) {
        first_count = GetTickCount();
        first = false;
    }
    // TODO: Handle timer wraparound
    DWORD cur = GetTickCount();
    cpu = ((cur - first_count)/1000.);
}

int boinc_calling_thread_cpu_time(double& cpu) {
    if (boinc_thread_cpu_time(GetCurrentThread(), cpu)) {
        get_elapsed_time(cpu);
    }
    return 0;
}

#else

// Unix: pthreads doesn't seem to provide an API for getting
// per-thread CPU time.  So just get the process's CPU time
//
int boinc_calling_thread_cpu_time(double &cpu_t) {
    int retval;
    struct rusage ru;
    retval = getrusage(RUSAGE_SELF, &ru);
    if (retval) {
        return ERR_GETRUSAGE;
    }
    // Sum the user and system time
    //
    cpu_t = (double)ru.ru_utime.tv_sec + (((double)ru.ru_utime.tv_usec) / ((double)1000000.0));
    cpu_t += (double)ru.ru_stime.tv_sec + (((double)ru.ru_stime.tv_usec) / ((double)1000000.0));
    return 0;
}

#endif


// Update an estimate of "units per day" of something (credit or CPU time).
// The estimate is exponentially averaged with a given half-life
// (i.e. if no new work is done, the average will decline by 50% in this time).
// This function can be called either with new work,
// or with zero work to decay an existing average.
//
// NOTE: if you change this, also change update_average in
// html/inc/credit.inc
//
void update_average(
    double work_start_time,       // when new work was started
                                    // (or zero if no new work)
    double work,                    // amount of new work
    double half_life,
    double& avg,                    // average work per day (in and out)
    double& avg_time                // when average was last computed
) {
    double now = dtime();

    if (avg_time) {
        // If an average R already exists, imagine that the new work was done
        // entirely between avg_time and now.
        // That gives a rate R'.
        // Replace R with a weighted average of R and R',
        // weighted so that we get the right half-life if R' == 0.
        //
        // But this blows up if avg_time == now; you get 0*(1/0)
        // So consider the limit as diff->0,
        // using the first-order Taylor expansion of
        // exp(x)=1+x+O(x^2).
        // So to the lowest order in diff:
        // weight = 1 - diff ln(2) / half_life
        // so one has
        // avg += (1-weight)*(work/diff_days)
        // avg += [diff*ln(2)/half_life] * (work*SECONDS_PER_DAY/diff)
        // notice that diff cancels out, leaving
        // avg += [ln(2)/half_life] * work*SECONDS_PER_DAY

        double diff, diff_days, weight;

        diff = now - avg_time;
        if (diff<0) diff=0;

        diff_days = diff/SECONDS_PER_DAY;
        weight = exp(-diff*M_LN2/half_life);

        avg *= weight;

        if ((1.0-weight) > 1.e-6) {
            avg += (1-weight)*(work/diff_days);
        } else {
            avg += M_LN2*work*SECONDS_PER_DAY/half_life;
        }
    } else if (work) {
        // If first time, average is just work/duration
        //
        double dd = (now - work_start_time)/SECONDS_PER_DAY;
        avg = work/dd;
    }
    avg_time = now;
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

// Return a text-string description of a given error.  Must be kept
// consistent with error_numbers.h
//
const char* boincerror(int which_error) {
    switch (which_error) {
        case BOINC_SUCCESS: return "Success";
        case ERR_SELECT: return "system select";
        case ERR_MALLOC: return "system malloc";
        case ERR_READ: return "system read";
        case ERR_WRITE: return "system write";
        case ERR_FREAD: return "system fread";
        case ERR_FWRITE: return "system fwrite";
        case ERR_IO: return "system I/O";
        case ERR_CONNECT: return "system connect";
        case ERR_FOPEN: return "system fopen";
        case ERR_RENAME: return "system rename";
        case ERR_UNLINK: return "system unlink";
        case ERR_OPENDIR: return "system opendir";
        case ERR_XML_PARSE: return "unexpected XML tag or syntax";
        case ERR_GETHOSTBYNAME: return "can't resolve hostname";
        case ERR_GIVEUP_DOWNLOAD: return "timeout on download";
        case ERR_GIVEUP_UPLOAD: return "timeout on upload";
        case ERR_NULL: return "null pointer";
        case ERR_NEG: return "unexpected negative value";
        case ERR_BUFFER_OVERFLOW: return "buffer overflow";
        case ERR_MD5_FAILED: return "md5 checksum failed for file";
        case ERR_RSA_FAILED: return "RSA key check failed for file";
        case ERR_OPEN: return "system open";
        case ERR_DUP2: return "system dup";
        case ERR_NO_SIGNATURE: return "no signature";
        case ERR_THREAD: return "creating a thread";
        case ERR_SIGNAL_CATCH: return "caught signal";
        case ERR_UPLOAD_TRANSIENT: return "transient upload error";
        case ERR_UPLOAD_PERMANENT: return "fatal upload error";
        case ERR_IDLE_PERIOD: return "user preferences say can't start work";
        case ERR_ALREADY_ATTACHED: return "already attached to project";
        case ERR_FILE_TOO_BIG: return "file size too big";
        case ERR_GETRUSAGE: return "system getrusage";
        case ERR_BENCHMARK_FAILED: return "benchmark failed";
        case ERR_BAD_HEX_FORMAT: return "hex format key data bad";
        case ERR_USER_REJECTED: return "user rejected executable file";
        case ERR_DB_NOT_FOUND: return "no database rows found in lookup/enumerate";
        case ERR_DB_NOT_UNIQUE: return "database lookup not unique";
        case ERR_DB_CANT_CONNECT: return "can't connect to datbase";
        case ERR_GETS: return "system gets/fgets";
        case ERR_SCANF: return "system scanf/fscanf";
        case ERR_STRCHR: return "system strchr";
        case ERR_STRSTR: return "system strstr";
        case ERR_READDIR: return "system readdir";
        case ERR_SHMGET: return "system shmget";
        case ERR_SHMCTL: return "system shmctl";
        case ERR_SHMAT: return "system shmat";
        case ERR_FORK: return "system fork";
        case ERR_EXEC: return "system exec";
        case ERR_NOT_EXITED: return "process didn't exit";
        case ERR_NOT_IMPLEMENTED: return "system call not implemented";
        case ERR_GETHOSTNAME: return "system gethostname";
        case ERR_NETOPEN: return "system netopen";
        case ERR_SOCKET: return "system socket";
        case ERR_FCNTL: return "system fcntl";
        case ERR_AUTHENTICATOR: return "authentication error";
        case ERR_SCHED_SHMEM: return "scheduler shared memory contents bad";
        case ERR_ASYNCSELECT: return "system async select";
        case ERR_BAD_RESULT_STATE: return "bad result state";
        case ERR_DB_CANT_INIT: return "can't init database";
        case ERR_NOT_UNIQUE: return "state files have redundant entries";
        case ERR_NOT_FOUND: return "not found";
        case ERR_NO_EXIT_STATUS: return "no exit status in scheduler request";
        case ERR_FILE_MISSING: return "file missing";
        case ERR_SEMGET: return "system get semaphore";
        case ERR_SEMCTL: return "system control semaphore";
        case ERR_SEMOP: return "system op semaphore";
        case ERR_FTOK: return "system ftok";
        case ERR_SOCKS_UNKNOWN_FAILURE: return "socket unknown";
        case ERR_SOCKS_REQUEST_FAILED: return "socket request";
        case ERR_SOCKS_BAD_USER_PASS: return "socket bad user password";
        case ERR_SOCKS_UNKNOWN_SERVER_VERSION: return "socket unknown server version";
        case ERR_SOCKS_UNSUPPORTED: return "socket unsupported";
        case ERR_SOCKS_CANT_REACH_HOST: return "socket can't reach host";
        case ERR_SOCKS_CONN_REFUSED: return "socket connection refused";
        case ERR_TIMER_INIT: return "timer init";
        case ERR_RSC_LIMIT_EXCEEDED: return "resource limit exceeded";
        case ERR_INVALID_PARAM: return "invalid parameter";
        case ERR_SIGNAL_OP: return "signal op";
        case ERR_BIND: return "system bind";
        case ERR_LISTEN: return "system listen";
        case ERR_TIMEOUT: return "timeout";
        case ERR_PROJECT_DOWN: return "project down";
        case ERR_HTTP_ERROR: return "http error";
        case ERR_RESULT_START: return "result start";
        case ERR_RESULT_DOWNLOAD: return "result download";
        case ERR_RESULT_UPLOAD: return "result upload";
        case ERR_INVALID_URL: return "invalid URL";
        case ERR_MAJOR_VERSION: return "major version";
        case ERR_NO_OPTION: return "no option";
        case ERR_MKDIR: return "mkdir";
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
        case ERR_NONUNIQUE_EMAIL: return "email address is already in use";
        case ERR_SHMEM_NAME: return "can't get shared mem segment name";
        case ERR_NO_NETWORK_CONNECTION: return "no available network connection";
        case ERR_ATTACH_FAIL_INIT: return "Couldn't start master page download";
        case ERR_ATTACH_FAIL_DOWNLOAD: return "Couldn't download master page";
        case ERR_ATTACH_FAIL_PARSE: return "Couldn't parse master page";
        case ERR_ATTACH_FAIL_BAD_KEY: return "Invalid account key";
        case ERR_ATTACH_FAIL_FILE_WRITE: return "Couldn't write account file";
        case ERR_FFLUSH: return "Couldn't flush file";
        case ERR_FSYNC: return "Couldn't sync file";
        case ERR_TRUNCATE: return "Couldn't truncate file";
        case ERR_ABORTED_BY_PROJECT: return "Aborted by project";
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
 
const char *BOINC_RCSID_ab65c90e1e = "$Id$";
