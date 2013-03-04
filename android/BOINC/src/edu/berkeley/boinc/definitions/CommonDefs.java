/*******************************************************************************
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
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
 ******************************************************************************/
package edu.berkeley.boinc.definitions;

/*
 * This tries to be the same as lib/common_defs.h
 */

public class CommonDefs {

	public static final Integer GUI_RPC_PORT = 31416;

	// run modes for CPU, GPU, network,
	// controlled by Activity menu and snooze button
	//
	public static final Integer RUN_MODE_ALWAYS=1;
	public static final Integer RUN_MODE_AUTO=2;
	public static final Integer RUN_MODE_NEVER  =3;
	public static final Integer RUN_MODE_RESTORE    =4;
	    // restore permanent mode - used only in set_X_mode() GUI RPC

	// values of ACTIVE_TASK::scheduler_state and ACTIVE_TASK::next_scheduler_state
	// "SCHEDULED" is synonymous with "executing" except when CPU throttling
	// is in use.
	public static final Integer CPU_SCHED_UNINITIALIZED   =0;
	public static final Integer CPU_SCHED_PREEMPTED       =1;
	public static final Integer CPU_SCHED_SCHEDULED       =2;

	// official HTTP status codes

	public static final Integer HTTP_STATUS_CONTINUE                =100;
	public static final Integer HTTP_STATUS_OK                      =200;
	public static final Integer HTTP_STATUS_PARTIAL_CONTENT         =206;
	public static final Integer HTTP_STATUS_MOVED_PERM             = 301;
	public static final Integer HTTP_STATUS_MOVED_TEMP              =302;
	public static final Integer HTTP_STATUS_NOT_FOUND              = 404;
	public static final Integer HTTP_STATUS_PROXY_AUTH_REQ         = 407;
	public static final Integer HTTP_STATUS_RANGE_REQUEST_ERROR    = 416;
	public static final Integer HTTP_STATUS_INTERNAL_SERVER_ERROR  = 500;
	public static final Integer HTTP_STATUS_SERVICE_UNAVAILABLE    = 503;

	// graphics messages
	//
	public static final Integer MODE_UNSUPPORTED       = 0;
	public static final Integer MODE_HIDE_GRAPHICS     = 1;
	public static final Integer MODE_WINDOW            = 2;
	public static final Integer MODE_FULLSCREEN        = 3;
	public static final Integer MODE_BLANKSCREEN       = 4;
	public static final Integer MODE_REREAD_PREFS      = 5;
	public static final Integer MODE_QUIT              = 6;
	public static final Integer NGRAPHICS_MSGS  =7;

	// priorities for client messages
	//
	public static final Integer MSG_INFO           = 1;
	    // write to stdout
	    // GUI: show in event log
	public static final Integer MSG_USER_ALERT     = 2;
	    // Conditions that require user intervention.
	    // Text should be user-friendly.
	    // write to stdout
	    // GUI: show in event log in bold or red; show in notices tab
	public static final Integer MSG_INTERNAL_ERROR = 3;
	    // Conditions that indicate a problem or bug with BOINC itself,
	    // or with a BOINC project or account manager.
	    // treat same as MSG_INFO, but prepend with [error]
	public static final Integer MSG_SCHEDULER_ALERT= 4;
	    // high-priority message from scheduler
	    // (used internally within the client;
	    // changed to MSG_USER_ALERT before passing to manager)

	// bitmap defs for task_suspend_reason, network_suspend_reason
	// Note: doesn't need to be a bitmap, but keep for compatibility
	//

	public static final Integer SUSPEND_NOT_SUSPENDED = 0;
	public static final Integer SUSPEND_REASON_BATTERIES = 1;
	public static final Integer SUSPEND_REASON_USER_ACTIVE = 2;
	public static final Integer SUSPEND_REASON_USER_REQ = 4;
	public static final Integer SUSPEND_REASON_TIME_OF_DAY = 8;
	public static final Integer SUSPEND_REASON_BENCHMARKS = 16;
	public static final Integer SUSPEND_REASON_DISK_SIZE = 32;
	public static final Integer SUSPEND_REASON_CPU_THROTTLE = 64;
	public static final Integer SUSPEND_REASON_NO_RECENT_INPUT = 128;
	public static final Integer SUSPEND_REASON_INITIAL_DELAY = 256;
	public static final Integer SUSPEND_REASON_EXCLUSIVE_APP_RUNNING = 512;
	public static final Integer SUSPEND_REASON_CPU_USAGE = 1024;
	public static final Integer SUSPEND_REASON_NETWORK_QUOTA_EXCEEDED = 2048;
	public static final Integer SUSPEND_REASON_OS = 4096;
	public static final Integer SUSPEND_REASON_WIFI_STATE = 8192;

	// Values of RESULT::state
	// THESE MUST BE IN NUMERICAL ORDER
	// (because of the > comparison in RESULT::computing_done())
	//
	public static final Integer RESULT_NEW                  =0;
	    // New result
	public static final Integer RESULT_FILES_DOWNLOADING    =1;
	    // Input files for result (WU, app version) are being downloaded
	public static final Integer RESULT_FILES_DOWNLOADED    = 2;
	    // Files are downloaded, result can be (or is being) computed
	public static final Integer RESULT_COMPUTE_ERROR       = 3;
	    // computation failed; no file upload
	public static final Integer RESULT_FILES_UPLOADING      =4;
	    // Output files for result are being uploaded
	public static final Integer RESULT_FILES_UPLOADED      = 5;
	    // Files are uploaded, notify scheduling server at some point
	public static final Integer RESULT_ABORTED             = 6;
	    // result was aborted
	public static final Integer RESULT_UPLOAD_FAILED       = 7;
	    // some output file permanent failure

	// values of ACTIVE_TASK::task_state
	//
	public static final Integer PROCESS_UNINITIALIZED  = 0;
	    // process doesn't exist yet
	public static final Integer PROCESS_EXECUTING      = 1;
	    // process is running, as far as we know
	public static final Integer PROCESS_SUSPENDED      = 9;
	    // we've sent it a "suspend" message
	public static final Integer PROCESS_ABORT_PENDING  = 5;
	    // process exceeded limits; send "abort" message, waiting to exit
	public static final Integer PROCESS_QUIT_PENDING   = 8;
	    // we've sent it a "quit" message, waiting to exit

	
	// states in which the process has exited
	public static final Integer PROCESS_EXITED         = 2;
	public static final Integer PROCESS_WAS_SIGNALED   = 3;
	public static final Integer PROCESS_EXIT_UNKNOWN   = 4;
	public static final Integer PROCESS_ABORTED        = 6;
	    // aborted process has exited
	public static final Integer PROCESS_COULDNT_START  = 7;

	// values of "network status"
	//
	public static final Integer NETWORK_STATUS_ONLINE         =          0;
	public static final Integer NETWORK_STATUS_WANT_CONNECTION = 1;
	public static final Integer NETWORK_STATUS_WANT_DISCONNECT = 2;
	public static final Integer NETWORK_STATUS_LOOKUP_PENDING  = 3;

	// reasons for making a scheduler RPC:
	//
	public static final Integer RPC_REASON_USER_REQ        = 1;
	public static final Integer RPC_REASON_RESULTS_DUE    =  2;
	public static final Integer RPC_REASON_NEED_WORK       = 3;
	public static final Integer RPC_REASON_TRICKLE_UP      = 4;
	public static final Integer RPC_REASON_ACCT_MGR_REQ    = 5;
	public static final Integer RPC_REASON_INIT            = 6;
	public static final Integer RPC_REASON_PROJECT_REQ     = 7;

	public static final String CLIENT_AUTH_FILENAME       = "client_auth.xml";
	public static final String GUI_RPC_PASSWD_FILE        = "gui_rpc_auth.cfg";



}
