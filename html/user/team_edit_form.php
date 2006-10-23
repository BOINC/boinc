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

    $team_name = ereg_replace("\"", "'", $team->name);
    $team_type = $team->type;
    page_head("Edit ".$team_name);
    team_edit_form($team, "Update team info", "team_edit_action.php");
    page_tail();
} else {
    error_page("You need to be the member and the founder of the team to edit team information.");
}

?>
