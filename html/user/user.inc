<?php

function show_user_stats($user) {
    row1("Account statistics");
    $row = sprintf("%s User since", $project);
    row2($row, time_str($user->create_time));
    row2("Total credit", $user->total_credit);
    row2("Recent average credit", $user->expavg_credit);
    if ($user->teamid) {
        $result = mysql_query("select * from team where id = $user->teamid");
        $team = mysql_fetch_object($result);
        row2("Team", "<a href=team_display.php?id=$team->id>$team->name</a>");
    } else {
        row2("Team", "None");
    }
}

function show_user_profile($user) {
    if (is_valid_email_addr($user->email_addr)) {
        $email_text = $user->email_addr;
    } else {
        $email_text = "Verification pending";
    }
    row1("Account information");
    row2("Name", $user->name);
    row2("Email address", $email_text);
    row2("Country", $user->country);
    row2("Postal code", $user->postal_code);
    row2("", "<a href=edit_email_form.php>Edit email address</a>");
    row2("", "<a href=edit_user_info_form.php>Edit other info</a>");
}

function show_hosts($user) {
    $result = mysql_query("select * from host where userid=$user->id order by rpc_time desc");
    start_table();
    echo "<tr><td>Host name</td><td>Total Credit</td><td>Recent credit</td></tr>";
    while ($host = mysql_fetch_object($result)) {
        echo "<p>\n";
        show_host_brief($host);
    }
    echo "</table>\n";
    mysql_free_result($result);
}

function show_host_brief($host) {
    echo "<tr>
        <td><a href=show_host_detail.php?hostid=$host->id>$host->domain_name</a></td>
        <td>$host->total_credit</td>
        <td>$host->expavg_credit</td>
        </tr>
    ";
}

// show a summary of the user.
// NOTE: This is intended to be shown only to that user.
// it has info that other users aren't supposed to see

function show_user_page_private($user) {
    start_table();
    show_user_profile($user);
    show_user_stats($user);
    end_table();
    echo "<ul>";
    echo "<li><a href=show_hosts.php>Your computer(s)</a></li>";
    echo "<li><a href=prefs.php?subset=global>General preferences</a></li>";
    echo "<li><a href=prefs.php?subset=project>".PROJECT." preferences</a></li>";
    echo "</ul>";

}

function user_table_start() {
    start_table();
    echo "
        <tr><th>Name</th><th>Average credit</th><th>Total credit</th>
        <th>Participant since</th></tr>
    ";
}

function show_user_row($user) {
    echo "<tr>
        <td>$user->name</td>
        <td>$user->expavg_credit</td>
        <td>$user->total_credit</td>
        <td>".time_str($user->create_time)."</td>
    <tr>\n";
}

?>
