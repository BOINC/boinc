/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2025 University of California
 *
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This tries to be the same as lib/common_defs.h
 */
package edu.berkeley.boinc.utils

// run modes for CPU, GPU, network,
// controlled by Activity menu and snooze button
//
const val RUN_MODE_ALWAYS = 1
const val RUN_MODE_AUTO = 2
const val RUN_MODE_NEVER = 3
const val RUN_MODE_RESTORE = 4

// restore permanent mode - used only in set_X_mode() GUI RPC
// bitmap defs for task_suspend_reason, network_suspend_reason
// Note: doesn't need to be a bitmap, but keep for compatibility
//
const val SUSPEND_NOT_SUSPENDED = 0
const val SUSPEND_REASON_BATTERIES = 1
const val SUSPEND_REASON_USER_ACTIVE = 2
const val SUSPEND_REASON_USER_REQ = 4
const val SUSPEND_REASON_TIME_OF_DAY = 8
const val SUSPEND_REASON_BENCHMARKS = 16
const val SUSPEND_REASON_DISK_SIZE = 32
const val SUSPEND_REASON_CPU_THROTTLE = 64
const val SUSPEND_REASON_NO_RECENT_INPUT = 128
const val SUSPEND_REASON_INITIAL_DELAY = 256
const val SUSPEND_REASON_EXCLUSIVE_APP_RUNNING = 512
const val SUSPEND_REASON_CPU_USAGE = 1024
const val SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED = 2048
const val SUSPEND_REASON_OS = 4096
const val SUSPEND_REASON_WIFI_STATE = 4097
const val SUSPEND_REASON_BATTERY_CHARGING = 4098
const val SUSPEND_REASON_BATTERY_OVERHEATED = 4099

// Values of RESULT::state
// THESE MUST BE IN NUMERICAL ORDER
// (because of the > comparison in RESULT::computing_done())
//
const val RESULT_NEW = 0

// New result
const val RESULT_FILES_DOWNLOADING = 1

// Input files for result (WU, app version) are being downloaded
const val RESULT_FILES_DOWNLOADED = 2

// Files are downloaded, result can be (or is being) computed
const val RESULT_COMPUTE_ERROR = 3

// computation failed; no file upload
const val RESULT_FILES_UPLOADING = 4

// Output files for result are being uploaded
const val RESULT_FILES_UPLOADED = 5

// Files are uploaded, notify scheduling server at some point
const val RESULT_ABORTED = 6

// result was aborted
const val RESULT_UPLOAD_FAILED = 7

// some output file permanent failure
// values of ACTIVE_TASK::task_state
//
const val PROCESS_UNINITIALIZED = 0

// process doesn't exist yet
const val PROCESS_EXECUTING = 1

// process is running, as far as we know
const val PROCESS_SUSPENDED = 9

// we've sent it a "suspend" message
const val PROCESS_ABORT_PENDING = 5

// process exceeded limits; send "abort" message, waiting to exit
const val PROCESS_QUIT_PENDING = 8

// we've sent it a "quit" message, waiting to exit
// custom value, representing booleans of edu.berkeley.boinc.rpc.Result
const val RESULT_SUSPENDED_VIA_GUI = 100

// process goes back to PROCESS_UNINITIALIZED after suspending
const val RESULT_PROJECT_SUSPENDED = 101
const val RESULT_READY_TO_REPORT = 102
const val PROCESS_ABORTED = 6

// aborted process has exited
// reasons for making a scheduler RPC:
//
const val RPC_REASON_USER_REQ = 1
const val RPC_REASON_RESULTS_DUE = 2
const val RPC_REASON_NEED_WORK = 3
const val RPC_REASON_TRICKLE_UP = 4
const val RPC_REASON_ACCT_MGR_REQ = 5
const val RPC_REASON_INIT = 6
const val RPC_REASON_PROJECT_REQ = 7
