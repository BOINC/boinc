<?php

require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/sanitize_html.inc");

function display_team_page($team, $user) {
    page_head("$team->name", $user);
    echo "<h2>Team page for ";
    if (strlen($team->name_html)) {
        echo "$team->name_html";
    } else {
        echo $team->name;
    }
    echo "</h2>";
    if ($user->teamid == $team->id) {
        echo "You currently belong to this team
            <ul>
            <li><a href=team_quit_form.php?id=$team->id>Quit</a>
            </ul>
        ";
    } else {
        echo "You don't currently belong to this team.
            <ul>
            <li><a href=team_join_form.php?id=$team->id>Join this team</a>
            </ul>
        ";
    }
    if ($user->id == $team->userid) {
        echo "
            You are the founder of this team.
            <ul>
            <li><a href=team_edit_form.php?teamid=$team->id>Edit team info</a>
            <li><a href=team_remove_inactive_form.php?teamid=$team->id>Remove inactive members</a>
            <li><a href=team_email_list.php?teamid=$team->id>View team email addresses</a>
            </ul>
        ";
    }
    start_table();
    row1("Team info");
    if (strlen($team->description)) {
        row2("Description", sanitize_html($team->description));
    }
    if (strlen($team->url)) {;
        row2("Web site", "<a href=http://$team->url>http://$team->url</a>");
    }
    row2("Members", $team->nusers);

    row2("Total credit", format_credit($team->total_credit));
    row2("Recent average credit", format_credit($team->expavg_credit));
    $user = lookup_user_id($team->userid);
    row2("Founder", user_links($user));
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
    while ($user = mysql_fetch_object($result)) {
        $user_total_credit = format_credit($user->total_credit);
        $user_expavg_credit = format_credit($user->expavg_credit);
        $x = user_links($user);
        echo "<tr class=row1>
            <td align=left>$j) $x
            <td align=center>$user_total_credit</td>
            <td align=center>$user_expavg_credit</td>
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
        <th>Rank</th>
        <th>Name</th>
        <th>Members</th>
        <th><a href=top_teams.php?sort_by=expavg_credit>Recent average credit</a></th>
        <th><a href=top_teams.php?sort_by=total_credit>Total credit</a></th>
        <th>Country</th>
        </tr>
    ";
}

function show_team_row($team, $i) {
    $team_expavg_credit = format_credit($team->expavg_credit);
    $team_total_credit = format_credit($team->total_credit);
    echo"<tr class=row1>
        <td>$i</td>
        <td><a href=team_display.php?teamid=$team->id>$team->name</a></td>
        <td>$team->nusers</td>
        <td>$team_expavg_credit</td>
        <td>$team_total_credit</td>
        <td>$team->country</td>
        </tr>\n";
}

function user_join_team($team, $user) {
    if ($user->teamid != 0) {
        $query_team_other = sprintf(
            "select * from team where id = %d",
            $user->teamid
        );
        $result_team_other = mysql_query($query_team_other);
        $first_team = mysql_fetch_object($result_team_other);
        $first_nusers = $first_team->nusers;
        $first_new_nusers = $first_nusers - 1;
        if ($first_new_nusers > 0) {
            $query_team_table_other = sprintf(
                "update team set nusers = %d where id = %d",
                $first_new_nusers,
                $first_team->id
            );
        } else {
            // no more users in this team: disband it.
            $query_team_table_other = sprintf(
                "delete from team where id = %d",
                $first_team->id
            );
        }
        $result_team_table_other = mysql_query($query_team_table_other);
    }
    $query_user_table = sprintf(
        "update user set teamid = %d where id = %d",
        $team->id,
        $user->id
    );
    $result_user_table = mysql_query($query_user_table);
    $result_team_table = mysql_query("update team set nusers=nusers+1 where id = $team->id");
    if ($result_user_table && $result_team_table) return true;
    else return false;
}

function team_edit_form($team, $label, $url) {
    echo "<form method=post action=$url>\n";
    if ($team) {
        echo "<input type=hidden name=teamid value=$team->id>\n";
    }
    start_table();
    row1("Create a team");
    row2( "Team name (text version)
        <br><font size=2>
        Don't use any HTML tags.
        This name will be used in the searchable team list.",
        "<input name=name type=text size=50 value='$team->name'>"
    );
    row2("Team name (HTML version)
        <br><font size=2>
        You may include HTML formatting, link, and image tags.
        If you don't know HTML, just leave this box blank.",
        "<input name=name_html type=text size=50 value=\"$team->name_html\">"
    );
    row2("URL of team web page, if any:<br><font size=2>(without \"http://\")
        This URL will be linked to from the team's page on this site.",
        "<input name=url size=60 value='$team->url'>"
    );
    row2("Description of team:",
        "<textarea name=description cols=60 rows=10>$team->description</textarea>"
    );
    $x1 = $team->type==1?"checked":"";
    $x2 = $team->type==2?"checked":"";
    $x3 = $team->type==3?"checked":"";
    $x4 = $team->type==4?"checked":"";
    $x5 = $team->type==5?"checked":"";
    $x6 = $team->type==6?"checked":"";
    $x7 = $team->type==7?"checked":"";
    if ($team==null) $x1 = "checked";
    row2("Type of team:",
        "<input type=radio name=type value=1 $x1> Other
        <br>
        <input type=radio name=type value=2 $x2> Company
        <br>
        <input type=radio name=type value=3 $x3> Primary School
        <br>
        <input type=radio name=type value=4 $x4> Secondary School
        <br>
        <input type=radio name=type value=5 $x5> Junior College
        <br>
        <input type=radio name=type value=6 $x6> University or Department
        <br>
        <input type=radio name=type value=7 $x7> Government Agency"
    );
    row2_init("Country",
        "<select name=country>"
    );
    print_country_select($team->country);

    echo "</select></b></td></tr>\n";
    row2("",
        "<input type=submit name=new value='$label'>"
    );
    end_table();
    echo "</form>\n";
}

?>
