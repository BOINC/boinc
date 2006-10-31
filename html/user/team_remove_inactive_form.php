<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user();
$teamid = get_int("teamid");

if ($user->teamid == $teamid) {

    $team = lookup_team($teamid);
    require_founder_login($user, $team);
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

    $result = mysql_query("select * from user where teamid = $team->id");

    $ninactive_users = 0;
    while ($user = mysql_fetch_object($result)) {
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
    echo "<input type=hidden name=ninactive_users value=$ninactive_users>";
    if ($result) {
	mysql_free_result($result);
    }
    end_table();
    echo "<input type=submit value=\"Remove users\">";
    echo "</form>";
    page_tail();
} else {
    error_page("You need to be the member and the founder of the team to edit team information.");
}
?>
