<?php

require_once("util.inc");
require_once("db.inc");
require_once("sanitize_html.inc");

function show_team($team) {
    start_table();
    row2("ID", $team->id);
    row2("founder", $team->userid);
    row2("name", $team->name);
    row2("url", $team->url);
    row2("type", $team->type);
    row2("name_html", $team->name_html);
    row2("description", sanitize_html($team->description));
}

function display_team_page($team) {
    page_head("$team->name");
    if ($team->name_html != null) {
        echo "<p>";
        echo "$team->name_html";
    }
    echo "<p>";
    echo "[<a href=team_join_form.php?id=$team->id><b>Join</b></a>]  ";
    echo "[<a href=team_quit_form.php?id=$team->id><b>Quit</b></a>]  ";
    echo "[<a href=team_edit_form.php?id=$team->id><b>Edit*</b></a>]  ";
    echo "[<a href=team_remove_inactive_form.php?id=$team->id><b>Remove Inactive Members*</b></a>]  ";
    echo "[<a href=team_disband_form.php?id=$team->id><b>Disband Team*</b></a>]  ";
    echo "[<a href=team_email_list.php?id=$team->id><b>View Team Emails*</b></a>]";
    echo "<br><font size=2>* Team founder only</font>";

    echo "<br>";
    echo "<p>";
    echo "<table border=0 width=580>";
    echo "<tr bgcolor=#708090><td colspan=2><font size=+1>";
    echo "<b>Team Info:</b></font></td></tr></table>";
    echo "<table>";
    if (strlen($team->description)) {
        row2("Description", sanitize_html($team->description));
    }
    if (strlen($team->url)) {;
        row2("Web site", "<a href=http://$team->url>http://$team->url</a>");
    }
    row2("Members", $team->nusers);

    $query = sprintf(
        "select * from user where teamid = %d",
        $team->id
    );
    $result = mysql_query($query);
    $total_credit = 0;
    for ($i = 0; $i < $team->nusers; $i++) {
        $user = mysql_fetch_object($result);
        $total_credit = $total_credit + $user->total_credit;
    }

    row2("Total Credit", $total_credit);
    $query = sprintf(
        "select * from user where id = %d",
        $team->userid
    );
    $result = mysql_query($query);
    $user = mysql_fetch_object($result);
    row2("Founder", $user->name);
    echo "</table>";
    echo "<p>";
    echo "<table border=0 width=580>";
    echo "<tr bgcolor=#708090><td colspan=2><font size=+1>";
    echo "<b>Team Members:</b></font></td></tr></table>";
    echo "<table border=1 cellpadding=2 width=40%><tr>";
    echo "<th>Name</th>";
    echo "<th>Total credit</th>";
    echo "<th>Recent average CPU<br>time per work unit</th>";
    echo "<th>Country</th>";
    echo "</tr>";

    $query = sprintf(
        "select * from user where teamid = %d order by expavg_credit desc",
        $team->id
    );
    $result = mysql_query($query);

    for ($i = 0; $i < $team->nusers; $i++) {
        $user = mysql_fetch_object($result);
        $j = $i+1;
        echo "<tr><td align=left>$j) $user->name";
        echo "<td align=center>$user->total_credit</td>";
        echo "<td align=center>$user->expavg_credit</td>";
        echo "<td align=center>$user->country</td>";
        echo "</tr>";
    }
    echo "</table>";
    page_tail();
}

// requires that the team exist
function require_team($team) {
    if (!$team) {
        page_head("Error");
        echo "Team does not exist.";
        page_tail();
        exit();
    }
}

// requires that the user is logged in as the founder of
// the team trying to be edited
function require_founder_login($user, $team) {
    require_login($user);
    require_team($team);
    if ($user->id != $team->userid) {
        page_head("Permission denied");
        echo "Only a team's founder may edit a team.";
        page_tail();
        exit();
    }
}

?>
