<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

function show_admin($user, $admin) {
    $date = date_str($admin->create_time);
    echo "<tr>
        <td>".user_links($user)."</td>
        <td>$date</td>
        <td>
    ";
    show_button("team_admins.php?teamid=$admin->teamid&action=remove&userid=$user->id", "Remove");
    echo "</td></tr>
    ";
}

function show_admins($teamid) {
    page_head("Add or remove Team Admins");
    echo "
        You can select team members as 'Team Admins'.
        Team Admins can:
        <ul>
        <li> Edit team information (name, URL, description, country).
        <li> Remove members.
        <li> View the member list (including email addresses).
        <li> View the team's join/quit history.
        </ul>
        Team Admins cannot:
        <ul>
        <li> Change the team founder.
        <li> Add or remove Team Admins.
        </ul>
        If a Team Admin quits the team, they cease to be a Team Admin.
        <p>
        We recommend that you select only people
        you know and trust very well as Team Admins.
    ";
    $admins = BoincTeamAdmin::enum("teamid=$teamid");
    start_table();
    if (count($admins)==0) {
        row1("No admins");
    } else {
        row1("Current Team Admins", 3);
        table_header("Name", "Became Team Admin on", "");
        foreach ($admins as $admin) {
            $user = BoincUser::lookup_id($admin->userid);
            show_admin($user, $admin);
        }
    }
    end_table();

    echo "
        <p>
        <form action=team_admins.php>
        <input type=hidden name=action value=add>
        <input type=hidden name=teamid value=$teamid>
    ";
    start_table();
    row1("Add Team Admin");
    row2("Email address of team member:", "<input name=email_addr>");
    row2("", "<input type=submit action value=\"Add\">");
    end_table();
    echo "</form>";

    page_tail();
}

function remove_admin($team) {
    $userid = get_int('userid');
    $ret = BoincTeamAdmin::delete("teamid=$team->id and userid=$userid");
    if (!$ret) {
        error_page("failed to remove admin");
    }
}

function add_admin($team) {
    $email_addr = get_str('email_addr');
    $user = BoincUser::lookup("email_addr='$email_addr'");
    if (!$user) error_page("no such user");
    if ($user->teamid != $team->id) error_page("User is not member of team");
    if (is_admin($user, $team)) {
        error_page("$email_addr is already an admin of $team->name");
    }
    $now = time();
    $ret = BoincTeamAdmin::insert("(teamid, userid, create_time) values ($team->id, $user->id, $now)");
    if (!$ret) error_page("Couldn't add admin");
}

$user = get_logged_in_user();
$teamid = get_int('teamid');
$team = BoincTeam::lookup_id($teamid);
if (!$team) error_page("No such team");
require_founder_login($user, $team);

$action = get_str('action', true);
switch($action) {
case 'remove':
    remove_admin($team);
    Header("Location: team_admins.php?teamid=$teamid");
    exit();
case 'add':
    add_admin($team);
    Header("Location: team_admins.php?teamid=$teamid");
    exit();
}
show_admins($teamid);

?>
