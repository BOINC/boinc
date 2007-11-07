<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

$user = get_logged_in_user();

$teamid = get_int("teamid");
$team = BoincTeam::lookup_id($teamid);
if (!$team) error_page("no such team");
require_admin($user, $team);

$team_name = ereg_replace("\"", "'", $team->name);
page_head("Edit ".$team_name);
team_edit_form($team, "Update team info", "team_edit_action.php");
page_tail();

?>
