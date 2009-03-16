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
            return "pending";
        }
        return $string_to_show;
    default:
        if ($result->granted_credit > 0) {
            return $string_to_show;
	    }
        return "---"; // no claimed credit 
    }
} 

function state_string($result) {
    switch ($result->server_state) {
    case 1: return "Inactive";
    case 2: return "Unsent";
    case 4: return "In progress";
    case 5:
        switch ($result->outcome) {
        case 1:
            switch ($result->validate_state) {
            case 0: return "Completed, waiting for validation";
            case 1: return "Completed and validated";
            case 2: return "Completed, marked as invalid";
            case 3: return "Completed, can't validate";
            case 4: return "Completed, validation inconclusive";
            case 4: return "Completed, too late to validate";
            }
            return "Completed";
        case 2: return "Couldn't send";
        case 3:
            if ($result->exit_status == -221) {
                return "Redundant result";
            }
            switch($result->client_state) {
            case 1: return "Error while downloading";
            case 2: return "Error while computing";
            case 3: return "Error while computing";
            case 4: return "Error while uploading";
            }
            return "Error";
        case 4: return "Timed out - no response";
        case 5: return "Didn't need";
        case 6: return "Validate error";
        case 7: return "Client detached";
        }
    }
    return "Unknown";
}

function result_server_state_string($result) {
    switch($result->server_state) {
    case 1: return "Inactive";
    case 2: return "Unsent";
    case 4: return "In progress";
    case 5: return "Over";
    }
    return "Unknown";
}

function result_outcome_string($result) {
    switch($result->outcome) {
    case 0: return "---";
    case 1: return "Success";
    case 2: return "Couldn't send";
    case 3:
        if ($result->exit_status <> -221) {
            return "Client error";
        }
        return "Redundant result";
    case 4: return "No reply";
    case 5: return "Didn't need";
    case 6: return "Validate error";
    case 7: return "Client detached";
    }
    return "Unknown";
}

function result_client_state_string($result) {
    switch($result->client_state) {
    case 0: return "New";
    case 1: return "Downloading";
    case 2: return "Computing";
    case 3: return "Compute error";
    case 4: return "Uploading";
    case 5: return "Done";
    case 6:
        if ($result->exit_status == -221) {
            return "Cancelled by server";
        }
        return "Aborted by user";
    }
}

function validate_state_str($result) {
    switch($result->validate_state) {
    case 0: return "Initial";
    case 1: return "Valid";
    case 2:
        if ($result->exit_status <> -221) {
            return "Invalid";
        }
        return "Not necessary";
    case 3: return "Workunit error - check skipped";
    case 4: return "Checked, but no consensus yet";
    case 5: return "Task was reported too late to validate";
    }
    return "Unknown";
}

function wu_error_mask_str($s) {
    $x = "";
    if ($s & 1) {
        $x = $x."Couldn't send result ";
        $s -= 1;
    }
    if ($s & 2) {
        $x = $x."Too many error results ";
        $s -= 2;
    }
    if ($s & 4) {
        $x = $x."Too many success results ";
        $s -= 4;
    }
    if ($s & 8) {
        $x = $x."Too many total results ";
        $s -= 8;
    }
    if ($s & 16) {
        $x = $x."Cancelled ";
        $s -= 16;
    }
    if ($s) {
        $x = $x."Unrecognized Error: $s ";
    }
    if (!strlen($x)) {
        $x="<br>";
    }
    return $x;
}

function result_page_url($info) {
    $c = $info->clause;
    $o = $info->offset;
    $sn = $info->show_names;
    return "results.php?$c&amp;offset=$o&amp;show_names=$sn";
}

function result_table_start($show_wu_link, $show_host_link, $info) {
    start_table();
    echo "<tr>";
    if ($info) {
        if ($info->show_names) {
            $i2 = clone $info;
            $i2->show_names = 0;
            $url = result_page_url($i2);
            echo "<th>Task name<br><span class=\"smalltext\">click for details<br><a href=$url>Show IDs</a></span></th>\n";
        } else {
            $i2 = clone $info;
            $i2->show_names = 1;
            $url = result_page_url($i2);
            echo "<th>Task ID<br><span class=\"smalltext\">click for details<br><a href=$url>Show names</a></span></th>\n";
        }
    } else {
        echo "<th>Task ID<br><span class=\"smalltext\">click for details</span></th>\n";
    }
    if ($show_wu_link) {
        echo "<th>Work unit ID<br><span class=\"smalltext\">click for details</span></th>\n";
    }
    if ($show_host_link) {
        echo "<th>Computer</th>\n";
    }
    echo "
        <th>Sent</th>
        <th>Time reported
            <br>or deadline
            <br><span class=\"smalltext\"><a href=\"explain_state.php?field=result_time\">explain</a></span>
        </th>
        <th>Status</th>
        <th>CPU time (sec)</th>
        <th>claimed credit</th>
        <th>granted credit</th>
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
    } else {
        $cpu_time = number_format($result->cpu_time, 2);
    }
    echo "
        <td>$s</td>
        <td>$r</td>
        <td>$ss</td>
        <td align=right>$cpu_time</td>
        <td align=right>$result_claimed_credit</td>
        <td align=right>$result_granted_credit</td>
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
    row2("Name", $result->name);
    row2("Workunit", "<a href=\"workunit.php?wuid=$result->workunitid\">$result->workunitid</a>");
    row2("Created", time_str($result->create_time));
    row2("Sent", time_str($result->sent_time));
    row2("Received", time_str($result->received_time));
    row2("Server state", result_server_state_string($result));
    row2("Outcome", result_outcome_string($result));
    row2("Client state", result_client_state_string($result));
    row2("Exit status", exit_status_string($result));
    row2("Computer ID", host_link($result->hostid));
    row2("Report deadline", time_str($result->report_deadline));
    row2("CPU time", $result->cpu_time);
    row2("stderr out", "<pre>".htmlspecialchars($result->stderr_out)."</pre>");
    row2("Validate state", validate_state_str($result));
    row2("Claimed credit", $result->claimed_credit);
    row2("Granted credit", $result->granted_credit);
    row2("application version", version_string($result->app_version_num));
    end_table();
}

function show_result_navigation($info) {
    echo "<br><center>";
    $show_prev = ($info->offset >= $info->results_per_page);
    $show_next = ($info->number_of_results > $info->results_per_page);
    if ($show_prev) {
        $i2 = clone $info;
        $i2->offset = $info->offset - $info->results_per_page;
        $url = result_page_url($i2);
        echo "<a href=$url>Previous ".$info->results_per_page."</a>";
    }
    if ($show_prev && $show_next) {
        echo "&nbsp;|&nbsp;";
    }
    if ($show_next) {
        $i2 = clone $info;
        $i2->offset = $info->offset + $info->results_per_page;
        $url = result_page_url($i2);
        echo "<a href=$url>Next ".$info->results_per_page."</a>";
    }
    echo "</center><br>";
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
