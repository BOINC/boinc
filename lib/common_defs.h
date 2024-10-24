// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// #defines and enums that are shared by more than one BOINC component
// (e.g. client, server, Manager, etc.)
//
// Notes:
// 1) Some of these are replicated in PHP code: html/inc/common_defs.inc.
//    If you change something, check there.
// 2) The script py/db_def_to_py scrapes this file for #defines (not enums)
//    and makes variables for them.
//    AFAIK these aren't used, and we can remove this.
// 3) we should use enums instead of defines where appropriate

#ifndef BOINC_COMMON_DEFS_H
#define BOINC_COMMON_DEFS_H

#include "miofile.h"
#include "parse.h"

#define GUI_RPC_PORT 31416
    // for TCP connection
#define GUI_RPC_FILE "boinc_socket"
    // for Unix-domain connection

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
// "SCHEDULED" doesn't mean the task is actually running;
// e.g. it won't be running if tasks are suspended or CPU throttling is in use
//
enum SCHEDULER_STATE {
    CPU_SCHED_UNINITIALIZED   = 0,
    CPU_SCHED_PREEMPTED       = 1,
    CPU_SCHED_SCHEDULED       = 2
};

// official HTTP status codes

#define HTTP_STATUS_CONTINUE                100
#define HTTP_STATUS_OK                      200
#define HTTP_STATUS_PARTIAL_CONTENT         206
#define HTTP_STATUS_MOVED_PERM              301
#define HTTP_STATUS_MOVED_TEMP              302
#define HTTP_STATUS_CLIENT_ERROR            400
#define HTTP_STATUS_NOT_FOUND               404
#define HTTP_STATUS_PROXY_AUTH_REQ          407
#define HTTP_STATUS_RANGE_REQUEST_ERROR     416
#define HTTP_STATUS_EXPECTATION_FAILED      417
#define HTTP_STATUS_INTERNAL_SERVER_ERROR   500
#define HTTP_STATUS_NOT_IMPLEMENTED         501
#define HTTP_STATUS_BAD_GATEWAY             502
#define HTTP_STATUS_SERVICE_UNAVAILABLE     503
#define HTTP_STATUS_GATEWAY_TIMEOUT         504

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

// process priorities
// Unfortunately different areas of code use two different numbering schemes.
// The following is used in wrapper job.xml files
//
#define PROCESS_PRIORITY_UNSPECIFIED    0
#define PROCESS_PRIORITY_LOWEST     1
    // win: IDLE; unix: 19
#define PROCESS_PRIORITY_LOW        2
    // win: BELOW_NORMAL; unix: 10
#define PROCESS_PRIORITY_NORMAL     3
    // win: NORMAL; unix: 0
#define PROCESS_PRIORITY_HIGH       4
    // win: ABOVE_NORMAL; unix: -10
#define PROCESS_PRIORITY_HIGHEST    5
    // win: HIGH; unix: -16

// The following is used in cc_config.xml,
// and passed to apps in the APP_INIT_DATA structure
//
#define CONFIG_PRIORITY_UNSPECIFIED -1
#define CONFIG_PRIORITY_LOWEST      0
#define CONFIG_PRIORITY_LOW         1
#define CONFIG_PRIORITY_NORMAL      2
#define CONFIG_PRIORITY_HIGH        3
#define CONFIG_PRIORITY_HIGHEST     4
#define CONFIG_PRIORITY_REALTIME    5

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

// values for suspend_reason, network_suspend_reason
// Notes:
// - doesn't need to be a bitmap, but keep for compatibility
// - with new CPU throttling implementation (separate thread)
//   CLIENT_STATE.suspend_reason will never be SUSPEND_REASON_CPU_THROTTLE.
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
    SUSPEND_REASON_OS = 4096,
    SUSPEND_REASON_WIFI_STATE = 4097,
    SUSPEND_REASON_BATTERY_CHARGING = 4098,
    SUSPEND_REASON_BATTERY_OVERHEATED = 4099,
    SUSPEND_REASON_NO_GUI_KEEPALIVE = 4100
};

// battery state (currently used only for Android)
//
enum BATTERY_STATE {
    BATTERY_STATE_UNKNOWN=0,
    BATTERY_STATE_DISCHARGING,
    BATTERY_STATE_CHARGING,
    BATTERY_STATE_FULL,
    BATTERY_STATE_OVERHEATED
};

// states for sporadic apps
//
// client state
enum SPORADIC_CA_STATE {
    CA_NONE             = 0,
    CA_DONT_COMPUTE     = 1,
    // computing suspended (CPU and perhaps GPU) or other project have priority
    CA_COULD_COMPUTE    = 2,
    // not computing, but could
    CA_COMPUTING        = 3
    // go ahead and compute
};

// app state
enum SPORADIC_AC_STATE {
    AC_NONE                 = 0,
    AC_DONT_WANT_COMPUTE    = 1,
    AC_WANT_COMPUTE         = 2
};

// Values of RESULT::state in client.
// THESE MUST BE IN NUMERICAL ORDER
// (because of the > comparison in RESULT::computing_done())
// see html/inc/common_defs.inc
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

// Values of FILE_INFO::status.
// If the status is none of these,
// it's an error code indicating an unrecoverable error
// in the transfer of the file,
// or that the file was too big and was deleted.
//
#define FILE_NOT_PRESENT    0
#define FILE_PRESENT        1
#define FILE_VERIFY_PENDING 2

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

// values of batch.state
// see html/inc/common_defs.inc
//
#define BATCH_STATE_INIT            0
#define BATCH_STATE_IN_PROGRESS     1
#define BATCH_STATE_COMPLETE        2
    // "complete" means all workunits have either
    // a canonical result or an error
#define BATCH_STATE_ABORTED         3
#define BATCH_STATE_RETIRED         4
    // input/output files can be deleted,
    // result and workunit records can be purged.

// credit types
//
#define CREDIT_TYPE_FLOPS           0
#define CREDIT_TYPE_STORAGE         1
#define CREDIT_TYPE_NETWORK         2
#define CREDIT_TYPE_PROJECT         3

// An arg type for the 'dummy constructors' used to zero out structs.
// Something that won't occur naturally, so that
// COPROC c = 0;
// will give a compile error, rather than creating a COPROC
// using a dummy COPROC(int) constructor
//
typedef enum DUMMY_ENUM{DUMMY=0} DUMMY_TYPE;

struct TIME_STATS {
    double now;
        // the client's current time of day

    // we maintain an exponentially weighted average of these quantities:
    double on_frac;
        // the fraction of total time this host runs the client
    double connected_frac;
        // of the time this host runs the client,
        // the fraction it is connected to the Internet,
        // or -1 if not known
    double cpu_and_network_available_frac;
        // of the time this host runs the client,
        // the fraction it is connected to the Internet
        // AND network usage is allowed (by prefs and user toggle)
        // AND CPU usage is allowed
    double active_frac;
        // of the time this host runs the client,
        // the fraction it is enabled to use CPU
        // (as determined by preferences, manual suspend/resume, etc.)
    double gpu_active_frac;
        // same, GPU

    // info for the current session (i.e. run of the client)
    //
    double client_start_time;
        // start of current session
    double previous_uptime;
        // duration of previous session
    double session_active_duration;
        // time computation enabled
    double session_gpu_active_duration;
        // time GPU computation enabled

    // info since the client was first run
    //
    double total_start_time;
    double total_duration;
        // time BOINC client has run
    double total_active_duration;
        // time computation allowed
    double total_gpu_active_duration;
        // time GPU computation allowed

    int parse(XML_PARSER&);
    void print();
    TIME_STATS() {
        now = 0;
        on_frac = 1;
        connected_frac = 1;
        cpu_and_network_available_frac = 1;
        active_frac = 1;
        gpu_active_frac = 1;
        client_start_time = 0;
        previous_uptime = 0;
        session_active_duration = 0;
        session_gpu_active_duration = 0;
        total_start_time = 0;
        total_duration = 0;
        total_active_duration = 0;
        total_gpu_active_duration = 0;
    }
};

struct VERSION_INFO {
    int major;
    int minor;
    int release;
    bool prerelease;
    int parse(MIOFILE&);
    void write(MIOFILE&);
    bool greater_than(VERSION_INFO&);
    VERSION_INFO() {
        major = 0;
        minor = 0;
        release = 0;
        prerelease = true;
    }
};

// used for Android
//
struct DEVICE_STATUS {
    bool on_ac_power;
    bool on_usb_power;
    double battery_charge_pct;
    int battery_state;      // see above
    double battery_temperature_celsius;
    bool wifi_online;
    bool user_active;
    char device_name[256];
        // if present, a user-selected name for the device.
        // This will be stored by the client as hostinfo.domain_name,
        // and reported to schedulers.

    int parse(XML_PARSER&);
    DEVICE_STATUS() {
        on_ac_power = false;
        on_usb_power = false;
        battery_charge_pct = 0;
        battery_state =  BATTERY_STATE_UNKNOWN;
        battery_temperature_celsius = 0;
        wifi_online = false;
        user_active = false;
        strncpy(device_name, "", sizeof(device_name));
    }
};

#define RUN_MUTEX                   "BoincSingleInstance"
#define CLIENT_AUTH_FILENAME        "client_auth.xml"
#define LOCK_FILE_NAME              "lockfile"
#define GRAPHICS_APP_FILENAME       "graphics_app"
#define GUI_RPC_PASSWD_FILE         "gui_rpc_auth.cfg"
#define SS_CONFIG_FILE              "ss_config.xml"
#define ACCOUNT_DATA_FILENAME       "login_token.txt"
    // can't call this account*; it would be mistaken for an account file

#ifdef _WIN32
#define DEFAULT_SS_EXECUTABLE       "boincscr.exe"
#else
#define DEFAULT_SS_EXECUTABLE       "boincscr"
#endif

#define LINUX_CONFIG_FILE           "/etc/boinc-client/config.properties"

// Used by Manager and boinccmd to locate the data dir.
// You can define this in "configure" if you want.
//
#ifndef LINUX_DEFAULT_DATA_DIR
#define LINUX_DEFAULT_DATA_DIR      "/var/lib/boinc"
#endif

// impementations of Docker
enum DOCKER_TYPE {NONE, DOCKER, PODMAN};

#endif
