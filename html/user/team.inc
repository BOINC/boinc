<?php

require_once("util.inc");
require_once("db.inc");
require_once("sanitize_html.inc");

function display_team_page($team) {
    page_head("$team->name");
    if ($team->name_html != null) {
        echo "<p>";
        echo "$team->name_html";
    }
    echo "<p>
        [<a href=team_join_form.php?id=$team->id><b>Join</b></a>] 
        [<a href=team_quit_form.php?id=$team->id><b>Quit</b></a>]
        [<a href=team_edit_form.php?id=$team->id><b>Edit*</b></a>]
        [<a href=team_remove_inactive_form.php?id=$team->id><b>Remove Inactive Members*</b></a>]
        [<a href=team_disband_form.php?id=$team->id><b>Disband Team*</b></a>]
        [<a href=team_email_list.php?id=$team->id><b>View Team Emails*</b></a>]
        <br><font size=2>* Team founder only</font>
        <br>
        <p>
    ";
    start_table();
    row1("Team info");
    if (strlen($team->description)) {
        row2("Description", sanitize_html($team->description));
    }
    if (strlen($team->url)) {;
        row2("Web site", "<a href=http://$team->url>http://$team->url</a>");
    }
    row2("Members", $team->nusers);

    $result = mysql_query("select * from user where teamid=$team->id");
    $total_credit = 0;
    for ($i = 0; $i < $team->nusers; $i++) {
        $user = mysql_fetch_object($result);
        $total_credit = $total_credit + $user->total_credit;
    }

    row2("Total credit", $total_credit);
    $result = mysql_query("select * from user where id=$team->userid");
    $user = mysql_fetch_object($result);
    row2("Founder", "<a href=show_user.php?userid=$user->id>$user->name</a>");
    row2("Country", $team->country);
    echo "</table>";
    echo "<p>";
    start_table();
    row1("Team members", 4);
    echo "<tr>
        <th>Name</th>
        <th>Total credit</th>
        <th>Recent average credit</th>
        <th>Country</th>
        </tr>
    ";

    $result = mysql_query("select * from user where teamid=$team->id order by expavg_credit desc");

    $j = 1;
    while (true) {
        $user = mysql_fetch_object($result);
        if (!$user) break;
        echo "<tr>
            <td align=left>$j) <a href=show_user.php?userid=$user->id>$user->name</a>
            <td align=center>$user->total_credit</td>
            <td align=center>$user->expavg_credit</td>
            <td align=center>$user->country</td>
            </tr>
        ";
        $j++;
    }
    mysql_free_result($result);
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
    require_team($team);
    if ($user->id != $team->userid) {
        page_head("Permission denied");
        echo "Only a team's founder may edit a team.";
        page_tail();
        exit();
    }
}

function team_table_start() {
    echo "<tr>
        <th>Name</th>
        <th>Members</th>
        <th>Average credit</th>
        <th>Total credit</th>
        <th>Country</th>
        </tr>
    ";
}

function show_team_row($team, $i) {
    echo "<tr>
        <td>$i) <a href=team_display.php?id=$team->id>$team->name</a></td>
        <td>$team->nusers</td>
        <td>$team->expavg_credit</td>
        <td>$team->total_credit</td>
        <td>$team->country</td>
    </tr>\n";
}

?>
