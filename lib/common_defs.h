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

#ifndef _COMMON_DEFS_
#define _COMMON_DEFS_

#include "miofile.h"

// #defines or enums that are shared by more than one BOINC component
// (e.g. client, server, Manager, etc.)

#define GUI_RPC_PORT                                31416

#define COBBLESTONE_SCALE 200/86400e9
    // multiply normalized PFC by this to get Cobblestones

// run modes for CPU, GPU, network,
// controlled by Activity menu and snooze button
//
#define RUN_MODE_ALWAYS 1
#define RUN_MODE_AUTO   2
#define RUN_MODE_NEVER  3
#define RUN_MODE_RESTORE    4
    // restore permanent mode - used only in set_X_mode() GUI RPC

// values of ACTIVE_TASK::scheduler_state and ACTIVE_TASK::next_scheduler_state
// "SCHEDULED" is synonymous with "executing" except when CPU throttling
// is in use.
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

// priorities for client messages
//
#define MSG_INFO            1
    // write to stdout
    // GUI: show in event log
#define MSG_USER_ALERT      2
    // Conditions that require user intervention.
    // Text should be user-friendly.
    // write to stdout
    // GUI: show in event log in bold or red; show in notices tab
#define MSG_INTERNAL_ERROR  3
    // Conditions that indicate a problem or bug with BOINC itself,
    // or with a BOINC project or account manager.
    // treat same as MSG_INFO, but prepend with [error]
#define MSG_SCHEDULER_ALERT 4
    // high-priority message from scheduler
    // (used internally within the client;
    // changed to MSG_USER_ALERT before passing to manager)
    
// bitmap defs for task_suspend_reason, network_suspend_reason
// Note: doesn't need to be a bitmap, but keep for compatibility
//
enum SUSPEND_REASON {
    SUSPEND_REASON_BATTERIES = 1,
    SUSPEND_REASON_USER_ACTIVE = 2,
    SUSPEND_REASON_USER_REQ = 4,
    SUSPEND_REASON_TIME_OF_DAY = 8,
    SUSPEND_REASON_BENCHMARKS = 16,
    SUSPEND_REASON_DISK_SIZE = 32,
    SUSPEND_REASON_CPU_THROTTLE = 64,
    SUSPEND_REASON_NO_RECENT_INPUT = 128,
    SUSPEND_REASON_INITIAL_DELAY = 256,
    SUSPEND_REASON_EXCLUSIVE_APP_RUNNING = 512,
    SUSPEND_REASON_CPU_USAGE = 1024,
    SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED = 2048,
    SUSPEND_REASON_OS = 4096
};

// Values of RESULT::state in client.
// THESE MUST BE IN NUMERICAL ORDER
// (because of the > comparison in RESULT::computing_done())
//
#define RESULT_NEW                  0
    // New result
#define RESULT_FILES_DOWNLOADING    1
    // Input files for result (WU, app version) are being downloaded
#define RESULT_FILES_DOWNLOADED     2
    // Files are downloaded, result can be (or is being) computed
#define RESULT_COMPUTE_ERROR        3
    // computation failed; no file upload
#define RESULT_FILES_UPLOADING      4
    // Output files for result are being uploaded
#define RESULT_FILES_UPLOADED       5
    // Files are uploaded, notify scheduling server at some point
#define RESULT_ABORTED              6
    // result was aborted
#define RESULT_UPLOAD_FAILED        7
    // some output file permanent failure

// values of ACTIVE_TASK::task_state
//
#define PROCESS_UNINITIALIZED   0
    // process doesn't exist yet
#define PROCESS_EXECUTING       1
    // process is running, as far as we know
#define PROCESS_SUSPENDED       9
    // we've sent it a "suspend" message
#define PROCESS_ABORT_PENDING   5
    // process exceeded limits; send "abort" message, waiting to exit
#define PROCESS_QUIT_PENDING    8
    // we've sent it a "quit" message, waiting to exit
#define PROCESS_COPY_PENDING    10
    // waiting for async file copies to finish

// states in which the process has exited
#define PROCESS_EXITED          2
#define PROCESS_WAS_SIGNALED    3
#define PROCESS_EXIT_UNKNOWN    4
#define PROCESS_ABORTED         6
    // aborted process has exited
#define PROCESS_COULDNT_START   7

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
    bool prerelease;
    int parse(MIOFILE&); 
    void write(MIOFILE&); 
    bool greater_than(VERSION_INFO&);
};

#define RUN_MUTEX                   "BoincSingleInstance"
#define CLIENT_AUTH_FILENAME        "client_auth.xml"
#define LOCK_FILE_NAME              "lockfile"
#define GRAPHICS_APP_FILENAME       "graphics_app"
#define GUI_RPC_PASSWD_FILE         "gui_rpc_auth.cfg"
#define SS_CONFIG_FILE              "ss_config.xml"

#ifdef _WIN32
#define DEFAULT_SS_EXECUTABLE       "boincscr.exe"
#else
#define DEFAULT_SS_EXECUTABLE       "boincscr"
#endif


#endif
