<?php

function show_user_stats($user) {
    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>User Stats:</b></font></td></tr>\n";
    row("<b>User ID:   </b>", $user->id);
    $row = sprintf("<b>%s user since:   </b>", $project);
    row($row, time_str($user->create_time));
    row("<b>Total credit:   </b>", $user->total_credit);
    row("<b>Recent averaged credit:   </b>", $user->expavg_credit);
    if ($user->teamid) {
        $result = mysql_query("select * from team where id = $user->teamid");
        $team = mysql_fetch_object($result);
        row("<b>Team:   </b>", "<a href=team_display.php?id=$team->id>$team->name</a>");
    } else {
        row("<b>Team:   </b>", "None");
    }
    echo "</table>\n";
}

function show_user_profile($user) {
    echo TABLE2."\n";
    echo "<tr>".TD2.LG_FONT."<b>User Information:</b></font></td></tr>\n";
    row("<b>User name:   </b>", $user->name);
    row("<b>Email address:   <b>", $user->email_addr);
    row("<b>Country:   </b>", $user->country);
    row("<b>Postal (ZIP) code:   </b>", $user->postal_code);
    echo "<tr><td align=right><a href=edit_user_info.php>Edit User Information</a></td>\n";
    echo "<td align=center><a href=change_password.php>Change Password</a></td></tr>\n";
    echo "</table>\n";
}

function show_hosts($user) {
    $result = mysql_query("select * from host where userid=$user->id");
    while ($host = mysql_fetch_object($result)) {
        echo "<p>\n";
        show_host($host);
    }
    mysql_free_result($result);
}

// show a summary of the user.
// NOTE: This is intended to be shown only to that user.
// it has info that other users aren't supposed to see

function show_user_page_private($user) {
    echo "<h1>$user->name</h1>\n";
    start_table();
    show_user_profile($user);
    echo "<p><p>\n";
    show_user_stats($user);
    echo "<p>\n";
    echo "<ul>";
    echo "<li><a href=show_hosts.php>Host info</a></li>";
    echo "<li><a href=prefs.php>Preferences</a></li>";
    echo "<li><a href=team.php>Teams</a></li>";
    echo "</ul>";

}

function user_table_start() {
    echo TABLE."
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
