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

$apps = array();
$app_versions = array();

function app_version_string($id) {
    global $apps, $app_versions;
    if ($id == 0) return "";
    if ($id == -1) return tra("Anonymous platform");
    if ($id == -2) return tra("Anonymous platform - CPU");
    if ($id == -3) return tra("Anonymous platform - NVIDIA GPU");
    if ($id == -4) return tra("Anonymous platform - ATI GPU");
    if (array_key_exists($id, $app_versions)) {
        $av = $app_versions[$id];
        $app = $apps[$av->appid];
    } else {
        $av = BoincAppVersion::lookup_id($id);
        if (!$av) {
            return tra("Not in DB");
        }
        $app_versions[$id] = $av;
        $app = BoincApp::lookup_id($av->appid);
        $apps[$av->appid] = $app;
    }
    $n = $app->user_friendly_name;
    $v = sprintf("%d.%02d", $av->version_num/100, $av->version_num%100);
    if ($av->plan_class) {
        $c = "($av->plan_class)";
    } else {
        $c = "";
    }
    return "$n v$v $c";
}

function result_claimed_credit_string($result, $string_to_show) {
    if ($result->server_state <> 5) return "---";
    switch($result->outcome) {
    case 3:
        if ($result->exit_status <> -221) {
                return $string_to_show; //Client error
        } 
    default:
        if ($result->claimed_credit > 0) {
            return $string_to_show;
	    }
        return "---"; // no claimed credit 
    }
}

function result_granted_credit_string($result, $string_to_show) {
    if ($result->server_state <> 5) return "---";
    switch($result->outcome) {
    case 1:              //Success
        switch ($result->validate_state) {
        case 0:
        case 4:
            return tra("pending");
        }
        return $string_to_show;
    default:
        if ($result->granted_credit > 0) {
            return $string_to_show;
	    }
        return "---"; // no claimed credit 
    }
} 

// various states that we allow users to filter on

define("STATE_ALL", 0);
define("STATE_IN_PROGRESS", 1);
define("STATE_PENDING", 2);
define("STATE_VALID", 3);
define("STATE_INVALID", 4);
define("STATE_ERROR", 5);
define("NSTATES", 6);

$state_name = array(
    tra("All"),
    tra("In progress"),
    tra("Pending"),
    tra("Valid"),
    tra("Invalid"),
    tra("Error"),
);

$state_clause = array(
    "",
    " and server_state=4 ",
    " and server_state=5 and outcome=1 and (validate_state=0 or validate_state=4) ",
    " and server_state=5 and outcome=1 and validate_state=1 ",
    " and server_state=5 and (outcome=6 or (outcome=1 and (validate_state=2 or validate_state=3 or validate_state=5))) ",
    " and server_state=5 and (outcome=3 or outcome=4 or outcome=7) ",
);

function state_string($result) {
    switch ($result->server_state) {
    case 1: return tra("Inactive");
    case 2: return tra("Unsent");
    case 4: return tra("In progress");
    case 5:
        switch ($result->outcome) {
        case 1:
            switch ($result->validate_state) {
            case 0: return tra("Completed, waiting for validation");
            case 1: return tra("Completed and validated");
            case 2: return tra("Completed, marked as invalid");
            case 3: return tra("Completed, can't validate");
            case 4: return tra("Completed, validation inconclusive");
            case 5: return tra("Completed, too late to validate");
            }
            return tra("Completed");
        case 2: return tra("Couldn't send");
        case 3:
            if ($result->exit_status == -221) {
                return tra("Cancelled by server");
            }
            switch($result->client_state) {
            case 1: return tra("Error while downloading");
            case 2:
            case 3: return tra("Error while computing");
            case 4: return tra("Error while uploading");
            case 6: return tra("Aborted by user");
            }
            return tra("Error");
        case 4: return tra("Timed out - no response");
        case 5: return tra("Didn't need");
        case 6: return tra("Validate error");
        case 7: return tra("Client detached");
        }
    }
    return tra("Unknown");
}

function result_server_state_string($result) {
    switch($result->server_state) {
    case 1: return tra("Inactive");
    case 2: return tra("Unsent");
    case 4: return tra("In progress");
    case 5: return tra("Over");
    }
    return tra("Unknown");
}

function result_outcome_string($result) {
    switch($result->outcome) {
    case 0: return "---";
    case 1: return tra("Success");
    case 2: return tra("Couldn't send");
    case 3:
        if ($result->exit_status <> -221) {
            return tra("Client error");
        }
        return tra("Redundant result");
    case 4: return tra("No reply");
    case 5: return tra("Didn't need");
    case 6: return tra("Validate error");
    case 7: return tra("Client detached");
    }
    return tra("Unknown");
}

function result_client_state_string($result) {
    switch($result->client_state) {
    case 0: return tra("New");
    case 1: return tra("Downloading");
    case 2: return tra("Processing");
    case 3: return tra("Compute error");
    case 4: return tra("Uploading");
    case 5: return tra("Done");
    case 6:
        if ($result->exit_status == -221) {
            return tra("Cancelled by server");
        }
        return tra("Aborted by user");
    }
}

function validate_state_str($result) {
    switch($result->validate_state) {
    case 0: return tra("Initial");
    case 1: return tra("Valid");
    case 2:
        if ($result->exit_status <> -221) {
            return tra("Invalid");
        }
        return tra("Not necessary");
    case 3: return tra("Workunit error - check skipped");
    case 4: return tra("Checked, but no consensus yet");
    case 5: return tra("Task was reported too late to validate");
    }
    return tra("Unknown");
}

function wu_error_mask_str($s) {
    $x = "";
    if ($s & 1) {
        $x = $x." ".tra("Couldn't send result");
        $s -= 1;
    }
    if ($s & 2) {
        $x = $x." ".tra("Too many errors (may have bug)");
        $s -= 2;
    }
    if ($s & 4) {
        $x = $x." ".tra("Too many results (may be nondeterministic)");
        $s -= 4;
    }
    if ($s & 8) {
        $x = $x." ".tra("Too many total results");
        $s -= 8;
    }
    if ($s & 16) {
        $x = $x." ".tra("WU cancelled");
        $s -= 16;
    }
    if ($s) {
        $x = $x." ".tra("Unrecognized Error: %1", $s);
    }
    if (strlen($x)) {
        $x="<font color=\"#ff3333\">".$x."</font>";
    } else {
        $x="";
    }
    return $x;
}

function result_page_url($info) {
    $c = $info->clause;
    $o = $info->offset;
    $sn = $info->show_names;
    $st = $info->state;
    return "results.php?$c&amp;offset=$o&amp;show_names=$sn&amp;state=$st";
}

function result_table_start($show_wu_link, $show_host_link, $info) {
    start_table();
    echo "<tr>";
    if ($info) {
        if ($info->show_names) {
            $i2 = clone $info;
            $i2->show_names = 0;
            $url = result_page_url($i2);
            echo "<th>".tra("Task name")."<br><span class=\"smalltext\">".tra("click for details")."<br><a href=$url>".tra("Show IDs")."</a></span></th>\n";
        } else {
            $i2 = clone $info;
            $i2->show_names = 1;
            $url = result_page_url($i2);
            echo "<th>Task<br><span class=\"smalltext\">".tra("click for details")."<br><a href=$url>".tra("Show names")."</a></span></th>\n";
        }
    } else {
        echo "<th>".tra("Task")."<br><span class=\"smalltext\">".tra("click for details")."</span></th>\n";
    }
    if ($show_wu_link) {
        echo "<th>".tra("Work unit")."<br><span class=\"smalltext\">".tra("click for details")."</span></th>\n";
    }
    if ($show_host_link) {
        echo "<th>".tra("Computer")."</th>\n";
    }
    echo "
        <th>".tra("Sent")."</th>
        <th>".tra("Time reported<br />or deadline")."
            <br><span class=\"smalltext\"><a href=\"explain_state.php?field=result_time\">".tra("explain")."</a></span>
        </th>
        <th>".tra("Status")."</th>
        <th>".tra("Run time<br />(sec)")."</th>
        <th>".tra("CPU time<br />(sec)")."</th>
        <th>".tra("Claimed credit")."</th>
        <th>".tra("Granted credit")."</th>
        <th>".tra("Application")."</th>
        </tr>
    ";
}

// was result invalid or timed out?
//
function bad_result($result) {
    if ($result->validate_state == 2) return true;
    if (!$result->received_time && ($result->report_deadline < time())) return true;
    return false;
}

function show_result_row(
    $result, $show_wu_link, $show_host_link, $show_name, $i
) {
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
    $result_claimed_credit = format_credit($result->claimed_credit);
    $result_granted_credit = format_credit($result->granted_credit);
    $result_claimed_credit = result_claimed_credit_string($result, $result_claimed_credit);
    $result_granted_credit = result_granted_credit_string($result, $result_granted_credit); 
    $j = $i % 2;
    echo "<tr class=row$j>";
    if ($show_name) {
        $x = $result->name;
    } else {
        $x = $result->id;
    }
    echo "<td><a href=\"result.php?resultid=$result->id\">$x</a></td>\n";
    if ($show_wu_link) {
        echo "<td><a href=\"workunit.php?wuid=$result->workunitid\">$result->workunitid</a></td>\n";
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
    $v = app_version_string($result->app_version_id);
    echo "
        <td>$s</td>
        <td>$r</td>
        <td>$ss</td>
        <td align=right>$elapsed_time</td>
        <td align=right>$cpu_time</td>
        <td align=right>$result_claimed_credit</td>
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
function exit_status_string($result) {
    $x = $result->exit_status;
    if ($x == 0) {
        $y = parse_element($result->stderr_out, "<exit_status>");
        if ($y) {
            $x = (int)$y;
        }
    }
    return sprintf("%d (0x%x)", $x, $x);
}

function show_result($result) {
    start_table();
    row2(tra("Name"), $result->name);
    row2(tra("Workunit"), "<a href=\"workunit.php?wuid=$result->workunitid\">$result->workunitid</a>");
    row2(tra("Created"), time_str($result->create_time));
    row2(tra("Sent"), time_str($result->sent_time));
    row2(tra("Received"), time_str($result->received_time));
    row2(tra("Server state"), result_server_state_string($result));
    row2(tra("Outcome"), result_outcome_string($result));
    row2(tra("Client state"), result_client_state_string($result));
    row2(tra("Exit status"), exit_status_string($result));
    row2(tra("Computer ID"), host_link($result->hostid));
    row2(tra("Report deadline"), time_str($result->report_deadline));
    row2(tra("Run time"), number_format($result->elapsed_time, 2));
    row2(tra("CPU time"), number_format($result->cpu_time, 2));
    row2(tra("Validate state"), validate_state_str($result));
    row2(tra("Claimed credit"), number_format($result->claimed_credit, 2));
    row2(tra("Granted credit"), number_format($result->granted_credit, 2));
    row2(tra("Application version"), app_version_string($result->app_version_id));
    end_table();
    echo "<h3>Stderr output</h3> <pre>".htmlspecialchars($result->stderr_out)."</pre>";
}

function show_result_navigation($info) {
    global $state_name;

    echo "<br><center>";
    $show_prev = ($info->offset >= $info->results_per_page);
    $show_next = ($info->number_of_results > $info->results_per_page);
    if ($show_prev) {
        $i2 = clone $info;
        $i2->offset = $info->offset - $info->results_per_page;
        $url = result_page_url($i2);
        echo "<a href=$url>".tra("Previous")." ".$info->results_per_page."</a>";
    }
    if ($show_prev && $show_next) {
        echo "&nbsp;|&nbsp;";
    }
    if ($show_next) {
        $i2 = clone $info;
        $i2->offset = $info->offset + $info->results_per_page;
        $url = result_page_url($i2);
        echo "<a href=$url>".tra("Next")." ".$info->results_per_page."</a>";
    }
    echo "<br>Show: ";
    for ($i=0; $i<NSTATES; $i++) {
        if ($i) echo " | ";
        if ($info->state == $i) {
            echo $state_name[$i];
        } else {
            $i2 = clone $info;
            $i2->state = $i;
            $i2->offset = 0;
            $url = result_page_url($i2);
            echo "<a href=$url>".$state_name[$i]."</a>";
        }
    }
    echo "</center><br>";
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
