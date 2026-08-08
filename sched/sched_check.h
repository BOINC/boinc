// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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
// values returned by wu_is_infeasible()

#ifndef BOINC_SCHED_CHECK_H
#define BOINC_SCHED_CHECK_H

#include "sched_shmem.h"
#include "sched_types.h"

// return values from wu_is_infeasible_fast()
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

extern const char* infeasible_string(int);

extern int wu_is_infeasible_fast(
    WORKUNIT&,
    int res_server_state, int res_priority, double res_report_deadline,
    APP&, BEST_APP_VERSION&
);

// return values from slow_check()
//
#define CHECK_OK        0
#define CHECK_NO_HOST   1
#define CHECK_NO_ANY    2

extern int slow_check(WU_RESULT&, APP*, BEST_APP_VERSION*);

extern bool result_still_sendable(DB_RESULT& result, WORKUNIT& wu);
extern bool app_not_selected(int appid);

#endif
