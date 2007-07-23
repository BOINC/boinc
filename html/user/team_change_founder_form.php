<?php

// handler for the "change founder" team management function

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user();

$teamid = $_GET["teamid"];
$team = lookup_team($teamid);
require_founder_login($user, $team);

page_head("Change founder of $team->name");

if ($team->ping_user) {
    $ping_user = lookup_user_id($team->ping_user);
    echo "<p>Team member ".user_links($ping_user)." has requested this
        team's founder position.  This may be because you left
        the team or haven't had contact with the team for a long time.
    ";
    echo "<p>Use the following form to transfer team founder position or
        <form method=\"post\" action=\"team_founder_transfer_action.php\">
        <input type=\"hidden\" name=\"action\" value=\"decline\">
        <input type=\"hidden\" name=\"teamid\" value=\"".$team->id."\">
        <input type=\"submit\" value=\"decline proposal\">
        </form>
        </p>
    ";
}

echo "
    <form method=post action=team_change_founder_action.php>
    <input type=hidden name=teamid value=$team->id>
";
start_table();
echo "<tr>
    <th>New founder?</th>
    <th>Name</th>
    <th>Total credit</th>
    <th>Recent average credit</th>
    </tr>
";

$result = mysql_query("select * from user where teamid = $team->id");

$navailable_users = 0;
while ($user = mysql_fetch_object($result)) {
    if($user->id!=$team->userid) {       //don't show current founder
        $user_total_credit = format_credit($user->total_credit);
        $user_expavg_credit = format_credit($user->expavg_credit);
        echo '
            <tr>
            <td align="center"><input type="radio" name="change_'.$navailable_users.'" value="'.$user->id.'">
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
mysql_free_result($result);
page_tail();

?>
