<?php

function result_claimed_credit_string($result, $string_to_show) {
    if ($result->server_state <> 5) return "---";
    switch($result->outcome) {
    case 1: return $string_to_show; //Success
    case 2: return "---"; //Couldn't send
    case 3: return $string_to_show; //Client error
    case 4: return "---"; //No reply
    case 5: return "---"; //Didn't need
    case 6: return "---"; // Validate error
    default: return $string_to_show;
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
    case 2: return "---"; //Couldn't send
    case 3: return "---"; //Client error
    case 4: return "---"; //No reply
    case 5: return "---"; //Didn't need
    case 6: return "---"; //Validate error
    default: return $string_to_show;
    }
} 

function result_server_state_string($s) {
    switch($s) {
    case 1: return "Inactive";
    case 2: return "Unsent";
    case 3: return "Unsent";
    case 4: return "In Progress";
    case 5: return "Over";
    }
    return "Unknown";
}

function result_outcome_string($s) {
    switch($s) {
    case 1: return "Success";
    case 2: return "Couldn't send";
    case 3: return "Client error";
    case 4: return "No reply";
    case 5: return "Didn't need";
    case 6: return "Validate error";
    }
    return "Unknown";
}

function result_client_state_string($s) {
    switch($s) {
    case 0: return "New";
    case 1: return "Downloading";
    case 2: return "Downloaded";
    case 3: return "Computing";
    case 4: return "Uploading";
    case 5: return "Done";
    }
}

function validate_state_str($s) {
    switch($s) {
    case 0: return "Initial";
    case 1: return "Valid";
    case 2: return "Invalid";
    case 3: return "WU error - check skipped";
    case 4: return "Checked, but no consensus yet";
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
    if (strlen($x)) {
    } else {
        $x="<br>";
    }
    return $x;
}

function result_table_start($show_wu_link, $show_host_link, $show_result_link) {
    start_table();
    echo "
        <tr>
    ";
    if ($show_result_link) {
        echo "<th>Result ID<br><font size=-2>click for details</a></th>\n";
    }
    if ($show_wu_link) {
        echo "<th>Work unit ID<br><font size=-2>click for details</font></th>\n";
    }
    if ($show_host_link) {
        echo "<th>Computer</th>\n";
    }
    echo "
        <th>Sent</th>
        <th>Time reported
            <br>or deadline
	    <br><font size=-2><a href=\"explain_state.php?field=result_time\">explain</a>
        </th>
        <th>Server state
            <br><font size=-2><a href=\"explain_state.php?field=result_server_state\">explain</a>
        </th>
        <th>Outcome
            <br><font size=-2><a href=\"explain_state.php?field=result_outcome\">explain</a>
        </th>
        <th>Client state
            <br><font size=-2><a href=\"explain_state.php?field=result_client_state\">explain</a>
        </th>
        <th>CPU time (sec)</th>
        <th>claimed credit</th>
        <th>granted credit</th>
        </tr>
    ";
}

function show_result_row(
    $result, $show_wu_link, $show_host_link, $show_result_link
) {
    $s = time_str($result->sent_time);
    // if result has been received, show the received time,
    // else show the reporting deadline in green if in the future
    // and in red if in the past.
    //
    if ($result->received_time) {
        $r = time_str($result->received_time);
    } else if ($result->report_deadline) {
        if ($result->report_deadline>time()) {
            $r = "<font color='33cc33'>" . time_str($result->report_deadline) . "</font>";
        } else {
            $r = "<font color='ff3333'>" . time_str($result->report_deadline) . "</font>";
        }
    } else {
        $r = "---";
    }
    $ss = result_server_state_string($result->server_state);
    $out = result_outcome_string($result->outcome);
    $cl = result_client_state_string($result->client_state);
    $result_claimed_credit = format_credit($result->claimed_credit);
    $result_granted_credit = format_credit($result->granted_credit);
    $result_claimed_credit = result_claimed_credit_string($result, $result_claimed_credit);
    $result_granted_credit = result_granted_credit_string($result, $result_granted_credit); 
    echo "
        <tr>
    ";
    if ($show_result_link) {
        echo "<td><a href=\"result.php?resultid=$result->id\">$result->id</a></td>\n";
    }
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
        <td>$out</td>
        <td>$cl</td>
        <td>$cpu_time</td>
        <td>$result_claimed_credit</td>
        <td>$result_granted_credit</td>
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

function server_state_string($s) {
    switch($s) {
    case 1: return "Inactive";
    case 2: return "Unsent";
    case 4: return "In Progress";
    case 5: return "Over";
    }
    return "Unknown";
}

function outcome_string($s) {
    switch($s) {
    case 1: return "Success";
    case 2: return "Couldn't send";
    case 3: return "Client error";
    case 4: return "No reply";
    case 5: return "Didn't need";
    }
    return "Unknown";
}

function client_state_string($s) {
    switch($s) {
    case 0: return "New";
    case 1: return "Downloading";
    case 2: return "Downloaded";
    case 3: return "Computing";
    case 4: return "Uploading";
    case 5: return "Done";
    }
    return "Unknown";
}

function show_result($result) {
    start_table();
    row2("Result ID", $result->id);
    row2("Name", $result->name);
    row2("Workunit", "<a href=\"workunit.php?wuid=$result->workunitid\">$result->workunitid</a>");
    row2("Created", time_str($result->create_time));
    row2("Sent", time_str($result->sent_time));
    row2("Received", time_str($result->received_time));
    row2("Server state", server_state_string($result->server_state));
    row2("Outcome", outcome_string($result->outcome));
    row2("Client state", client_state_string($result->client_state));
    row2("Exit status", exit_status_string($result));
    row2("Computer ID", host_link($result->hostid));
    row2("Report deadline", time_str($result->report_deadline));
    row2("CPU time", $result->cpu_time);
    row2("stderr out", "<pre>".htmlspecialchars($result->stderr_out)."</pre>");
    row2("Validate state", validate_state_str($result->validate_state));
    row2("Claimed credit", $result->claimed_credit);
    row2("Granted credit", $result->granted_credit);
    row2("application version", version_string($result->app_version_num));
    end_table();
}

?>
