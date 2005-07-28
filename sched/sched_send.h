// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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

extern int send_work(
    SCHEDULER_REQUEST&, SCHEDULER_REPLY&, PLATFORM&, SCHED_SHMEM&
);

extern int add_result_to_reply(
    DB_RESULT& result, WORKUNIT& wu, SCHEDULER_REQUEST&, SCHEDULER_REPLY&,
    PLATFORM&, APP* app, APP_VERSION* avp
);

extern bool anonymous(PLATFORM&);

extern bool find_app_version(
    WORK_REQ& wreq, WORKUNIT& wu, PLATFORM& platform, SCHED_SHMEM& ss,
    APP*& app, APP_VERSION*& avp
);

extern bool app_core_compatible(WORK_REQ& wreq, APP_VERSION& av);

// bits in bitmap returned by wu_is_infeasible()
//
#define INFEASIBLE_MEM      1
#define INFEASIBLE_DISK     2
#define INFEASIBLE_CPU      4
#define INFEASIBLE_WORK_BUF 8

extern int wu_is_infeasible(WORKUNIT&, SCHEDULER_REQUEST&, SCHEDULER_REPLY&);

extern double max_allowable_disk(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);
extern bool resend_lost_work(SCHEDULER_REQUEST&, SCHEDULER_REPLY&);
