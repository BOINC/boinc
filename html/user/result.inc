<?php

function result_server_state_string($s) {
    switch($s) {
    case 1: return "Inactive";
    case 2: return "Unsent";
    case 4: return "In Progress";
    case 5: return "Over";
    }
    return "unknown";
}

function result_outcome_string($s) {
    switch($s) {
    case 1: return "Success";
    case 2: return "Couldn't send";
    case 3: return "Client error";
    case 4: return "No reply";
    case 5: return "Didn't need";
    }
    return "unknown";
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
    }
    return "unknown";
}

function result_table_start() {
    start_table();
    echo "
        <tr>
        <th>Sent</th>
        <th>Received</th>
        <th>Server state</th>
        <th>Outcome</th>
        <th>Client state</th>
        <th>CPU time</th>
        <th>claimed credit</th>
        <th>granted credit</th>
        </tr>
    ";
}

function show_result_row($result) {
    $s = time_str($result->sent_time);
    $r = time_str($result->received_time);
    $ss = result_server_state_string($result->server_state);
    $out = result_outcome_string($result->outcome);
    $cl = result_client_state_string($result->client_state);
    echo "
        <tr>
        <td>$s</td>
        <td>$r</td>
        <td>$ss</td>
        <td>$out</td>
        <td>$cl</td>
        <td>$result->cpu_time</td>
        <td>$result->claimed_credit</td>
        <td>$result->granted_credit</td>
        </tr>
    ";
}

?>
