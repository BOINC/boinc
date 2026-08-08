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

#ifndef BOINC_SCHED_SEND_H
#define BOINC_SCHED_SEND_H

#include <string.h>

#include "boinc_db.h"
#include "sched_shmem.h"
#include "sched_types.h"
#include "buda.h"

const int MAX_GPUS = 64;
    // don't believe clients who claim they have more GPUs than this
const int MAX_CPUS = 4096;
    // don't believe clients who claim they have more CPUs than this

extern void send_work();

extern int add_result_to_reply(
    SCHED_DB_RESULT& result, WORKUNIT& wu, BEST_APP_VERSION* bavp,
    HOST_USAGE&, BUDA_VARIANT*,
    bool locality_scheduling
);

inline bool is_anonymous(PLATFORM* platform) {
    return (!strcmp(platform->name, "anonymous"));
}

extern bool app_core_compatible(WORK_REQ& wreq, APP_VERSION& av);

extern double max_allowable_disk();

extern bool wu_already_in_reply(WORKUNIT& wu);

extern double estimate_duration(WORKUNIT& wu, BEST_APP_VERSION&);

extern int update_wu_on_send(WORKUNIT wu, time_t x, APP&, BEST_APP_VERSION&);

extern void lock_sema();
extern void unlock_sema();
extern const char* find_user_friendly_name(int appid);
extern bool work_needed(bool);
extern void send_work_setup();
extern int effective_ncpus();
extern int selected_app_message_index;
extern void update_n_jobs_today();
extern int nfiles_on_host(WORKUNIT&);

#endif
