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
#include <math.h>
#include <ctype.h>

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
        sprintf( day_buf, "%.2f day ", days );
    } else if (days > 1 && smallest_timescale < 3) {
        sprintf( day_buf, "%d day ", (int)days );
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
//
double dtime() {
#ifdef _WIN32
    return (double)time(0);
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + (tv.tv_usec/1.e6);
#endif
}

// sleep for a specified number of seconds
//
void boinc_sleep(int seconds) {
#ifdef _WIN32
    ::Sleep(1000*seconds);
#else
    sleep(seconds);
#endif
}

// take a string containing some space separated words.
// return an array of pointers to the null-terminated words.
// Modifies the string arg.
// Returns argc
// TODO: use strtok here
int parse_command_line(char* p, char** argv) {
    char** pp = argv;
    bool space = true;
    int argc=0;

    while (*p) {
        if (isspace(*p)) {
            *p = 0;
            space = true;
        } else {
            if (space) {
                *pp++ = p;
		argc++;
                space = false;
            }
        }
        p++;
    }
    *pp++ = 0;

    return argc;
}

