<?php

require_once("db.inc");
require_once("util.inc");
require_once("team.inc");

db_init();

$user = get_logged_in_user();

page_head("Create a team");
team_edit_form(null, "Create team", "team_create_action.php");
page_tail();
?>
