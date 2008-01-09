<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

ini_set("memory_limit", "1024M");

$logged_in_user = get_logged_in_user();
$teamid = get_int("teamid");
$team = BoincTeam::lookup_id($teamid);
if (!$team) error_page("no such team");
require_admin($logged_in_user, $team);
page_head("Remove Members from $team->name");
echo "
    <form method=\"post\" action=\"team_remove_inactive_action.php\">
    <input type=\"hidden\" name=\"id\" value=\"".$team->id."\">
";
start_table();
echo "<tr>
    <th>Remove?</th>
    <th>Name</th>
    <th>Total credit</th>
    <th>Recent average credit</th>
    </tr>
";

$users = BoincUser::enum("teamid=$team->id");
$ninactive_users = 0;
foreach($users as $user) {
    if ($user->id == $logged_in_user->id) continue;
    if ($user->id == $team->userid) continue;
    $user_total_credit = format_credit($user->total_credit);
    $user_expavg_credit = format_credit($user->expavg_credit);
    echo "
        <tr>
        <td align=center><input type=checkbox name=remove_$ninactive_users value=$user->id>
        <td>$user->name</td>
        <td>$user_total_credit</td>
        <td>$user_expavg_credit</td>
        </tr>
    ";
    $ninactive_users++;
}
end_table();
if ($ninactive_users == 0) {
    echo "<p>No members are eligible for removal.";
} else {
    echo "<input type=hidden name=ninactive_users value=$ninactive_users>";
    echo "<input type=submit value=\"Remove users\">";
}
echo "</form>";
page_tail();
?>
