// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef _WIN32
#include <time.h>
#include <afxwin.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "error_numbers.h"
#include "util.h"

// Converts a double precision time (where the value of 1 represents
// a day) into a string.  smallest_timescale determines the smallest
// unit of time division used
// smallest_timescale: 0=seconds, 1=minutes, 2=hours, 3=days, 4=years
//
int double_to_ydhms (double x, int smallest_timescale, char *buf) {
    double years, days, hours, minutes, seconds;
    char year_buf[64], day_buf[16], hour_buf[16], min_buf[16], sec_buf[16];
    
    if (x < 0 || buf == NULL) return -1;
    
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
    return (double)time.QuadPart/10000000;	// Convert to 1 s units
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
    sleep((int)seconds);
    usleep((int)fmod(seconds*1000000,1000000));
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

int lock_file(char* filename) {
    int retval;
        
    // some systems have both!
#ifdef HAVE_FLOCK
    int lock = open(filename, O_WRONLY|O_CREAT, 0644);
    retval = flock(lock, LOCK_EX|LOCK_NB);
#else
#ifdef HAVE_LOCKF
    int lock = open(filename, O_WRONLY|O_CREAT, 0644);
    retval = lockf(lock, F_TLOCK, 1);
    // must leave fd open
#endif
#endif
                        
#ifdef _WIN32
    HANDLE hfile = CreateFile(
        filename, GENERIC_WRITE,
        0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
    );
    if (hfile == INVALID_HANDLE_VALUE) retval = 1;
    else retval = 0;
#endif
    return retval;
}

double drand() {
    return (double)rand()/(double)RAND_MAX;
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

void strip_whitespace(char *str) {
    int read_pos=0, write_pos=0;
    while (str[read_pos]) {
        if (!isspace(str[read_pos])) {
            str[write_pos++] = str[read_pos];
        }
        read_pos++;
    }
    str[write_pos] = 0;
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

void safe_strncpy(char* dst, char* src, int len) {
    strncpy(dst, src, len);
    dst[len-1]=0;
}

// return current time of day as ASCII string, no CR
//
char* timestamp() {
    time_t now = time(0);
    char* p = ctime(&now);
    *(strchr(p, '\n')) = 0;
    return p;
}
