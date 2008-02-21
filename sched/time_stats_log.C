// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#include <sys/types.h>
#include <sys/stat.h>

#include "filesys.h"
#include "parse.h"

#include "sched_msgs.h"

#include "time_stats_log.h"

static char* stats_buf = 0;

// Got a <time_stats_log> flag in scheduler request.
// Copy the contents to a malloced buffer;
// don't write them to disk yet, since we haven't authenticated the host
//

void handle_time_stats_log(FILE* fin) {
    dup_element_contents(fin, "</time_stats_log>", &stats_buf);
}

// The host has been authenticated, so write the stats.
// Use a directory hierarchy since there may be many hosts
//
void write_time_stats_log(SCHEDULER_REPLY& reply) {
    char dirname[256], filename[256];

    int hostid = reply.host.id;
    int dirnum = hostid % 1000;
    sprintf(dirname, "../time_stats_log/%d", dirnum);
    if (!is_dir(dirname)) {
        int retval = boinc_mkdir(dirname);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "Can't make time stats log dir %s: %d\n", dirname, retval
            );
            perror("mkdir");
            return;
        }
    }
    sprintf(filename, "../time_stats_log/%d/%d", dirnum, hostid);
    FILE* f = fopen(filename, "w");
    if (!f) {
        log_messages.printf(MSG_CRITICAL,
            "Can't create time stats file %s\n", filename
        );
        return;
    }
    fputs(stats_buf, f);
    fclose(f);
}

bool have_time_stats_log(SCHEDULER_REPLY& reply) {
    char filename[256];

    int hostid = reply.host.id;
    int dirnum = hostid % 1000;
    sprintf(filename, "../time_stats_log/%d/%d", dirnum, hostid);
    return is_file(filename);
}
