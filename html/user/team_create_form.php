<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();

$user = get_logged_in_user();

page_head("Create a team");
team_edit_form(null, "Create team", "team_create_action.php");
page_tail();
?>
