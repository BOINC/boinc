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

#include "../sched/parse.h"
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
        "<poll_period>%f</poll_period>\n",
        ai.graphics.xsize,
        ai.graphics.ysize,
        ai.graphics.refresh_period,
        ai.checkpoint_period,
        ai.poll_period
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
        else fprintf(stderr, "read_core_file: unrecognized %s", buf);
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
}

void boinc_init(APP_IN& ai) {
    FILE* f;

    memset(&ai, 0, sizeof(ai));
    f = fopen(CORE_TO_APP_FILE, "r");
    if (f) {
        parse_core_file(f, ai);
        unlink(CORE_TO_APP_FILE);
    }
}

double boinc_time() {
    return double_time();
}

void boinc_poll(APP_IN& ai, APP_OUT& ao) {
    FILE* f;

    f = fopen("_app_temp", "w");
    write_app_file(f, ao);
    rename("_app_temp", APP_TO_CORE_FILE);
}
