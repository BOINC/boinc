<?php

include_once("db.inc");
include_once("util.inc");
include_once("login.inc");
include_once("team.inc");

db_init();

$user = get_user_from_cookie();
print_teams_display();

?>
