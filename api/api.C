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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#include <sys/stat.h>
#include "windows_cpp.h"
#else
#include <unistd.h>
#endif
#ifdef UNIX
#include <sys/time.h>
#endif
#include <signal.h>

#include <fcntl.h>
#include <sys/types.h>
#include "parse.h"
#include "api.h"

int MFILE::open(char* path, char* mode) {
    buf = 0;
    len = 0;
    f = fopen(path, mode);
    if (!f) return -1;
    return 0;
}

int MFILE::printf(char* format, ...) {
    va_list ap;
    char buf2[4096];
    int n, k;

    va_start(ap, format);
    k = vsprintf(buf2, format, ap);
    va_end(ap);
    n = strlen(buf2);
    buf = (char*)realloc(buf, len+n);
    strncpy(buf+len, buf2, n);
    len += n;
    return k;
}

size_t MFILE::write(const void *ptr,size_t size,size_t nitems) {
    buf = (char *)realloc( buf, len+(size*nitems) );
    memcpy( buf+len, ptr, size*nitems );
    len += size*nitems;
    return nitems;
}

int MFILE::_putchar(char c) {
    buf = (char*)realloc(buf, len+1);
    buf[len] = c;
    len++;
    return c;
}

int MFILE::puts(char* p) {
    int n = strlen(p);
    buf = (char*)realloc(buf, len+n);
    strncpy(buf+len, p, n);
    len += n;
    return 0;
}

int MFILE::close() {
    fwrite(buf, 1, len, f);
    free(buf);
    buf = 0;
    return fclose(f);
}

int MFILE::flush() {
    fwrite(buf, 1, len, f);
    len = 0;
    return fflush(f);
}

void write_core_file(FILE* f, APP_IN& ai) {
    fprintf(f,
        "<graphics_xsize>%d</graphics_xsize>\n"
        "<graphics_ysize>%d</graphics_ysize>\n"
        "<graphics_refresh_period>%f</graphics_refresh_period>\n"
        "<checkpoint_period>%f</checkpoint_period>\n"
        "<poll_period>%f</poll_period>\n"
        "<checkpoint_period>%f</checkpoint_period>\n"
        "<cpu_time>%f</cpu_time>\n",
        ai.graphics.xsize,
        ai.graphics.ysize,
        ai.graphics.refresh_period,
        ai.checkpoint_period,
	ai.poll_period,
        ai.checkpoint_period,
        ai.cpu_time
    );
}

void parse_core_file(FILE* f, APP_IN& ai) {
    char buf[256];

    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<app_specific_prefs>")) {
            strcpy(ai.app_preferences, "");
            while (fgets(buf, 256, f)) {
                if (match_tag(buf, "</app_specific_prefs>")) break;
                strcat(ai.app_preferences, buf);
            }
            continue;
        }
        else if (parse_int(buf, "<graphics_xsize>", ai.graphics.xsize)) continue;
        else if (parse_int(buf, "<graphics_ysize>", ai.graphics.ysize)) continue;
        else if (parse_double(buf, "<graphics_refresh_period>", ai.graphics.refresh_period)) continue;
        else if (parse_double(buf, "<checkpoint_period>", ai.checkpoint_period)) continue;
        else if (parse_double(buf, "<poll_period>", ai.poll_period)) continue;
        else if (parse_double(buf, "<checkpoint_period>", ai.checkpoint_period)) continue;
	else if (parse_double(buf, "<cpu_time>", ai.cpu_time)) continue;
        else fprintf(stderr, "parse_core_file: unrecognized %s", buf);
    }
}

void write_app_file(FILE* f, APP_OUT& ao) {
    fprintf(f,
        "<percent_done>%f</percent_done>\n"
        "<cpu_time_at_checkpoint>%f</cpu_time_at_checkpoint>\n",
        ao.percent_done,
        ao.cpu_time_at_checkpoint
    );
    if (ao.checkpointed) {
        fprintf(f, "<checkpointed/>\n");
    }
}

void parse_app_file(FILE* f, APP_OUT& ao) {
    char buf[256];
    while (fgets(buf, 256, f)) {
        if (parse_double(buf, "<percent_done>", ao.percent_done)) continue;
        else if (parse_double(buf, "<cpu_time_at_checkpoint>", 
            ao.cpu_time_at_checkpoint)) continue;
        else if (match_tag(buf, "<checkpointed/>")) ao.checkpointed = true;
        else fprintf(stderr, "parse_app_file: unrecognized %s", buf);
    }
}

void write_init_file(FILE* f, char *file_name, int fdesc, int input_file ) {
    if( input_file ) {
        fprintf( f, "<fdesc_dup_infile>%s</fdesc_dup_infile>\n", file_name );
        fprintf( f, "<fdesc_dup_innum>%d</fdesc_dup_innum>\n", fdesc );
    } else {
        fprintf( f, "<fdesc_dup_outfile>%s</fdesc_dup_outfile>\n", file_name );
        fprintf( f, "<fdesc_dup_outnum>%d</fdesc_dup_outnum>\n", fdesc );
    }
}

void parse_init_file(FILE* f) {
    char buf[256],filename[256];
    int filedesc,fd,retval;

    while (fgets(buf, 256, f)) {
        if (parse_str(buf, "<fdesc_dup_infile>", filename)) {
            if (fgets(buf, 256, f)) {
                if (parse_int(buf, "<fdesc_dup_innum>", filedesc)) {
                    fd = open(filename, O_RDONLY);
                    if (fd != filedesc) {
                        retval = dup2(fd, filedesc);
                        if (retval < 0) {
                            fprintf(stderr, "dup2 %d %d returned %d\n", fd, filedesc, retval);
                            exit(retval);
                        }
                        close(fd);
                    }
                }
            }
            continue;
        }
        else if (parse_str(buf, "<fdesc_dup_outfile>", filename)) {
            if (fgets(buf, 256, f)) {
                if (parse_int(buf, "<fdesc_dup_outnum>", filedesc)) {
                    fd = open(filename, O_WRONLY|O_CREAT, 0660);
                    if (fd != filedesc) {
                        retval = dup2(fd, filedesc);
                        if (retval < 0) {
                            fprintf(stderr, "dup2 %d %d returned %d\n", fd, filedesc, retval);
                            exit(retval);
                        }
                        close(fd);
                    }
                }
            }
            continue;
        }
        else fprintf(stderr, "parse_init_file: unrecognized %s", buf);
    }
}


/* boinc_init reads the BOINC_INIT_FILE and CORE_TO_APP_FILE files and performs the necessary initialization */

void boinc_init(APP_IN& ai) {
    FILE* f;

    memset(&ai, 0, sizeof(ai));
    f = fopen(CORE_TO_APP_FILE, "r");
    if (f) {
        parse_core_file(f, ai);
        unlink(CORE_TO_APP_FILE);
    }

    f = fopen( BOINC_INIT_FILE, "r" );
    if (f) {
        parse_init_file(f);
        unlink(BOINC_INIT_FILE);
    }
    set_timer(ai.checkpoint_period);
}

void boinc_poll(APP_IN& ai, APP_OUT& ao) {
    FILE* f;

    f = fopen("_app_temp", "w");
    write_app_file(f, ao);
    rename("_app_temp", APP_TO_CORE_FILE);
}

/* boinc_resolve_link is used to resolve the XML soft links in a file. */

int boinc_resolve_link(char *file_name, char *resolved_name) 
{
    FILE *fp;
    char buf[512];

    // Open the file and load the first line
    fp = fopen( file_name, "r" );
    if (!fp) {
        strcpy( resolved_name, file_name );
        return -1;
    }
    rewind( fp );
    fgets(buf, 512, fp);
    fclose( fp );

    // If it's the <soft_link> XML tag, return it's value,
    // otherwise, return the original file name
    if( !parse_str( buf, "<soft_link>", resolved_name ) ) {
        strcpy( resolved_name, file_name );
    }

    return 0;
}

bool _checkpoint = false;

double get_cpu_time() {
    int retval, pid = getpid();
#ifdef unix
    struct rusage ru;
    retval = getrusage(RUSAGE_SELF, &ru);
    if(retval) fprintf(stderr, "error: could not get cpu time for %d\n", pid);
    return (double)ru.ru_utime.tv_sec + (
	((double)ru.ru_utime.tv_usec) / ((double)1000000.0)
    ); //this should be a decimal, but isn't
#else
    return 0;
#endif
}

int checkpoint_completed() {
    int retval;
    APP_OUT ao;
    FILE *f = fopen(APP_TO_CORE_FILE, "w");
    ao.cpu_time_at_checkpoint = get_cpu_time();
    write_app_file(f, ao);
    retval = fflush(f);
    if(retval) {
	fprintf(stderr,"error: could not flush %s\n", APP_TO_CORE_FILE);
	return retval;
    }
    retval = fclose(f);
    if(retval) {
	fprintf(stderr, "error: could not close %s\n", APP_TO_CORE_FILE);
	return retval;
    }
    _checkpoint = false;
    return 0;
}

void on_timer(int a) {
    _checkpoint = true;
}

int set_timer(double period) {
    int retval=0;
    struct sigaction sa;
    sa.sa_handler = on_timer;
    sa.sa_flags = 0;
    sigaction(SIGVTALRM, &sa, NULL);
#ifdef unix
    itimerval value;
    value.it_value.tv_sec = (int)period;
    value.it_value.tv_usec = ((int)(period*1000000))%1000000;
    value.it_interval = value.it_value;
    retval = setitimer(ITIMER_VIRTUAL, &value, NULL);
#endif
    return retval;
}

