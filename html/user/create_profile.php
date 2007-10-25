<?php

require_once("../inc/profile.inc");

db_init();

$user = get_logged_in_user(true);
show_profile_creation_page($user);

?>
