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

#include "boinc_db.h"
#include "sched_config.h"
#include "synch.h"
#include "sched_types.h"
#include "sched_shmem.h"

// various delay params.
// Any of these could be moved into SCHED_CONFIG, if projects need control.

#define DELAY_MISSING_KEY           3600
    // account key missing or invalid
#define DELAY_UNACCEPTABLE_OS       3600*24
    // Darwin 5.x or 6.x (E@h only)
#define DELAY_BAD_CLIENT_VERSION    3600*24
    // client version < config.min_core_client_version
#define DELAY_NO_WORK_SKIP          0
    // no work, config.nowork_skip is set
    // Rely on the client's exponential backoff in this case
#define DELAY_PLATFORM_UNSUPPORTED  3600*24
    // platform not in our DB
#define DELAY_DISK_SPACE            3600
    // too little disk space or prefs (locality scheduling)
#define DELAY_DELETE_FILE           3600*4
    // wait for client to delete a file (locality scheduling)
#define DELAY_ANONYMOUS             3600*4
    // anonymous platform client doesn't have version
#define DELAY_NO_WORK_TEMP          0
    // client asked for work but we didn't send any,
    // because of a reason that could be fixed by user
    // (e.g. prefs, or run BOINC more)
    // Rely on the client's exponential backoff in this case
#define DELAY_NO_WORK_PERM          3600*24
    // client asked for work but we didn't send any,
    // because of a reason not easily changed
    // (like wrong kind of computer)
#define DELAY_NO_WORK_CACHE         0
    // client asked for work but we didn't send any,
    // because user had too many results in cache.
    // Rely on client's exponential backoff
#define DELAY_MAX (2*86400)
    // maximum delay request

extern GUI_URLS gui_urls;
extern PROJECT_FILES project_files;
extern key_t sema_key;
extern int g_pid;
extern SCHED_SHMEM* ssp;
extern bool batch;
    // read sequences of requests from stdin (for testing)
extern bool mark_jobs_done;
    // mark jobs as successfully done immediately after send
    // (for debugging/testing)
extern bool all_apps_use_hr;

extern int open_database();
extern void debug_sched(const char *trigger);
