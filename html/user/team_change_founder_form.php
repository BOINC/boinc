<?php

// Let the founder assign foundership to someone else,
// or decline a foundership change request

require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/boinc_db.inc");

$user = get_logged_in_user();

$teamid = get_int("teamid");
$team = BoincTeam::lookup_id($teamid);
if (!$team) {
    error_page("no such team");
}
require_founder_login($user, $team);

page_head("Change founder of $team->name");

if ($team->ping_user != 0) {
    if ($team->ping_user < 0) {
        $ping_user = BoincUser::lookup_id(-$team->ping_user);
        $x = date_str($team->ping_time);
        echo "<p>Team member ".user_links($ping_user)." requested this
            team's foundership on $x, but left the team, thus canceling the request.
        ";
    } else {
        $ping_user = BoincUser::lookup_id($team->ping_user);
        $x = date_str(transfer_ok_time($team));
        echo "<p>Team member ".user_links($ping_user)." has requested this
            team's foundership.
            This may be because you left the team or haven't had contact
            with the team for a long time.
        ";
        echo "<p>
            <form method=\"post\" action=\"team_founder_transfer_action.php\">
            <input type=\"hidden\" name=\"action\" value=\"decline\">
            <input type=\"hidden\" name=\"teamid\" value=\"".$team->id."\">
            To decline the request, <input type=\"submit\" value=\"click here\">
            </form>
            <p>
            If you don't decline the request by $x,
            $ping_user->name will have the option of
            assuming team foundership.
            <p>
            To accept the request, assign foundership to $ping_user->name
            using the form below.
            <p>
            <hr>
            <p>
        ";
    }
}

echo "
    To assign foundership of this team to another member,
    check the box next to member name and click <b>Change founder</b> below.
    <form method=post action=team_change_founder_action.php>
    <input type=hidden name=teamid value=$team->id>
";
echo form_tokens($user->authenticator);

start_table();
echo "<tr>
    <th>New founder?</th>
    <th>Name</th>
    <th>Total credit</th>
    <th>Recent average credit</th>
    </tr>
";

$users = BoincUser::enum("teamid=$team->id");

$navailable_users = 0;
foreach ($users as $user) {
    if ($user->id != $team->userid) {       //don't show current founder
        $user_total_credit = format_credit($user->total_credit);
        $user_expavg_credit = format_credit($user->expavg_credit);
        $selected = ($user->id == $team->ping_user)?"selected":"";
        echo '
            <tr>
            <td align="center"><input type="radio" name="userid" value="'.$user->id.'">
            <td>'.$user->name.'</td>
            <td>'.$user_total_credit.'</td>
            <td>'.$user_expavg_credit.'</td>
            </tr>
        ';
        $navailable_users++;
    }
}
if ($navailable_users > 0) {
    echo "<input type=hidden name=navailable_users value=$navailable_users>";
    end_table();
    echo "<input type=submit value=\"Change founder\">";
} else {
    echo '<tr>
        <td colspan="4">There are no users to transfer team to.</td>
        </tr>
    ';
    end_table();
}
echo "</form>";
page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
