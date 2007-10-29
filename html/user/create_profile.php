<?php

require_once("../inc/profile.inc");

$user = get_logged_in_user(true);
show_profile_creation_page($user);

?>
