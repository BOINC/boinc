<?php

require_once("util.inc");
require_once("db.inc");
require_once("project_specific/project.inc");
require_once("profile.inc");

db_init();
$user = get_logged_in_user(true);
show_profile_creation_page($user);

?>
