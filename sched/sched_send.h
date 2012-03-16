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

#ifndef _SCHED_SEND_
#define _SCHED_SEND_

#include <string.h>

#include "boinc_db.h"
#include "sched_types.h"

extern void send_work();

extern int add_result_to_reply(
    SCHED_DB_RESULT& result, WORKUNIT& wu, BEST_APP_VERSION* bavp,
    bool locality_scheduling
);

inline bool is_anonymous(PLATFORM* platform) {
    return (!strcmp(platform->name, "anonymous"));
}

extern bool app_core_compatible(WORK_REQ& wreq, APP_VERSION& av);

// values returned by wu_is_infeasible()
//
#define INFEASIBLE_MEM          1
#define INFEASIBLE_DISK         2
#define INFEASIBLE_CPU          3
#define INFEASIBLE_WORK_BUF     4
#define INFEASIBLE_APP_SETTING  5
#define INFEASIBLE_WORKLOAD     6
#define INFEASIBLE_DUP          7
#define INFEASIBLE_HR           8
#define INFEASIBLE_BANDWIDTH    9
#define INFEASIBLE_CUSTOM       10
#define INFEASIBLE_USER_FILTER  11
#define INFEASIBLE_HAV          12

extern int wu_is_infeasible_fast(
    WORKUNIT&,
    int res_server_state, int res_priority, double res_report_deadline,
    APP&, BEST_APP_VERSION&
);
 
extern double max_allowable_disk();

extern bool wu_already_in_reply(WORKUNIT& wu);

extern double estimate_duration(WORKUNIT& wu, BEST_APP_VERSION&);

extern int update_wu_on_send(WORKUNIT wu, time_t x, APP&, BEST_APP_VERSION&);

extern void lock_sema();
extern void unlock_sema();
extern const char* infeasible_string(int);
extern bool app_not_selected(WORKUNIT&);
extern bool work_needed(bool);
extern void send_work_setup();
extern int effective_ncpus();
extern int preferred_app_message_index;
extern void update_n_jobs_today();

#endif
