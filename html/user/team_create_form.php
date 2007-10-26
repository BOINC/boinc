<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

$user = get_logged_in_user();

page_head("Create a team");
if ($user->teamid) {
    echo "You already belong to a team.
        You must quit your current team before creating a new one.
    ";
} else {
    team_edit_form(null, "Create team", "team_create_action.php");
}
page_tail();
?>
