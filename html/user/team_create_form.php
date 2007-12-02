<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

$user = get_logged_in_user();

page_head("Create a team");
if ($user->teamid) {
    $team = BoincTeam::lookup_id($user->teamid);
    echo "You belong to 
        <a href=team_display.php?teamid=$team->id>$team->name</a>.
        You must <a href=team_quit_form.php>quit this team</a> before creating a new one.
    ";
} else {
    team_edit_form(null, "Create team", "team_create_action.php");
}
page_tail();
?>
