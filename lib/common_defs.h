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

#ifndef _COMMON_DEFS_
#define _COMMON_DEFS_

#include "miofile.h"

// #defines or enums that are shared by more than one BOINC component
// (e.g. client, server, Manager, etc.)

#define GUI_RPC_PORT                                31416

#define RUN_MODE_ALWAYS 1
#define RUN_MODE_AUTO   2
#define RUN_MODE_NEVER  3
#define RUN_MODE_RESTORE    4
    // restore permanent mode - used only in set_X_mode() GUI RPC

// values of ACTIVE_TASK::scheduler_state and ACTIVE_TASK::next_scheduler_state
#define CPU_SCHED_UNINITIALIZED   0
#define CPU_SCHED_PREEMPTED       1
#define CPU_SCHED_SCHEDULED       2

// official HTTP status codes

#define HTTP_STATUS_CONTINUE                100
#define HTTP_STATUS_OK                      200
#define HTTP_STATUS_PARTIAL_CONTENT         206
#define HTTP_STATUS_MOVED_PERM              301
#define HTTP_STATUS_MOVED_TEMP              302
#define HTTP_STATUS_NOT_FOUND               404
#define HTTP_STATUS_PROXY_AUTH_REQ          407
#define HTTP_STATUS_RANGE_REQUEST_ERROR     416
#define HTTP_STATUS_INTERNAL_SERVER_ERROR   500
#define HTTP_STATUS_SERVICE_UNAVAILABLE     503

// the core client can be requested to provide screensaver graphics (SSG).
// The following are states of this function:

#define SS_STATUS_ENABLED                           1
    // requested to provide SSG
#define SS_STATUS_BLANKED                           3
    // not providing SSG, SS should blank screen
#define SS_STATUS_BOINCSUSPENDED                    4
    // not providing SS because suspended
#define SS_STATUS_NOAPPSEXECUTING                   6
    // no apps executing
#define SS_STATUS_NOGRAPHICSAPPSEXECUTING           7
    // apps executing, but none graphical
#define SS_STATUS_QUIT                              8
    // not requested to provide SSG
#define SS_STATUS_NOPROJECTSDETECTED                9
    // SSG unsupported; client running as daemon
#define SS_STATUS_DAEMONALLOWSNOGRAPHICS            10

// graphics messages
//
#define MODE_UNSUPPORTED        0
#define MODE_HIDE_GRAPHICS      1
#define MODE_WINDOW             2
#define MODE_FULLSCREEN         3
#define MODE_BLANKSCREEN        4
#define MODE_REREAD_PREFS       5
#define MODE_QUIT               6
#define NGRAPHICS_MSGS  7

// message priorities
#define MSG_INFO    1
    // write to stdout
    // GUI: write to msg window
#define MSG_ERROR   2
    // write to stdout
    // GUI: write to msg window in bold or red

// bitmap defs for task_suspend_reason, network_suspend_reason
//
enum SUSPEND_REASON {
    SUSPEND_REASON_BATTERIES = 1,
    SUSPEND_REASON_USER_ACTIVE = 2,
    SUSPEND_REASON_USER_REQ = 4,
    SUSPEND_REASON_TIME_OF_DAY = 8,
    SUSPEND_REASON_BENCHMARKS = 16,
    SUSPEND_REASON_DISK_SIZE = 32,
    SUSPEND_REASON_CPU_USAGE_LIMIT = 64
};

// States of a result on a client.
// THESE MUST BE IN NUMERICAL ORDER
// (because of the > comparison in RESULT::computing_done())
//
#define RESULT_NEW               0
    // New result
#define RESULT_FILES_DOWNLOADING 1
    // Input files for result (WU, app version) are being downloaded
#define RESULT_FILES_DOWNLOADED  2
    // Files are downloaded, result can be (or is being) computed
#define RESULT_COMPUTE_ERROR     3
    // computation failed; no file upload
#define RESULT_FILES_UPLOADING   4
    // Output files for result are being uploaded
#define RESULT_FILES_UPLOADED    5
    // Files are uploaded, notify scheduling server at some point
#define RESULT_ABORTED          6
    // result was aborted

// values of "network status"
//
#define NETWORK_STATUS_ONLINE			0
#define NETWORK_STATUS_WANT_CONNECTION	1
#define NETWORK_STATUS_WANT_DISCONNECT	2
#define NETWORK_STATUS_LOOKUP_PENDING	3

// reasons for making a scheduler RPC:
//

#define RPC_REASON_USER_REQ         1
#define RPC_REASON_RESULTS_DUE      2
#define RPC_REASON_NEED_WORK        3
#define RPC_REASON_TRICKLE_UP       4
#define RPC_REASON_ACCT_MGR_REQ     5
#define RPC_REASON_INIT             6
#define RPC_REASON_PROJECT_REQ      7

struct VERSION_INFO {
    int major;
    int minor;
    int release;
    int parse(MIOFILE&); 
    void write(MIOFILE&); 
    bool greater_than(VERSION_INFO&);
};

#ifdef _WIN32
#define RUN_MUTEX           "BoincSingleInstance"
#define REG_BLANK_NAME      "Blank"
#define REG_BLANK_TIME      "Blank Time"
#define REG_STARTUP_NAME    "BOINC"
#else
#define LOCK_FILE_NAME              "lockfile"
#endif

#endif
