<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user();
$teamid = get_int("id");

$team = lookup_team($teamid);
$team_name = $team->name;
page_head("Join $team_name");
echo " <p><b>Please note:</b>
    <ul>
    <li> Joining a team gives its founder access to your email address.
    <li> Joining a team does not affect your account's credit.
    </ul>
    <hr>
    <form method=post action=team_join_action.php>
    <input type=hidden name=teamid value=$teamid>
    <input type=submit value='Join team'>
    </form>
";
page_tail();

?>
