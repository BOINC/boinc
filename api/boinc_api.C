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

// API_IGNORE_CLIENT will make the app ignore the core client
// this is useful for debugging just the application
//#define API_IGNORE_CLIENT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#include <sys/stat.h>
#include <windows.h>
#include <winuser.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <signal.h>

#include <fcntl.h>
#include <sys/types.h>

#include "parse.h"
#include "error_numbers.h"
#include "graphics_api.h"

#include "boinc_api.h"

static APP_INIT_DATA aid;
static GRAPHICS_INFO gi;
static double timer_period = 0.1;
static double time_until_checkpoint;
static double time_until_fraction_done_update;
static double fraction_done;
static bool ready_to_checkpoint = false;
static bool this_process_active;

#ifdef _WIN32
DWORD WINAPI graphics_main( LPVOID duff );
#endif

// read the INIT_DATA and FD_INIT files
//
int boinc_init() {
    FILE* f;
    int retval;

#ifdef _WIN32
    freopen(STDERR_FILE, "a", stderr);
    fprintf(stderr, "in boinc_init()\n");
#endif

#ifndef API_IGNORE_CLIENT
    f = fopen(INIT_DATA_FILE, "r");
    if (!f) {
        fprintf(stderr, "boinc_init(): can't open init data file\n");
        return ERR_FOPEN;
    }
    retval = parse_init_data_file(f, aid);
    if (retval) {
        fprintf(stderr, "boinc_init(): can't parse init data file\n");
        return retval;
    }
    fclose(f);

    f = fopen(GRAPHICS_DATA_FILE, "r");
    if (!f) {
        fprintf(stderr, "boinc_init(): can't open graphics data file\n");
        return ERR_FOPEN;
    }
    retval = parse_graphics_file(f, &gi);
    if (retval) {
        fprintf(stderr, "boinc_init(): can't parse graphics data file\n");
        return retval;
    }
    fclose(f);

    f = fopen(FD_INIT_FILE, "r");
    if (f) {
        parse_fd_init_file(f);
        fclose(f);
    }
#else
	strcpy(aid.app_preferences, "");
	strcpy(aid.user_name, "John Smith");
	strcpy(aid.team_name, "The A-Team");
    aid.wu_cpu_time = 1000;
    aid.total_cobblestones = 1000;
    aid.recent_avg_cobblestones = 500;
    aid.checkpoint_period = DEFAULT_CHECKPOINT_PERIOD;
    aid.fraction_done_update_period = DEFAULT_FRACTION_DONE_UPDATE_PERIOD;

	gi.graphics_mode = MODE_WINDOW;
	gi.refresh_period = 0.1; // 1/10th of a second
	gi.xsize = 640;
	gi.ysize = 480;
#endif
    time_until_checkpoint = aid.checkpoint_period;
    time_until_fraction_done_update = aid.fraction_done_update_period;
    this_process_active = true;
    set_timer(timer_period);

    return 0;
}

int boinc_finish(int status) {
    write_checkpoint_cpu_file(boinc_cpu_time());
    exit(status);
    return 0;
}

int boinc_get_init_data(APP_INIT_DATA& app_init_data) {
    app_init_data = aid;
    return 0;
}

// resolve XML soft links
//
int boinc_resolve_filename(char *virtual_name, char *physical_name) {
    FILE *fp;
    char buf[512];

    strcpy(physical_name, virtual_name);

    // Open the file and load the first line
    fp = fopen(virtual_name, "r");
    if (!fp) return 0;

    fgets(buf, 512, fp);
    fclose(fp);

    // If it's the <soft_link> XML tag, return its value,
    // otherwise, return the original file name
    //
    parse_str( buf, "<soft_link>", physical_name);
    return 0;
}


bool boinc_time_to_checkpoint() {
    return ready_to_checkpoint;
}

int boinc_checkpoint_completed() {
    write_checkpoint_cpu_file(boinc_cpu_time());
    ready_to_checkpoint = false;
    time_until_checkpoint = aid.checkpoint_period;
    return 0;
}

int boinc_fraction_done(double x) {
    fraction_done = x;
    return 0;
}

int boinc_child_start() {
    this_process_active = false;
    return 0;
}

int boinc_child_done(double cpu) {
    this_process_active = true;
    return 0;
}

double boinc_cpu_time() {
#ifdef HAVE_SYS_RESOURCE_H
    int retval, pid = getpid();
    struct rusage ru;
    retval = getrusage(RUSAGE_SELF, &ru);
    if(retval) fprintf(stderr, "error: could not get cpu time for %d\n", pid);
    return (double)ru.ru_utime.tv_sec + (
        ((double)ru.ru_utime.tv_usec) / ((double)1000000.0)
    );
#else
#ifdef _WIN32
#ifdef WINNT_CLOCK
    HANDLE hProcess;
    FILETIME creationTime,exitTime,kernelTime,userTime;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, GetCurrentProcessId());
    if (GetProcessTimes(
        hProcess, &creationTime, &exitTime, &kernelTime, &userTime)
    ) {
        ULARGE_INTEGER tKernel, tUser;
        LONGLONG totTime;

        CloseHandle(hProcess);

        tKernel.LowPart  = kernelTime.dwLowDateTime;
        tKernel.HighPart = kernelTime.dwHighDateTime;
        tUser.LowPart    = userTime.dwLowDateTime;
        tUser.HighPart   = userTime.dwHighDateTime;

        // Runtimes in 100-nanosecond units
        totTime = tKernel.QuadPart + tUser.QuadPart;

        // Convert to seconds and return
        return(totTime / 10000000.0);
    }
    CloseHandle(hProcess);
    // ... fall through
#endif  // WINNT_CLOCK
    static bool first=true;
    static DWORD last_count = 0;

    if (first) {
        last_count = GetTickCount();
        first = true;
    }
    DWORD cur = GetTickCount();
    double x = (cur - last_count)/1000.;
    last_count = cur;
    return x;
#endif  // _WIN32
#endif

#ifdef macintosh
    return (double) GetCPUTime() / CLOCKS_PER_SEC;
#endif

    fprintf(stderr, "boinc_cpu_time(): not implemented\n");
}

#ifdef _WIN32
void CALLBACK on_timer( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime ) {
#else
void on_timer(int a) {
#endif

    if (!ready_to_checkpoint) {
        time_until_checkpoint -= timer_period;
        if (time_until_checkpoint <= 0) {
            ready_to_checkpoint = true;
        }
    }

    if (this_process_active) {
        time_until_fraction_done_update -= timer_period;
        if (time_until_fraction_done_update < 0) {
            FILE* f = fopen(FRACTION_DONE_FILE, "w");
            write_fraction_done_file(f, boinc_cpu_time(), fraction_done);
            fclose(f);
            time_until_fraction_done_update = aid.fraction_done_update_period;
        }
    }
}


int set_timer(double period) {
    int retval=0;
#ifdef _WIN32
    retval = SetTimer( NULL, 0, (int)(period*1000), on_timer );
#endif

#if HAVE_SIGNAL_H
#if HAVE_SYS_TIME_H
    struct sigaction sa;
    sa.sa_handler = on_timer;
    sa.sa_flags = 0;
    sigaction(SIGVTALRM, &sa, NULL);
    itimerval value;
    value.it_value.tv_sec = (int)period;
    value.it_value.tv_usec = ((int)(period*1000000))%1000000;
    value.it_interval = value.it_value;
    retval = setitimer(ITIMER_VIRTUAL, &value, NULL);
#endif
#endif
    return retval;
}

int write_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    if (strlen(ai.app_preferences)) {
        fprintf(f, "<app_preferences>\n%s</app_preferences>\n", ai.app_preferences);
    }
    if (strlen(ai.team_name)) {
        fprintf(f, "<team_name>\n%s</team_name>\n", ai.team_name);
    }
    if (strlen(ai.user_name)) {
        fprintf(f, "<user_name>\n%s</user_name>\n", ai.user_name);
    }
    fprintf(f,
        "<wu_cpu_time>%f</wu_cpu_time>\n"
        "<total_cobblestones>%f</total_cobblestones>\n"
        "<recent_avg_cobblestones>%f</recent_avg_cobblestones>\n"
        "<checkpoint_period>%f</checkpoint_period>\n"
        "<fraction_done_update_period>%f</fraction_done_update_period>\n",
        ai.wu_cpu_time,
        ai.total_cobblestones,
        ai.recent_avg_cobblestones,
        ai.checkpoint_period,
        ai.fraction_done_update_period
    );
    return 0;
}

int parse_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    char buf[256];
    memset(&ai, 0, sizeof(ai));
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<app_preferences>")) {
            strcpy(ai.app_preferences, "");
            while (fgets(buf, 256, f)) {
                if (match_tag(buf, "</app_specific_prefs>")) break;
                strcat(ai.app_preferences, buf);
            }
            continue;
        }
        else if (parse_str(buf, "<user_name>", ai.user_name)) continue;
        else if (parse_str(buf, "<team_name>", ai.team_name)) continue;
        else if (parse_double(buf, "<total_cobblestones>", ai.total_cobblestones)) continue;
        else if (parse_double(buf, "<recent_avg_cobblestones>", ai.recent_avg_cobblestones)) continue;
        else if (parse_double(buf, "<wu_cpu_time>", ai.wu_cpu_time)) continue;
        else if (parse_double(buf, "<checkpoint_period>", ai.checkpoint_period)) continue;
        else if (parse_double(buf, "<fraction_done_update_period>", ai.fraction_done_update_period)) continue;
        else fprintf(stderr, "parse_init_data_file: unrecognized %s", buf);
    }
    return 0;
}

int write_checkpoint_cpu_file(double cpu) {
    FILE *f = fopen(CHECKPOINT_CPU_FILE, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f, "<checkpoint_cpu_time>%f</checkpoint_cpu_time>\n", cpu);
    fclose(f);
    return 0;
}

int parse_checkpoint_cpu_file(FILE* f, double& checkpoint_cpu) {
    char buf[256];
    while (fgets(buf, 256, f)) {
        if (parse_double(buf, "<checkpoint_cpu_time>", checkpoint_cpu)) continue;
    }
    return 0;
}

int write_fraction_done_file(FILE* f, double pct, double cpu) {
    fprintf(f,
        "<fraction_done>%f</fraction_done>\n"
        "<cpu_time>%f</cpu_time>\n",
        pct,
        cpu
    );
    return 0;
}

int parse_fraction_done_file(FILE* f, double& pct, double& cpu) {
    char buf[256];
    while (fgets(buf, 256, f)) {
        if (parse_double(buf, "<fraction_done>", pct)) continue;
        else if (parse_double(buf, "<cpu_time>", cpu)) continue;
        else fprintf(stderr, "parse_fraction_done_file: unrecognized %s", buf);
    }
    return 0;
}

// TODO: this should handle arbitrarily many fd/filename pairs.
// Also, give the tags better names
int write_fd_init_file(FILE* f, char *file_name, int fdesc, int input_file ) {
    if (input_file) {
        fprintf(f, "<fdesc_dup_infile>%s</fdesc_dup_infile>\n", file_name);
        fprintf(f, "<fdesc_dup_innum>%d</fdesc_dup_innum>\n", fdesc);
    } else {
        fprintf(f, "<fdesc_dup_outfile>%s</fdesc_dup_outfile>\n", file_name);
        fprintf(f, "<fdesc_dup_outnum>%d</fdesc_dup_outnum>\n", fdesc);
    }
    return 0;
}

// TODO: this should handle arbitrarily many fd/filename pairs.
// Also, this shouldn't be doing the actual duping!
//
int parse_fd_init_file(FILE* f) {
    char buf[256],filename[256];
    int filedesc;
    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<fdesc_dup_infile>", filename)) {
            if (fgets(buf, 256, f)) {
                if (parse_int(buf, "<fdesc_dup_innum>", filedesc)) {
                    freopen(filename, "r", stdin);
                    fprintf(stderr, "opened input file %s\n", filename);
                }
            }
        } else if (parse_str(buf, "<fdesc_dup_outfile>", filename)) {
            if (fgets(buf, 256, f)) {
                if (parse_int(buf, "<fdesc_dup_outnum>", filedesc)) {
                    freopen(filename, "w", stdout);
                    fprintf(stderr, "opened output file %s\n", filename);
                }
            }
        } else fprintf(stderr, "parse_fd_init_file: unrecognized %s", buf);
    }
    return 0;
}
