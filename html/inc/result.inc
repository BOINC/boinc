<?php
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

require_once("../inc/translation.inc");
require_once("../inc/dir_hier.inc");
require_once("../inc/common_defs.inc");

// used by app_version_string(), see below
//
global $apps;
global $app_versions;
global $platforms;
$apps = array();
$app_versions = array();
$platforms = array();

function get_app($id) {
    global $apps;
    if (!array_key_exists($id, $apps)) {
        $apps[$id] = BoincApp::lookup_id($id);
    }
    return $apps[$id];
}

function get_app_version($id) {
    global $app_versions;
    if (!array_key_exists($id, $app_versions)) {
        $app_versions[$id] = BoincAppVersion::lookup_id($id);
    }
    return $app_versions[$id];
}

function get_platform($id) {
    global $platforms;
    if (!array_key_exists($id, $platforms)) {
        $platforms[$id] = BoincPlatform::lookup_id($id);
    }
    return $platforms[$id];
}

function anon_platform_string($result, $rsc_name=null) {
    $app = get_app($result->appid);
    $n = $app->user_friendly_name."<br>".  tra("Anonymous platform");
    if ($rsc_name) {
        $n .= " ($rsc_name)";
    }
    return $n;
}

function app_version_string($result) {
    global $apps, $app_versions;

    $id = $result->app_version_id;
    switch ($id) {
    case 0: return "---";
    case ANON_PLATFORM_UNKNOWN:
        return anon_platform_string($result);
    case ANON_PLATFORM_CPU:
        return anon_platform_string($result, tra("CPU"));
    case ANON_PLATFORM_NVIDIA:
        return anon_platform_string($result, tra("NVIDIA GPU"));
    case ANON_PLATFORM_ATI:
        return anon_platform_string($result, tra("AMD GPU"));
    case ANON_PLATFORM_INTEL_GPU:
        return anon_platform_string($result, tra("Intel GPU"));
    case ANON_PLATFORM_APPLE_GPU:
        return anon_platform_string($result, tra("Apple GPU"));
    }
    $av = get_app_version($id);
    if (!$av) {
        return "missing app version $id";
    }
    $app = get_app($av->appid);
    $platform = get_platform($av->platformid);
    $n = $app->user_friendly_name;
    $v = sprintf("%d.%02d", $av->version_num/100, $av->version_num%100);
    if ($av->plan_class) {
        $c = "($av->plan_class)";
    } else {
        $c = "";
    }
    return "$n v$v $c<br>$platform->name";
}

function result_granted_credit_string($result, $string_to_show) {
    if ($result->server_state == RESULT_SERVER_STATE_IN_PROGRESS && $result->granted_credit > 0) {
        return $string_to_show;
    }
    if ($result->server_state <> RESULT_SERVER_STATE_OVER) return "---";
    switch($result->outcome) {
    case RESULT_OUTCOME_SUCCESS:
        switch ($result->validate_state) {
        case VALIDATE_STATE_INIT:
        case VALIDATE_STATE_INCONCLUSIVE:
            return tra("pending");
        }
        return $string_to_show;
    default:
        if ($result->granted_credit > 0) {
            return $string_to_show;
        }
        return "---";
    }
}

// various result states that users can filter on

define("STATE_ALL", 0);
define("STATE_IN_PROGRESS", 1);
define("STATE_PENDING", 2);
define("STATE_INCONCLUSIVE", 3);
define("STATE_VALID", 4);
define("STATE_INVALID", 5);
define("STATE_ERROR", 6);
define("NSTATES", 7);

$state_name = array(
    tra("All"),
    tra("In progress"),
    tra("Validation pending"),
    tra("Validation inconclusive"),
    tra("Valid"),
    tra("Invalid"),
    tra("Error"),
);

$state_clause = array(
    "",
    sprintf(
        ' and server_state=%d ',
        RESULT_SERVER_STATE_IN_PROGRESS
    ),
    sprintf(
        ' and server_state=%d and outcome=%d and validate_state=%d ',
        RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS, VALIDATE_STATE_INIT
    ),
    sprintf(
        ' and server_state=%d and outcome=%d and validate_state=%d ',
        RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS,
        VALIDATE_STATE_INCONCLUSIVE
    ),
    sprintf(
        ' and server_state=%d and outcome=%d and validate_state=%d ',
        RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS, VALIDATE_STATE_VALID
    ),
    sprintf(
        ' and server_state=%d and (outcome=%d or (outcome=%d and (validate_state=%d or validate_state=%d or validate_state=%d))) ',
        RESULT_SERVER_STATE_OVER,
        RESULT_OUTCOME_VALIDATE_ERROR, RESULT_OUTCOME_SUCCESS,
        VALIDATE_STATE_INVALID, VALIDATE_STATE_NO_CHECK,
        VALIDATE_STATE_TOO_LATE
    ),
    sprintf(
        ' and server_state=%d and (outcome=%d or outcome=%d or outcome=%d) ',
        RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_CLIENT_ERROR,
        RESULT_OUTCOME_NO_REPLY, RESULT_OUTCOME_CLIENT_DETACHED
    )
);

function state_num($result) {
    if ($result->server_state == RESULT_SERVER_STATE_IN_PROGRESS) return 1;
    if ($result->server_state == RESULT_SERVER_STATE_OVER && $result->outcome == RESULT_OUTCOME_SUCCESS) {
        if ($result->validate_state == VALIDATE_STATE_INIT) {
            return STATE_PENDING;
        }
        if ($result->validate_state == VALIDATE_STATE_INCONCLUSIVE) {
            return STATE_INCONCLUSIVE;
        }
    }
    if ($result->server_state == RESULT_SERVER_STATE_OVER
        && $result->outcome == RESULT_OUTCOME_SUCCESS
        && $result->validate_state == VALIDATE_STATE_VALID
    ) {
        return STATE_VALID;
    }
    if ($result->server_state == RESULT_SERVER_STATE_OVER
        && ($result->outcome == RESULT_OUTCOME_VALIDATE_ERROR
            || ($result->outcome == RESULT_OUTCOME_SUCCESS
                && ($result->validate_state == VALIDATE_STATE_INVALID
                    || $result->validate_state == VALIDATE_STATE_NO_CHECK
                    || $result->validate_state == VALIDATE_STATE_TOO_LATE
                )
            )
        )
    ) {
        return STATE_INVALID;
    }
    if ($result->server_state == RESULT_SERVER_STATE_OVER
        && ($result->outcome == RESULT_OUTCOME_CLIENT_ERROR
            || $result->outcome == RESULT_OUTCOME_NO_REPLY
            || $result->outcome == RESULT_OUTCOME_CLIENT_DETACHED
        )
    ) {
        return STATE_ERROR;
    }
    return 0;
}

function state_string($result) {
    switch ($result->server_state) {
    case RESULT_SERVER_STATE_INACTIVE: return tra("Inactive");
    case RESULT_SERVER_STATE_UNSENT: return tra("Unsent");
    case RESULT_SERVER_STATE_IN_PROGRESS: return tra("In progress");
    case RESULT_SERVER_STATE_OVER:
        switch ($result->outcome) {
        case RESULT_OUTCOME_SUCCESS:
            switch ($result->validate_state) {
            case VALIDATE_STATE_INIT: return tra("Completed, waiting for validation");
            case VALIDATE_STATE_VALID: return tra("Completed and validated");
            case VALIDATE_STATE_INVALID: return tra("Completed, marked as invalid");
            case VALIDATE_STATE_NO_CHECK: return tra("Completed, can't validate");
            case VALIDATE_STATE_INCONCLUSIVE: return tra("Completed, validation inconclusive");
            case VALIDATE_STATE_TOO_LATE: return tra("Completed, too late to validate");
            }
            return tra("Completed");
        case RESULT_OUTCOME_COULDNT_SEND: return tra("Couldn't send");
        case RESULT_OUTCOME_CLIENT_ERROR:
            if ($result->exit_status == -221
                || $result->exit_status == 202
            ) {
                return tra("Cancelled by server");
            }
            if ($result->exit_status == -233
                || $result->exit_status == 200
            ) {
                return tra("Not started by deadline - canceled");
            }
            switch($result->client_state) {
            case RESULT_FILES_DOWNLOADING: return tra("Error while downloading");
            case RESULT_FILES_DOWNLOADED:
            case RESULT_COMPUTE_ERROR: return tra("Error while computing");
            case RESULT_FILES_UPLOADING: return tra("Error while uploading");
            case RESULT_ABORTED: return tra("Aborted");
            case RESULT_UPLOAD_FAILED: return tra("Upload failed");
            }
            return tra("Error");
        case RESULT_OUTCOME_NO_REPLY: return tra("Timed out - no response");
        case RESULT_OUTCOME_DIDNT_NEED: return tra("Didn't need");
        case RESULT_OUTCOME_VALIDATE_ERROR: return tra("Validate error");
        case RESULT_OUTCOME_CLIENT_DETACHED: return tra("Abandoned");
        }
    }
    return tra("Unknown");
}

function result_server_state_string($result) {
    switch($result->server_state) {
    case RESULT_SERVER_STATE_INACTIVE: return tra("Inactive");
    case RESULT_SERVER_STATE_UNSENT: return tra("Unsent");
    case RESULT_SERVER_STATE_IN_PROGRESS: return tra("In progress");
    case RESULT_SERVER_STATE_OVER: return tra("Over");
    }
    return tra("Unknown");
}

function result_outcome_string($result) {
    switch($result->outcome) {
    case RESULT_OUTCOME_INIT: return "---";
    case RESULT_OUTCOME_SUCCESS: return tra("Success");
    case RESULT_OUTCOME_COULDNT_SEND: return tra("Couldn't send");
    case RESULT_OUTCOME_CLIENT_ERROR:
        if ($result->exit_status <> -221) {
            return tra("Computation error");
        }
        return tra("Redundant result");
    case RESULT_OUTCOME_NO_REPLY: return tra("No reply");
    case RESULT_OUTCOME_DIDNT_NEED: return tra("Didn't need");
    case RESULT_OUTCOME_VALIDATE_ERROR: return tra("Validate error");
    case RESULT_OUTCOME_CLIENT_DETACHED: return tra("Abandoned");
    }
    return tra("Unknown");
}

function result_client_state_string($result) {
    switch($result->client_state) {
    case RESULT_NEW: return tra("New");
    case RESULT_FILES_DOWNLOADING: return tra("Downloading");
    case RESULT_FILES_DOWNLOADED: return tra("Processing");
    case RESULT_COMPUTE_ERROR: return tra("Compute error");
    case RESULT_FILES_UPLOADING: return tra("Uploading");
    case RESULT_FILES_UPLOADED: return tra("Done");
    case RESULT_ABORTED:
        if ($result->exit_status == -221 || $result->exit_status == 202) {
            return tra("Cancelled by server");
        }
        return tra("Aborted by user");
    case RESULT_UPLOAD_FAILED: return tra("Upload failed");
    }
    return tra("Unknown");
}

function validate_state_str($result) {
    switch($result->validate_state) {
    case VALIDATE_STATE_INIT: return tra("Initial");
    case VALIDATE_STATE_VALID: return tra("Valid");
    case VALIDATE_STATE_INVALID:
        if ($result->exit_status <> -221) {
            return tra("Invalid");
        }
        return tra("Not necessary");
    case VALIDATE_STATE_NO_CHECK: return tra("Workunit error - check skipped");
    case VALIDATE_STATE_INCONCLUSIVE: return tra("Checked, but no consensus yet");
    case VALIDATE_STATE_TOO_LATE: return tra("Task was reported too late to validate");
    }
    return tra("Unknown");
}

function assimilate_state_str($s) {
    switch($s) {
    case ASSIMILATE_INIT: return "Initial";
    case ASSIMILATE_READY: return "Ready to assimilate";
    case ASSIMILATE_DONE: return "Assimilated";
    }
    return "Unknown";
}

function file_delete_state_str($s) {
    switch($s) {
    case FILE_DELETE_INIT: return "Initial";
    case FILE_DELETE_READY: return "Ready to delete";
    case FILE_DELETE_DONE: return "Deleted";
    case FILE_DELETE_ERROR: return "Delete Error";
    }
    return "Unknown";
}

// convert WU error bitmask to str.
// If $color, add HTML red color
//
function wu_error_mask_str($s, $color=false) {
    $x = "";
    if ($s & WU_ERROR_COULDNT_SEND_RESULT) {
        $x = $x." ".tra("Couldn't send result");
        $s -= WU_ERROR_COULDNT_SEND_RESULT;
    }
    if ($s & WU_ERROR_TOO_MANY_ERROR_RESULTS) {
        $x = $x." ".tra("Too many errors (may have bug)");
        $s -= WU_ERROR_TOO_MANY_ERROR_RESULTS;
    }
    if ($s & WU_ERROR_TOO_MANY_SUCCESS_RESULTS) {
        $x = $x." ".tra("Too many results (may be nondeterministic)");
        $s -= WU_ERROR_TOO_MANY_SUCCESS_RESULTS;
    }
    if ($s & WU_ERROR_TOO_MANY_TOTAL_RESULTS) {
        $x = $x." ".tra("Too many total results");
        $s -= WU_ERROR_TOO_MANY_TOTAL_RESULTS;
    }
    if ($s & WU_ERROR_CANCELLED) {
        $x = $x." ".tra("WU cancelled");
        $s -= WU_ERROR_CANCELLED;
    }
    if ($s & WU_ERROR_NO_CANONICAL_RESULT) {
        $x = $x." ".tra("Canonical result is missing");
        $s -= WU_ERROR_NO_CANONICAL_RESULT;
    }
    if ($s) {
        $x = $x." ".tra("Unrecognized Error: %1", $s);
    }
    if ($color) {
        return sprintf('<font color="#ff3333">%s</font>', $x);
    }
    return $x;
}

function result_page_url($info) {
    $c = $info->clause;
    $o = $info->offset;
    $sn = $info->show_names;
    $st = $info->state;
    $appid = $info->appid;
    return "results.php?$c&amp;offset=$o&amp;show_names=$sn&amp;state=$st&amp;appid=$appid";
}

function result_table_start($show_wu_link, $show_host_link, $info) {
    start_table('table-striped');
    $x = array();
    $a = array(null);

    if ($info) {
        if ($info->show_names) {
            $i2 = clone $info;
            $i2->show_names = 0;
            $url = result_page_url($i2);
            $x[] = tra("Task name")."<br><small>".tra("click for details")."<br><a href=$url>".tra("Show IDs")."</a></small>";
        } else {
            $i2 = clone $info;
            $i2->show_names = 1;
            $url = result_page_url($i2);
            $x[] = "Task<br><small>".tra("click for details")."<br><a href=$url>".tra("Show names")."</a></small>";
        }
    } else {
        $x[] = tra("Task")."<br><small>".tra("click for details")."</small>";
    }
    if ($show_wu_link) {
        $x[] = tra("Work unit")."<br><small>".tra("click for details")."</small>";
        $a[] = null;
        if (function_exists('project_result_info_heading')) {
            $y = project_result_info_heading();
            if ($y) {
                $x[] = $y;
                $a[] = null;
            }
        }
    }
    if ($show_host_link) {
        $x[] = tra("Computer");
        $a[] = null;
    }
    $x[] = tra("Sent");
    $x[] = tra("Time reported<br />or deadline")."
        <br><small><a href=\"explain_state.php?field=result_time\">".tra("explain")."</a></small>";
    $x[] = tra("Status");
    $a[] = null;
    $a[] = null;
    $a[] = null;
    $x[] = tra("Run time<br />(sec)");
    $x[] = tra("CPU time<br />(sec)");
    $x[] = tra("Credit");
    $a[] = ALIGN_RIGHT;
    $a[] = ALIGN_RIGHT;
    $a[] = ALIGN_RIGHT;
    $x[] = tra("Application");
    $a[] = null;
    row_heading_array($x, $a);
}

// was result invalid or timed out?
//
function bad_result($result) {
    if ($result->validate_state == 2) return true;
    if (!$result->received_time && ($result->report_deadline < time())) return true;
    return false;
}

function show_result_row($result, $show_wu_link, $show_host_link, $show_name) {
    $s = time_str($result->sent_time);
    // if result has been reported, show the received time,
    // else show the reporting deadline in green if in the future
    // and in red if in the past.
    //
    if ($result->received_time) {
        $r = time_str($result->received_time);
    } else if ($result->report_deadline) {
        if ($result->report_deadline>time()) {
            $r = "<font color='#33cc33'>" . time_str($result->report_deadline) . "</font>";
        } else {
            $r = "<font color='#ff3333'>" . time_str($result->report_deadline) . "</font>";
        }
    } else {
        $r = "---";
    }
    $ss = state_string($result);
    $result_granted_credit = format_credit($result->granted_credit);
    $result_granted_credit = result_granted_credit_string($result, $result_granted_credit);
    echo "<tr>";
    if ($show_name) {
        $x = $result->name;
    } else {
        $x = $result->id;
    }
    echo "<td><a href=\"result.php?resultid=$result->id\">$x</a></td>\n";
    if ($show_wu_link) {
        echo "<td><a href=\"workunit.php?wuid=$result->workunitid\">$result->workunitid</a></td>\n";
        if (function_exists('project_result_info_link')) {
            $x = project_result_info_link($result);
            if ($x) {
                echo "<td>$x</td>\n";
            }
        }
    }
    if ($show_host_link) {
        echo "<td>", host_link($result->hostid), "</td>\n";
    }
    if ($result->server_state <> 5) {
        $cpu_time = "---";
        $elapsed_time = "---";
    } else {
        $cpu_time = number_format($result->cpu_time, 2);
        $elapsed_time = number_format($result->elapsed_time, 2);
    }
    $v = app_version_string($result);
    echo "
        <td>$s</td>
        <td>$r</td>
        <td>$ss</td>
        <td align=right>$elapsed_time</td>
        <td align=right>$cpu_time</td>
        <td align=right>$result_granted_credit</td>
        <td>$v</td>
        </tr>
    ";
}

function version_string($version_num) {
    if (!$version_num) {
        return '---';
    } else {
        return sprintf("%.2f", $version_num/100);
    }
}

// transform a 4byte integer to an 8 character hex representation
//
// @return String The hex representation (uppercased) starting with 0x. Length is always 10 characters!
// @param Integer $dec A signed integer
//
function int2hex($dec) {
    return "0x".strtoupper(substr(sprintf("%08x",$dec), -8));
}

// Decode a windows error number into semi-human-readable,
// taken from ntstatus.h for selected codes
// They need to be uppercase after 0x!
//
// @return String A human readable error message
// @param Integer $x An error number
//
function windows_error_code_str($x){
    $h=int2hex($x);
    switch($h){
    case "0xC0000005": return "STATUS_ACCESS_VIOLATION";
    case "0xC000001D": return "STATUS_ILLEGAL_INSTRUCTION";
    case "0xC0000094": return "STATUS_INTEGER_DIVIDE_BY_ZERO";
    case "0xC00000FD": return "STATUS_STACK_OVERFLOW";
    case "0xC0000135": return "STATUS_DLL_NOT_FOUND";
    case "0xC0000139": return "STATUS_ENTRYPOINT_NOT_FOUND";
    case "0xC0000142": return "STATUS_DLL_INIT_FAILED";
    case "0xC000026B": return "STATUS_DLL_INIT_FAILED_LOGOFF";
    case "0xC0000409": return "STATUS_STACK_BUFFER_OVERRUN";
    //case "": return "";
    default: return "Unknown error code";
    }
}

// Decode ErrorNumber into semi-human-readable,
// taken from lib/error_numbers.h keep this up to date
//
// @return String A human readable error message
// @param Integer $x An error number
//
function error_code_str($x){
    // severe Windows error numbers are always large negative integers
    if ($x<-400) return windows_error_code_str($x);
    switch($x){
    case 0: return "";
    case 192: return "EXIT_STATEFILE_WRITE";
    case 193: return "EXIT_SIGNAL";
    case 194: return "EXIT_ABORTED_BY_CLIENT";
    case 195: return "EXIT_CHILD_FAILED";
    case 196: return "EXIT_DISK_LIMIT_EXCEEDED";
    case 197: return "EXIT_TIME_LIMIT_EXCEEDED";
    case 198: return "EXIT_MEM_LIMIT_EXCEEDED";
    case 199: return "EXIT_CLIENT_EXITING";
    case 200: return "EXIT_UNSTARTED_LATE";
    case 201: return "EXIT_MISSING_COPROC";
    case 202: return "EXIT_ABORTED_BY_PROJECT";
    case 203: return "EXIT_ABORTED_VIA_GUI";
    case 204: return "EXIT_UNKNOWN";
    case 205: return "EXIT_OUT_OF_MEMORY";
    case 206: return "EXIT_INIT_FAILURE";
    case 207: return "EXIT_NO_SUB_TASKS";
    case 208: return "EXIT_SUB_TASK_FAILURE";
    case -100: return "ERR_SELECT";
    case -102: return "ERR_READ";
    case -103: return "ERR_WRITE";
    case -104: return "ERR_FREAD";
    case -105: return "ERR_FWRITE";
    case -106: return "ERR_IO";
    case -107: return "ERR_CONNECT";
    case -108: return "ERR_FOPEN";
    case -109: return "ERR_RENAME";
    case -110: return "ERR_UNLINK";
    case -111: return "ERR_OPENDIR";
    case -112: return "ERR_XML_PARSE";
    case -113: return "ERR_GETHOSTBYNAME";
    case -114: return "ERR_GIVEUP_DOWNLOAD";
    case -115: return "ERR_GIVEUP_UPLOAD";
    case -116: return "ERR_NULL";
    case -117: return "ERR_NEG";
    case -118: return "ERR_BUFFER_OVERFLOW";
    case -119: return "ERR_MD5_FAILED";
    case -120: return "ERR_RSA_FAILED";
    case -121: return "ERR_OPEN";
    case -122: return "ERR_DUP2";
    case -123: return "ERR_NO_SIGNATURE";
    case -124: return "ERR_THREAD";
    case -125: return "ERR_SIGNAL_CATCH";
    case -126: return "ERR_QUIT_REQUEST";
    case -127: return "ERR_UPLOAD_TRANSIENT";
    case -128: return "ERR_UPLOAD_PERMANENT";
    case -129: return "ERR_IDLE_PERIOD";
    case -130: return "ERR_ALREADY_ATTACHED";
    case -131: return "ERR_FILE_TOO_BIG";
    case -132: return "ERR_GETRUSAGE";
    case -133: return "ERR_BENCHMARK_FAILED";
    case -134: return "ERR_BAD_HEX_FORMAT";
    case -135: return "ERR_USER_REJECTED";
    case -136: return "ERR_DB_NOT_FOUND";
    case -137: return "ERR_DB_NOT_UNIQUE";
    case -138: return "ERR_DB_CANT_CONNECT";
    case -139: return "ERR_GETS";
    case -140: return "ERR_SCANF";
    case -141: return "ERR_STRCHR";
    case -142: return "ERR_STRSTR";
    case -143: return "ERR_READDIR";
    case -144: return "ERR_SHMGET";
    case -145: return "ERR_SHMCTL";
    case -146: return "ERR_SHMAT";
    case -147: return "ERR_FORK";
    case -148: return "ERR_EXEC";
    case -149: return "ERR_NOT_EXITED";
    case -150: return "ERR_NOT_IMPLEMENTED";
    case -151: return "ERR_GETHOSTNAME";
    case -152: return "ERR_NETOPEN";
    case -153: return "ERR_SOCKET";
    case -154: return "ERR_FCNTL";
    case -155: return "ERR_AUTHENTICATOR";
    case -156: return "ERR_SCHED_SHMEM";
    case -157: return "ERR_ASYNCSELECT";
    case -158: return "ERR_BAD_RESULT_STATE";
    case -159: return "ERR_DB_CANT_INIT";
    case -160: return "ERR_NOT_UNIQUE";
    case -161: return "ERR_NOT_FOUND";
    case -162: return "ERR_NO_EXIT_STATUS";
    case -163: return "ERR_FILE_MISSING";
    case -164: return "ERR_NESTED_UNHANDLED_EXCEPTION_DETECTED";
    case -165: return "ERR_SEMGET";
    case -166: return "ERR_SEMCTL";
    case -167: return "ERR_SEMOP";
    case -168: return "ERR_FTOK";
    case -169: return "ERR_SOCKS_UNKNOWN_FAILURE";
    case -170: return "ERR_SOCKS_REQUEST_FAILED";
    case -171: return "ERR_SOCKS_BAD_USER_PASS";
    case -172: return "ERR_SOCKS_UNKNOWN_SERVER_VERSION";
    case -173: return "ERR_SOCKS_UNSUPPORTED";
    case -174: return "ERR_SOCKS_CANT_REACH_HOST";
    case -175: return "ERR_SOCKS_CONN_REFUSED";
    case -176: return "ERR_TIMER_INIT";
    case -177: return "ERR_RSC_LIMIT_EXCEEDED";
    case -178: return "ERR_INVALID_PARAM";
    case -179: return "ERR_SIGNAL_OP";
    case -180: return "ERR_BIND";
    case -181: return "ERR_LISTEN";
    case -182: return "ERR_TIMEOUT";
    case -183: return "ERR_PROJECT_DOWN";
    case -184: return "ERR_HTTP_ERROR";
    case -185: return "ERR_RESULT_START";
    case -186: return "ERR_RESULT_DOWNLOAD";
    case -187: return "ERR_RESULT_UPLOAD";
    case -189: return "ERR_INVALID_URL";
    case -190: return "ERR_MAJOR_VERSION";
    case -191: return "ERR_NO_OPTION";
    case -192: return "ERR_MKDIR";
    case -193: return "ERR_INVALID_EVENT";
    case -194: return "ERR_ALREADY_RUNNING";
    case -195: return "ERR_NO_APP_VERSION";
    case -196: return "ERR_WU_USER_RULE";
    case -197: return "ERR_ABORTED_VIA_GUI";
    case -198: return "ERR_INSUFFICIENT_RESOURCE";
    case -199: return "ERR_RETRY";
    case -200: return "ERR_WRONG_SIZE";
    case -201: return "ERR_USER_PERMISSION";
    case -202: return "ERR_SHMEM_NAME";
    case -203: return "ERR_NO_NETWORK_CONNECTION";
    case -204: return "ERR_IN_PROGRESS";
    case -205: return "ERR_BAD_EMAIL_ADDR";
    case -206: return "ERR_BAD_PASSWD";
    case -207: return "ERR_NONUNIQUE_EMAIL";
    case -208: return "ERR_ACCT_CREATION_DISABLED";
    case -209: return "ERR_ATTACH_FAIL_INIT";
    case -210: return "ERR_ATTACH_FAIL_DOWNLOAD";
    case -211: return "ERR_ATTACH_FAIL_PARSE";
    case -212: return "ERR_ATTACH_FAIL_BAD_KEY";
    case -213: return "ERR_ATTACH_FAIL_FILE_WRITE";
    case -214: return "ERR_ATTACH_FAIL_SERVER_ERROR";
    case -215: return "ERR_SIGNING_KEY";
    case -216: return "ERR_FFLUSH";
    case -217: return "ERR_FSYNC";
    case -218: return "ERR_TRUNCATE";
    case -219: return "ERR_WRONG_URL";
    case -220: return "ERR_DUP_NAME";
    case -221: return "ERR_ABORTED_BY_PROJECT";
    case -222: return "ERR_GETGRNAM";
    case -223: return "ERR_CHOWN";
    case -224: return "ERR_FILE_NOT_FOUND";
    case -225: return "ERR_BAD_FILENAME";
    case -226: return "ERR_TOO_MANY_EXITS";
    case -227: return "ERR_RMDIR";
    case -228: return "ERR_CHILD_FAILED";
    case -229: return "ERR_SYMLINK";
    case -230: return "ERR_DB_CONN_LOST";
    case -231: return "ERR_CRYPTO";
    case -232: return "ERR_ABORTED_ON_EXIT";
    case -233: return "ERR_UNSTARTED_LATE";
    case -234: return "ERR_MISSING_COPROC";
    case -235: return "ERR_PROC_PARSE";
    default: return "Unknown error code";
    }
}

function exit_status_string($x) {
    return $x." (".int2hex($x).") ".error_code_str($x);
}

function show_result($result, $show_outfile_links=false) {
    start_table();
    row2(tra("Name"), $result->name);
    row2(tra("Workunit"), "<a href=\"workunit.php?wuid=$result->workunitid\">$result->workunitid</a>");
    row2(tra("Created"), time_str($result->create_time));
    row2(tra("Sent"), time_str($result->sent_time));
    row2(tra("Report deadline"), time_str($result->report_deadline));
    row2(tra("Received"), time_str($result->received_time));
    row2(tra("Server state"), result_server_state_string($result));
    row2(tra("Outcome"), result_outcome_string($result));
    row2(tra("Client state"), result_client_state_string($result));
    row2(tra("Exit status"), exit_status_string($result->exit_status));
    row2(tra("Computer ID"), host_link($result->hostid));
    row2(tra("Run time"), time_diff($result->elapsed_time));
    row2(tra("CPU time"), time_diff($result->cpu_time));
    row2('Priority', $result->priority);
    row2(tra("Validate state"), validate_state_str($result));
    row2(tra("Credit"), number_format($result->granted_credit, 2));
    row2(tra("Device peak FLOPS"), number_format($result->flops_estimate/1e9, 2)." GFLOPS");
    row2(tra("Application version"), app_version_string($result));
    if ($result->peak_working_set_size) {
        row2("Peak working set size",
            size_string($result->peak_working_set_size)
        );
    }
    if ($result->peak_swap_size) {
        row2("Peak swap size", size_string($result->peak_swap_size));
    }
    if ($result->peak_disk_usage) {
        row2("Peak disk usage", size_string($result->peak_disk_usage));
    }
    if ($show_outfile_links && $result->outcome == 1) {
        $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
        $names = get_outfile_phys_names($result);
        $i = 0;
        $x = "";
        foreach ($names as $name) {
            if ($i) $x .= " &middot; ";
            $url = dir_hier_url($name, "upload", $fanout);
            echo $name;
            $x .= " <a href=$url>$i</a> ";
            $i++;
        }
        row2(tra("Output files"), $x);
    }
    if (function_exists('project_result')) {
        project_result($result);
    }
    end_table();
    if ($result->server_state == RESULT_SERVER_STATE_OVER) {
        echo "<h3>".tra("Stderr output")."</h3> <pre>"
            .htmlspecialchars(
                $result->stderr_out,
                ENT_QUOTES | (defined('ENT_SUBSTITUTE')?ENT_SUBSTITUTE:0),
                'utf-8'
            )
            ."</pre>"
        ;
    }
}

function result_navigation($info, $where_clause) {
    global $state_name;
    $state_count = array();
    $app_count = array();
    $x = "";

    $apps = BoincApp::enum('deprecated=0 ORDER BY user_friendly_name');

    for ($i=0; $i<NSTATES; $i++) {
        $state_count[$i] = 0;
    }
    foreach ($apps as $app) {
        $app_count[$app->id] = 0;
    }
    $app_count[0] = 0;

    $results = BoincResult::enum_fields(
        "appid, server_state, outcome, validate_state, count(*) AS cnt",
        $where_clause." group by appid, server_state, outcome, validate_state", null
    );
    foreach ($results as $r) {
        $cnt = $r->cnt;
        if (array_key_exists($r->appid, $app_count)) {
            $app_count[$r->appid] += $cnt;
        }
        $app_count[0] += $cnt;
        if (!$info->appid || ($r->appid == $info->appid)) {
            $state_count[state_num($r)] += $cnt;
            $state_count[0] += $cnt;
        }
    }

    $x .= "<br><center>";
    $show_prev = ($info->offset >= $info->results_per_page);
    $show_next = ($info->number_of_results > $info->results_per_page);
    if ($show_prev) {
        $i2 = clone $info;
        $i2->offset = $info->offset - $info->results_per_page;
        $url = result_page_url($i2);
        $x .= "<a href=$url>".tra("Previous")." ".$info->results_per_page."</a>";
    }
    if ($show_prev && $show_next) {
        $x .= "&nbsp;&middot;&nbsp;";
    }
    if ($show_next) {
        $i2 = clone $info;
        $i2->offset = $info->offset + $info->results_per_page;
        $url = result_page_url($i2);
        $x .= "<a href=$url>".tra("Next")." ".$info->results_per_page."</a>";
    }
    $x .= "<br>".tra("State").": ";
    for ($i=0; $i<NSTATES; $i++) {
        if ($i) $x .= " &middot; ";
        if ($info->state == $i) {
            $x .= $state_name[$i];
        } else {
            $i2 = clone $info;
            $i2->state = $i;
            $i2->offset = 0;
            $url = result_page_url($i2);
            $x .= "<a href=$url>".$state_name[$i]."</a>";
        }
        $x .= " (".$state_count[$i].") ";
    }

    if (count($apps) > 1) {
        $i2 = clone $info;
        $i2->offset = 0;
        $x .= "<br>".tra("Application").": ";
        if ($info->appid) {
            $i2->appid = 0;
            $url = result_page_url($i2);
            $x .= '<a href="'.$url.'">All</a>';
        } else {
            $x .= 'All';
        }
        $x .= " (".$app_count[0].") ";

        foreach ($apps as $app) {
            $i2->appid = $app->id;
            $url = result_page_url($i2);
            $x .= ' &middot; ';
            if ($info->appid == $app->id) {
                $x .= $app->user_friendly_name;
            } else {
                $x .= '<a href="'.$url.'">'.$app->user_friendly_name.'</a>';
            }
            $x .= " (".$app_count[$app->id].") ";
        }
    }

    $x .= '<form action="result.php">'.tra("Task name:").' <input "size=40" name="result_name"> <input class="btn btn-primary" type="submit" value="OK"></form>';
    $x .= "</center><br>";
    return $x;
}


?>
