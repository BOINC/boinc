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

// code related to "time stats logs".
// These are summaries of client availability send in scheduler requests.
// The scheduler writes them to a directory hierarchy for later analysis.

#include <sys/types.h>
#include <sys/stat.h>

#include "filesys.h"
#include "parse.h"

#include "sched_msgs.h"
#include "sched_config.h"
#include "sched_types.h"

#include "time_stats_log.h"

static char* stats_buf = 0;

// Got a <time_stats_log> flag in scheduler request.
// Copy the contents to a malloced buffer;
// don't write them to disk yet, since we haven't authenticated the host
//

int handle_time_stats_log(FILE* fin) {
    return dup_element_contents(fin, "</time_stats_log>", &stats_buf);
}

// The host has been authenticated, so write the stats.
// Use a directory hierarchy since there may be many hosts
//
void write_time_stats_log() {
    char filename[256];
    const char *dirname;

    int hostid = g_reply->host.id;
    int dirnum = hostid % 1000;
    dirname = config.project_path("time_stats_log/%d", dirnum);
    if (!is_dir(dirname)) {
        int retval = boinc_mkdir(dirname);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "Can't make time stats log dir %s: %s\n",
                dirname, boincerror(retval)
            );
            std::perror("mkdir");
            return;
        }
    }
    sprintf(filename, "%s/%d", dirname, hostid);
    FILE* f = boinc::fopen(filename, "w");
    if (!f) {
        log_messages.printf(MSG_CRITICAL,
            "Can't create time stats file %s\n", filename
        );
        return;
    }
    boinc::fputs(stats_buf, f);
    boinc::fclose(f);
    free(stats_buf);
    stats_buf = 0;
}

bool have_time_stats_log() {
    int hostid = g_reply->host.id;
    int dirnum = hostid % 1000;
    return is_file(
        config.project_path("time_stats_log/%d/%d", dirnum, hostid)
    );
}
