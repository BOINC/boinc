<?php

function show_user_stats($user) {
    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>User stats</b></font></td></tr>\n";
    $row = sprintf("<b>%s user since</b>", $project);
    row($row, time_str($user->create_time));
    row("<b>Total credit</b>", $user->total_credit);
    row("<b>Recent average credit</b>", $user->expavg_credit);
    if ($user->teamid) {
        $result = mysql_query("select * from team where id = $user->teamid");
        $team = mysql_fetch_object($result);
        row("<b>Team</b>", "<a href=team_display.php?id=$team->id>$team->name</a>");
    } else {
        row("<b>Team:   </b>", "None");
    }
    echo "</table>\n";
}

function edit_link() {
    return "<br><font size=-1><a href=edit_user_info_form.php>edit</a></font>";
}
function edit_email_link() {
    return "<br><font size=-1><a href=edit_email_form.php>edit</a></font>";
}

function show_user_profile($user) {
    if (is_valid_email_addr($user->email_addr)) {
        $email_text = $user->email_addr;
    } else {
        $email_text = "Verification pending";
    }
    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>User information</b></font></td></tr>\n";
    row("<b>name</b>", $user->name.edit_link());
    row("<b>email address</b>", $email_text.edit_email_link());
    row("<b>country</b>", $user->country.edit_link());
    row("<b>postal (ZIP) code</b>", $user->postal_code.edit_link());
    echo "</table>\n";
}

function show_hosts($user) {
    $result = mysql_query("select * from host where userid=$user->id order by rpc_time desc");
    echo VISTABLE."\n";
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
    echo "<table cellpadding=4>\n";
    show_user_profile($user);
    show_user_stats($user);
    echo "</table>\n";
    echo "<ul>";
    echo "<li><a href=show_hosts.php>Your computer(s)</a></li>";
    echo "<li><a href=prefs.php>Your preferences</a></li>";
    echo "</ul>";

}

function user_table_start() {
    echo VISTABLE."
        <tr><th>Name</th><th>Average credit</th><th>Total credit</th>
        <th>Participant since</th></tr>";
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
