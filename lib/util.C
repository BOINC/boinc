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

#ifdef _WIN32
#include "boinc_win.h"
#define M_LN2      0.693147180559945309417
#endif

#ifndef _WIN32
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <iostream>
#ifdef HAVE_STD_LOCALE
#include <locale>
#endif
#include <fstream>
#include <cctype>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#endif

#include "error_numbers.h"
#include "util.h"
#include "boinc_api.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

using std::min;
using std::string;

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

    safe_strncpy(str, buf, len);
}

// return time of day as a double
// Not necessarily in terms of UNIX time (especially on Windows)
//
double dtime() {
#ifdef _WIN32
    LARGE_INTEGER time;
    FILETIME sysTime;
    GetSystemTimeAsFileTime(&sysTime);
    time.LowPart = sysTime.dwLowDateTime;
    time.HighPart = sysTime.dwHighDateTime;  // Time is in 100 ns units
    return (double)time.QuadPart/10000000;    // Convert to 1 s units
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + (tv.tv_usec/1.e6);
#endif
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
        if (rem <= 0) break;
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
    while (isascii(str[0]) && isspace(str[0])) {
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
    while (isascii(str[0]) && isspace(str[0])) {
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

void unescape_url(char *url) {
    register int x,y;

    for(x=0,y=0;url[y];++x,++y) {
        if((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
    }
    url[x] = '\0';
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

inline void replace_string(
    string& s, string const& src,
    string const& dest, string::size_type start=0
) {
    string::size_type p;
    while ( (p=s.find(src, start)) != string::npos ) {
        s.replace(p, src.length(), dest);
        start = p + dest.length();
    }
}

#ifdef HAVE_STD_LOCALE
// In order for toupper and tolower to work under certain conditions
//   it needs to know about local.
// See: http://linux-rep.fnal.gov/software/gcc/onlinedocs/libstdc++/22_locale/howto.html#7
struct Tolower
{
    Tolower (std::locale const& l) : loc(l) {;}
    char operator() (char c)  { return std::tolower(c,loc); }
private:
    std::locale const& loc;
};
#endif

// Canonicalize a master url.
//   - Convert the first part of a URL (before the "://") to lowercase
//   - Remove double slashes
//   - Add a trailing slash if necessary
//
void canonicalize_master_url(string& url) {
#ifdef HAVE_STD_LOCALE
    Tolower      down ( std::locale("C") );
#endif
    string::size_type p = url.find("://");
    // lowercase http://
    if (p != string::npos) {
#ifdef HAVE_STD_LOCALE
        transform(url.begin(), url.begin()+p, url.begin(), down);
#else
        transform(url.begin(), url.begin()+p, url.begin(), tolower);
#endif
        p += 3;
    } else {
        p = 0;
    }
    // remove double slashes
    replace_string(url, "//", "/", p);

    // ensure trailing slash
    if (url[url.length()-1] != '/') {
        url += '/';
    }
}

void canonicalize_master_url(char *xurl) {
    string url = xurl;
    canonicalize_master_url(url);
    strcpy(xurl, url.c_str());
}

bool invalid_url(char* p) {
    if (strstr(p, "http://") != p) return true;
    if (strlen(p) == strlen("http://")) return true;
    return false;
}

void safe_strncpy(char* dst, const char* src, int len) {
    strncpy(dst, src, len);
    dst[len-1]=0;
}

char* time_to_string(time_t x) {
    static char buf[100];
    struct tm* tm = localtime(&x);
    strftime(buf, sizeof(buf)-1, "%Y-%m-%d %H:%M:%S", tm);
    return buf;
}

// set by command line
bool debug_fake_exponential_backoff = false;
double debug_total_exponential_backoff = 0;
static int count_debug_fake_exponential_backoff = 0;
static const int max_debug_fake_exponential_backoff = 1000; // safety limit

// return a random integer in the range [MIN,min(e^n,MAX))
int calculate_exponential_backoff(
    const char* debug_descr, int n, double MIN, double MAX,
    double factor /* = 1.0 */
) {
    double rmax = min(MAX, factor*exp((double)n));

    if (debug_fake_exponential_backoff) {
        // For debugging/testing purposes, fake exponential back-off by
        // returning 0 seconds; report arguments so we can tell what we would
        // have done (this doesn't test the rand_range() functions but is
        // very useful for testing backoff/retry policies).
        //
        double expected_backoff = (MIN > rmax) ? MIN : (rmax-MIN)/2.0;

        debug_total_exponential_backoff += expected_backoff;
        ++count_debug_fake_exponential_backoff;
        fprintf(
            stderr,
            "## calculate_exponential_backoff(): #%5d descr=\"%s\", n=%d, MIN=%.1f, MAX=%.1f, factor=%.1f; rand_range [%.1f,%.1f); total expected backoff=%.1f\n",
            count_debug_fake_exponential_backoff,
            debug_descr, n, MIN, MAX, factor,
            MIN, rmax, debug_total_exponential_backoff
        );
        if (count_debug_fake_exponential_backoff >= max_debug_fake_exponential_backoff) {
            fprintf(
                stderr,
                "## calculate_exponential_backoff(): reached max_debug_fake_exponential_backoff\n"
            );
            exit(1);
        }
        return 0;
    }

    return (int) rand_range(MIN, rmax);
}

string timediff_format(long tdiff) {
    char buf[256];

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
        sprintf(buf, "%d hours, %d minutes, and %d seconds", hours, min, sex);
        return buf;
    }

    int days = tdiff % 7;
    tdiff /= 7;
    if (!tdiff) {
        sprintf(buf, "%d days, %d hours, %d minutes, and %d seconds", days, hours, min, sex);
        return buf;
    }

    sprintf(buf, "%d weeks, %d days, %d hours, %d minutes, and %d seconds", (int)tdiff, days, hours, min, sex);
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

int boinc_thread_cpu_time(HANDLE thread_handle, double& cpu, double& ws) {
    FILETIME creationTime,exitTime,kernelTime,userTime;
    static bool first = true;
    static DWORD first_count = 0;

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
		ws = 0;
    } else {
        if (first) {
            first_count = GetTickCount();
            first = false;
        }
        // TODO: Handle timer wraparound
        DWORD cur = GetTickCount();
	    cpu = ((cur - first_count)/1000.);
	    ws = 0;
    }
    return 0;
}

int boinc_calling_thread_cpu_time(double& cpu, double& ws) {
    return boinc_thread_cpu_time(GetCurrentThread(), cpu, ws);
}

#else

int boinc_calling_thread_cpu_time(double &cpu_t, double &ws_t) {
    int retval;
    struct rusage ru;
    retval = getrusage(RUSAGE_SELF, &ru);
    if (retval) {
        fprintf(stderr, "error: could not get CPU time\n");
    	return ERR_GETRUSAGE;
    }
    // Sum the user and system time spent in this process
    cpu_t = (double)ru.ru_utime.tv_sec + (((double)ru.ru_utime.tv_usec) / ((double)1000000.0));
    cpu_t += (double)ru.ru_stime.tv_sec + (((double)ru.ru_stime.tv_usec) / ((double)1000000.0));
    ws_t = ru.ru_idrss;     // TODO: fix this (mult by page size)
    return 0;
}

#endif


// Update an estimate of "units per day" of something (credit or CPU time).
// The estimate is exponentially averaged with a given half-life
// (i.e. if no new work is done, the average will decline by 50% in this time).
// This function can be called either with new work,
// or with zero work to decay an existing average.
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
        double diff = now - avg_time;
        if (diff<=0) diff=3600;     // just in case
        double diff_days = diff/SECONDS_PER_DAY;
        double weight = exp(-diff*M_LN2/half_life);
        avg *= weight;
        avg += (1-weight)*(work/diff_days);
    } else if (work) {
        double dd = (now - work_start_time)/SECONDS_PER_DAY;
        avg = work/dd;
    }
    avg_time = now;
}
